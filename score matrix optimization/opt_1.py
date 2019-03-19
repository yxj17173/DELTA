#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import datetime
import itertools
import os
import random
import re
import string
import traceback
from random import randint
from functools import reduce

import numpy as np
import pandas as pd

from functions import *

startTime = datetime.datetime.now()
#parameters
file_name = "fun.alm"
num_split = 5
value_range = 1
scale = 2
depth = 3
fake_max = 100 # OR 1000
prune = 10 * value_range
whether_diag = True

#initialize
clt_name = str(file_name.split('.')[0])
result = pd.read_csv(file_name,delimiter='\t')['Class'].value_counts().to_frame()
num = int(len(result.index))
sort_cell_type = dict(zip(list(string.digits[0:len(result.index)]),result.index))
matrix_zero = np.zeros((num,num), dtype = int)

#generate the main directory
if  whether_diag == True:
    dir_0 = "./result_diag_" + clt_name + "_split_" +str(num_split) + "_depth_" + str(depth)
else:
    dir_0 = "./result_" + clt_name + "_split_" +str(num_split) + "_depth_" + str(depth)

mkdir(dir_0)
dir_clt_0 = dir_0 + "/" + clt_name 
mkdir(dir_clt_0)

#generate all the fake trees of whole cell types 
dir_clt_full = dir_clt_0 + "/" + clt_name + "_full"

list_cell_type_full = list_mapping(sort_cell_type, list(range(0,num)))
fake_select_generate(file_name, dir_clt_full, list_cell_type_full)

for L in range(depth + 1):
    prune = prune * (scale ** L)
    dir_1 = dir_0 + "/depth_%d" % L
    mkdir(dir_1)
    #update the matrix by the current optimization
    if L > 0:
        dir_matrix = dir_0 + "/depth_%d" % (L-1) + "/round_8/df_matrix.csv"
        matrix_pd = pd.read_csv(dir_matrix, delimiter=" ")
        matrix_zero = matrix_pd.iloc[:,1:].values

    
    np_order = order_diag(num)
    np_order_splited = np_vsplit(np_order, num_split)
    turns = len(np_order_splited)
    rounds = 0
    while rounds < turns:
        print(rounds)
        #create directory
        dir_2 = dir_1 + "/round_%d" % rounds
        mkdir(dir_2)
        dir_matrix = dir_2 + "/matrix"
        mkdir(dir_matrix)
        dir_script = dir_2 + "/script"
        mkdir(dir_script)
        dir_result = dir_2 + "/result"
        mkdir(dir_result)
        dir_np_result = dir_2 + "/np_result"
        mkdir(dir_np_result)

        #selcted cell types by round
        order_i = np_order_splited[rounds]
        order_i_list = list(set(order_i.flatten()))
        order_i_list.sort()
        list_cell_type = list_mapping(sort_cell_type, order_i_list)

        #generate all fake trees of associated cell types 
        fake_count = reduce(lambda x,y:x*y, range(1,len(order_i_list) + 1))
        if len(order_i_list) == num:
            dir_clt = dir_clt_full
        else:
            dir_clt = dir_clt_0 + "/" + clt_name + "_%d" % rounds
            if L == 0:
                mkdir(dir_clt)
                fake_select_generate(file_name, dir_clt, list_cell_type)

            
        #generate all the different matrixs
        np_matrix = np.zeros(shape=(2**num_split, num_split))
        names = locals()
        num_element = 0 #num_elementi(max)= len(order)
        for x,y in order_i:
            num = int(matrix_zero[x][y])
            if L > 0:
                if num > 0:
                    names["element%s" % num_element] = range(num * scale - 1, num * scale+1)
                    #names["element%s" % num_element] = range(num-value_range,num+value_range+1) #Change the value
                else:
                    names["element%s" % num_element] = range(num * scale, num * scale + 2)
            else:
                names["element%s" % num_element] = [-1,1]
            num_element += 1

        count_matrix = 0
        for p in itertools.product(names["element0"],names["element1"],names["element2"],names["element3"],names["element4"]):
        #for p in itertools.product([-1,1],repeat=5):
            np_matrix[count_matrix] = p
            matrix_name = dir_matrix+"/matrix_%d.csv" % count_matrix
            count_matrix = count_matrix + 1
            z = 0
            for x,y in order_i:
                matrix_zero[x][y] = p[z]# Somehow need to test symmetry
                z = z + 1
            matrix_final = symarray(matrix_zero)
            print(matrix_final)
            DataFrame2vector(result,result,matrix_final,matrix_name)
        x = "There is "+str(count_matrix)+ " matrixs are genetated"
        print(x)


        df_parameter = pd.DataFrame(columns=['target','time','remark'])
        for matrix_NO in range(0,count_matrix):
        #for matrix_NO in range(1,11): ## small test
            try:
                #Generate HSA scripts   
                HSA_scripts = dir_script + "/run_HSA_%d.sh" % matrix_NO
                f_script = open(HSA_scripts, "w+")
                f_script.write("#!/bin/bash\n")
                file_matrix = dir_matrix+"/"+"matrix_%d.csv" % matrix_NO
                file_result_real = dir_result+"/result.almg"
                f_script.write(HSA_g(file_name,file_name,file_matrix,file_result_real, prune)) 

                if fake_count > fake_max:
                    sum_rand = 0
                    for fake_NO in range(0,fake_max):
                        matrix_rand = randint(0,fake_count-1)
                        sum_rand = sum_rand + matrix_rand
                        fake_clt = dir_clt+"/"+clt_name+"_%d.alm" % matrix_rand
                        file_result = dir_result+"/result_%d.almg" % fake_NO
                        f_script.write(HSA_g(fake_clt,file_name,file_matrix,file_result,prune))
                    #random_seed = sum_rand/100
                    f_script.close()

                else:
                    for fake_NO in range(0,fake_count):
                        fake_clt = dir_clt+"/"+clt_name+"_%d.alm" % fake_NO
                        file_result = dir_result+"/result_%d.almg" % fake_NO
                        f_script.write(HSA_g(fake_clt,file_name,file_matrix,file_result,prune))
                    f_script.close()
                print("HSA scipts are generated well")

                #run HSA and calculate the target score
                os.system("python multiThread.py 50 " + HSA_scripts)
                print("HSA ran well done")
                #alalysis and calculate the target
                np_result = dir_np_result +"/np_result_%d.txt" % matrix_NO
                f_np_result = open(np_result, "w+")
                
                Global_score = []
                Parameter = []
                score_real = grep_score(file_result_real)
                f_np_result.write(str(score_real))

                if fake_count > fake_max:
                    for fake_NO in range(0,fake_max):
                        file_result = dir_result+"/result_%d.almg" % fake_NO
                        Global_score.append(grep_score(file_result))

                else:
                    for fake_NO in range(0,fake_count):
                        file_result = dir_result+"/result_%d.almg" % fake_NO
                        Global_score.append(grep_score(file_result))
                f_np_result.write(str(Global_score))
                f_np_result.close()

                Global_score_np = np.array(Global_score)
                score_mean = Global_score_np.mean()
                score_max = Global_score_np.max()
                if score_max == score_real:
                    remark = "cool"
                else:
                    remark = "bad"
                target = int(score_real - score_mean)
                timePerMatrix = datetime.datetime.now() - startTime
                Parameter.append(target)
                Parameter.append(timePerMatrix)
                Parameter.append(remark)
                df_parameter.loc[matrix_NO] = Parameter

            except:
                f=open("error_log.txt",'a')   
                traceback.print_exc(file=f)   
                f.flush()   
                f.close()   

        df_matrix = pd.DataFrame(np_matrix)
        df_result = pd.concat([df_parameter,df_matrix], axis=1)
        df_result_name = dir_2+"/df_result.csv"
        df_result.to_csv(df_result_name, sep = ',')

        #update the matrix
        df_filtered = df_result.loc[df_result["target"] == df_result["target"].max()]
        if len(df_filtered.index) > 1:
            f_warning = open("warningg.txt",'a')
            warning = "The No. "+str(rounds)+" rounds has one more maximum"
            f_warning.write(warning)
        matrix_result = df_filtered.values.tolist()[0][3:]
        alpha = 0
        for x,y in order_i:
            matrix_zero[x][y] = matrix_result[alpha]
            alpha += 1
        matrix_final0 = symarray(matrix_zero)
        df_output = pd.DataFrame(matrix_final0,index=result.index,columns=result.index)
        df_matrix_name = dir_2+"/df_matrix.csv"
        df_output.to_csv(df_matrix_name, sep = ' ')

        rounds += 1 

timeAll = datetime.datetime.now() - startTime
print(timeAll)
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import datetime
import itertools
import linecache
import os
import random
import re
import string
import traceback
from functools import reduce
from random import randint

import numpy as np
import pandas as pd
from numpy import genfromtxt

from functions import *

startTime = datetime.datetime.now()
#parameters
file_name0 = "fun.alm"
file_name1 = "pma.alm"
num_split = 6
value_range = 1
scale = 2
depth = 3
fake_max = 1000
prune = 10 * value_range
repeat = 1
witch_fake = False #"True" mean file_name0, "False" mean file_name1

#initialize
result0 = pd.read_csv(file_name0,delimiter='\t')['Class'].value_counts().to_frame()
num0 = int(len(result0.index))
sort_cell_type0 = dict(zip(list(string.digits[0:num0]), result0.index))

result1 = pd.read_csv(file_name1,delimiter='\t')['Class'].value_counts().to_frame()
num1 = int(len(result1.index))
sort_cell_type1 = dict(zip(list(string.digits[0:num1]), result1.index))

num_max = max(num0,num1)
matrix_zero = np.zeros((num0,num1), dtype = int)

if witch_fake == True:
    clt_name = str(file_name0.split('.')[0])
    sort_cell_type = dict(zip(list(string.digits[0:num0]),result0.index))
    file_name = file_name0
    file_name_solid = file_name1
    num = num0
    result = result0
else:
    clt_name = str(file_name1.split('.')[0])
    sort_cell_type = dict(zip(list(string.digits[0:num1]),result1.index))
    file_name = file_name1
    file_name_solid = file_name0
    num = num1
    result = result1

#generate the main directory
dir_0 = "./new_ v    result.vs_fake." + clt_name + "_split." +str(num_split) + "_depth." + str(depth) +"_fm." +str(fake_max)+"_r." + str(repeat)
mkdir(dir_0)
dir_clt_0 = dir_0 + "/" + clt_name 
mkdir(dir_clt_0)

#generate all the fake trees of whole cell types 
dir_clt_full = dir_clt_0 + "/" + clt_name + "_full"

list_cell_type_full = list_mapping(sort_cell_type, list(range(0,num)))
fake_select_generate(file_name, dir_clt_full, list_cell_type_full)

np_order = order_diag(num_max)
np_order_splited = np_vsplit(np_order, num_split)
turns = len(np_order_splited)

for L in range(depth + 1):
    prune = prune * (scale ** L)
    dir_1 = dir_0 + "/depth_%d" % L
    mkdir(dir_1)
    #update the matrix by the current optimization
    if L > 0:
        dir_matrix = dir_0 + "/depth_" +str(L - 1) + "/round_"+ str(turns - 1) + "/df_matrix.csv"
        matrix_pd = pd.read_csv(dir_matrix, delimiter=" ")
        matrix_zero = matrix_pd.iloc[:,1:].values

    def order_split(num_split,num):
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
            lst0 = []
            lst1 = []
            for x, y in order_i:
                lst0.append(x)
                lst1.append(y)
            order_i_list0 = list(set(lst0))
            list_cell_type0 = list_mapping(sort_cell_type0, order_i_list0)

            order_i_list1 = list(set(lst1))
            list_cell_type1 = list_mapping(sort_cell_type1, order_i_list1)
            if witch_fake == True:
                order_i_list = order_i_list0
                list_cell_type = list_cell_type0
            else:
                order_i_list = order_i_list1
                list_cell_type = list_cell_type1

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
            names = dict()
            num_element = 0 #num_elementi(max)= len(order)
            for x,y in order_i:
                number = int(matrix_zero[x][y])
                if L > 0:
                    if number > 0:
                        names["element%s" % num_element] = range(number * scale - 1, number * scale + 1)
                    else:
                        names["element%s" % num_element] = range(number * scale, number * scale + 2)
                else:
                    names["element%s" % num_element] = [-1,1]
                num_element += 1
            
            pools = [tuple(vales_range) for vales_range in names.values()]
            result = [[]]
            for pool in pools:
                result = [x+[y] for x in result for y in pool]

            count_matrix = 0
            for p in result:
                np_matrix[count_matrix] = p
                matrix_name = dir_matrix+"/matrix_%d.csv" % count_matrix
                count_matrix = count_matrix + 1
                z = 0
                for x,y in order_i:
                    matrix_zero[x][y] = p[z]# Somehow need to test symmetry
                    z = z + 1
                print(matrix_zero)
                DataFrame2vector(result0, result1, matrix_zero, matrix_name)
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
                    f_script.write(HSA_g(file_name, file_name_solid, file_matrix, file_result_real, prune)) 

                    if fake_count > fake_max:
                        #sum_rand = 0
                        for fake_NO in range(0,fake_max):
                            matrix_rand = randint(0,fake_count-1)
                            fake_clt = dir_clt+"/"+clt_name+"_%d.alm" % matrix_rand
                            file_result = dir_result+"/result_%d.almg" % fake_NO
                            f_script.write(HSA_g(fake_clt, file_name_solid, file_matrix, file_result, prune))
                            #sum_rand = sum_rand + matrix_rand
                        #random_seed = sum_rand/fake_max
                        f_script.close()

                    else:
                        for fake_NO in range(0,fake_count):
                            fake_clt = dir_clt+"/"+clt_name+"_%d.alm" % fake_NO
                            file_result = dir_result+"/result_%d.almg" % fake_NO
                            f_script.write(HSA_g(fake_clt, file_name_solid, file_matrix, file_result, prune))
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
                    print(score_real)
                    f_np_result.write(str(score_real))

                    if fake_count > fake_max:
                        for fake_NO in range(0,fake_max):
                            file_result = dir_result+"/result_%d.almg" % fake_NO
                            Global_score.append(grep_score(file_result))

                    else:
                        for fake_NO in range(0,fake_count):
                            file_result = dir_result+"/result_%d.almg" % fake_NO
                            Global_score.append(grep_score(file_result))

                    print(Global_score)
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
                    dir_error = dir_1 + "/error_log.txt"
                    f = open(dir_error, 'a')
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
                dir_warning = dir_1 + "/warning.txt"
                f_warning = open(dir_warning, 'a')
                warning = "The No. "+str(rounds)+" rounds has one more maximum"
                f_warning.write(warning)
            matrix_result = df_filtered.values.tolist()[0][3:]
            alpha = 0
            for x,y in order_i:
                matrix_zero[x][y] = matrix_result[alpha]
                alpha += 1

            df_output = pd.DataFrame(matrix_zero, index=result0.index, columns=result1.index)
            df_matrix_name = dir_2+"/df_matrix.csv"
            df_output.to_csv(df_matrix_name, sep = ' ')

            rounds += 1 

    order_split(num_split,num_max)

timeAll = datetime.datetime.now() - startTime
print(timeAll)

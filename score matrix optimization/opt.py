#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Optimize to get a better score matrix(Fig.7 and supply Fig.4)
# Before run this script, move the DELTA app(from ./app/) to the current directory. The final optmiazed matrix is in the '~/depth_3/round_17/df_matrix.csv'
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

from functions import mkdir, fake_select_generate, DataFrame2vector, HSA_g, grep_score, list_mapping, np_vsplit, order_asym_new2

startTime = datetime.datetime.now()

#parameters
# use order_asym_new2 num_split = 4
file_name0 = "fun.alm"
file_name1 = "pma.alm"
num_split = 4
value_range = 1
scale = 2
depth = 3
fake_max = 1000
prune = 10 * value_range
repeat = 1

#initialize
result0 = pd.read_csv(file_name0,delimiter='\t')['Class'].value_counts().to_frame()
num0 = int(len(result0.index))
clt_name0 = str(file_name0.split('.')[0])
sort_cell_type0 = dict(zip(list(string.digits[0:num0]), result0.index))

result1 = pd.read_csv(file_name1,delimiter='\t')['Class'].value_counts().to_frame()
num1 = int(len(result1.index))
clt_name1 = str(file_name1.split('.')[0])
sort_cell_type1 = dict(zip(list(string.digits[0:num1]), result1.index))

num_max = max(num0, num1)
matrix_zero = np.zeros((num0, num1), dtype=int)

#generate the main directory
dir_0 = "./new_result_split." +str(num_split) + "_depth." + str(depth) + "_r." + str(repeat)
mkdir(dir_0)
dir_clt_0 = dir_0 + "/" + clt_name0
mkdir(dir_clt_0)
dir_clt_1 = dir_0 + "/" + clt_name1
mkdir(dir_clt_1)

np_order = order_asym_new2(num0, num1)
np_order_splited = np_vsplit(np_order, num_split)
turns = len(np_order_splited)

for L in range(depth + 1):
    prune = prune * (scale ** L)
    dir_1 = dir_0 + "/depth_%d" % L
    mkdir(dir_1)
    #update the matrix by the current optimization
    if L > 0:
        dir_matrix = dir_0 + "/depth_" + \
            str(L - 1) + "/round_" + str(turns - 1) + "/df_matrix.csv"
        matrix_pd = pd.read_csv(dir_matrix, delimiter=" ")
        matrix_zero = matrix_pd.iloc[:, 1:].values

    def order_split(num_split, num):
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

            #generate all fake trees of associated cell types
            fake_count0 = reduce(lambda x, y: x*y, range(1, len(order_i_list0) + 1))
            """if len(order_i_list0) == num0:
                dir_clt0 = dir_clt_full0
            else:"""
            dir_clt0 = dir_clt_0 + "/" + clt_name0 + "_%d" % rounds
            if L == 0:
                mkdir(dir_clt0)
                fake_select_generate(file_name0, dir_clt0, list_cell_type0)

            fake_count1 = reduce(lambda x, y: x*y, range(1, len(order_i_list1) + 1))
            """if len(order_i_list1) == num1:
                dir_clt1 = dir_clt_full1
            else:"""
            dir_clt1 = dir_clt_1 + "/" + clt_name1 + "_%d" % rounds
            if L == 0:
                mkdir(dir_clt1)
                fake_select_generate(file_name1, dir_clt1, list_cell_type1)

            #generate all the different matrixs
            np_matrix = np.zeros(shape=(2**num_split, num_split))
            names = dict()
            num_element = 0  # num_elementi(max)= len(order)
            for x, y in order_i:
                number = int(matrix_zero[x][y])
                if L > 0:
                    if number > 0:
                        names["element%s" % num_element] = range(
                            number * scale - 1, number * scale + 1)
                    else:
                        names["element%s" % num_element] = range(
                            number * scale, number * scale + 2)
                else:
                    names["element%s" % num_element] = [-1, 1]
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
                for x, y in order_i:
                    matrix_zero[x][y] = p[z]  # Somehow need to test symmetry
                    z = z + 1
                print(matrix_zero)
                DataFrame2vector(result0, result1, matrix_zero, matrix_name)
            x = "There is "+str(count_matrix) + " matrixs are genetated"
            print(x)

            df_parameter = pd.DataFrame(columns=['target', 'time', 'remark'])
            for matrix_NO in range(0, count_matrix):
                #for matrix_NO in range(1,11): ## small test
                try:
                    #Generate HSA scripts
                    HSA_scripts = dir_script + "/run_HSA_%d.sh" % matrix_NO
                    f_script = open(HSA_scripts, "w+")
                    f_script.write("#!/bin/bash\n")
                    file_matrix = dir_matrix+"/"+"matrix_%d.csv" % matrix_NO
                    file_result_real = dir_result+"/result.almg"
                    f_script.write(HSA_g(file_name0, file_name1, file_matrix, file_result_real, prune))

                    if fake_count0*fake_count1 > fake_max:
                        for fake_NO in range(0, fake_max):
                            matrix_rand0 = randint(0, fake_count0-1)
                            matrix_rand1 = randint(0, fake_count1-1)
                            fake_clt0 = dir_clt0+"/"+clt_name0+"_%d.alm" % matrix_rand0
                            fake_clt1 = dir_clt1+"/"+clt_name1+"_%d.alm" % matrix_rand1
                            file_result = dir_result+"/result_%d.almg" % fake_NO
                            f_script.write(HSA_g(fake_clt0, fake_clt1, file_matrix, file_result, prune))
                        f_script.close()

                    else:
                        fake_NO = 0
                        for fake_NO_0 in range(0, fake_count0):
                            for fake_NO_1 in range(0, fake_count1):
                                fake_clt0 = dir_clt0+"/"+clt_name0+"_%d.alm" % fake_NO_0
                                fake_clt1 = dir_clt1+"/"+clt_name1+"_%d.alm" % fake_NO_1
                                file_result = dir_result+"/result_%d.almg" % fake_NO
                                fake_NO += 1
                                f_script.write(HSA_g(fake_clt0, fake_clt1, file_matrix, file_result, prune))
                        f_script.close()
                    print("HSA scipts are generated well")

                    #run HSA and calculate the target score
                    os.system("python multiThread.py 50 " + HSA_scripts)
                    print("HSA ran well done")

                    #alalysis and calculate the target
                    np_result = dir_np_result + "/np_result_%d.txt" % matrix_NO
                    f_np_result = open(np_result, "w+")

                    Global_score = []
                    Parameter = []
                    score_real = grep_score(file_result_real)
                    print(score_real)
                    f_np_result.write(str(score_real))

                    if fake_count0*fake_count1 > fake_max:
                        for fake_NO in range(0, fake_max):
                            file_result = dir_result+"/result_%d.almg" % fake_NO
                            Global_score.append(grep_score(file_result))

                    else:
                        for fake_NO in range(0, fake_count0*fake_count1):
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
            df_result = pd.concat([df_parameter, df_matrix], axis=1)
            df_result_name = dir_2+"/df_result.csv"
            df_result.to_csv(df_result_name, sep=',')

            #update the matrix
            df_filtered = df_result.loc[df_result["target"]
                                        == df_result["target"].max()]
            if len(df_filtered.index) > 1:
                dir_warning = dir_1 + "/warning.txt"
                f_warning = open(dir_warning, 'a')
                warning = "The No. "+str(rounds)+" rounds has one more maximum"
                f_warning.write(warning)
            matrix_result = df_filtered.values.tolist()[0][3:]
            alpha = 0
            for x, y in order_i:
                matrix_zero[x][y] = matrix_result[alpha]
                alpha += 1

            df_output = pd.DataFrame(
                matrix_zero, index=result0.index, columns=result1.index)
            df_matrix_name = dir_2+"/df_matrix.csv"
            df_output.to_csv(df_matrix_name, sep=' ')

            rounds += 1

    order_split(num_split, num_max)

timeAll = datetime.datetime.now() - startTime
print(timeAll)

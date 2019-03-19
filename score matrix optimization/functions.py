#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Some functions"""

import datetime
import itertools
import os
import re
import string
import math

import numpy as np
import pandas as pd


def replace_all(repls, str):
    """Replace all the elements in the repls dictionary  of strings,
    For example, replace dict{key(current):value(target)} """
    return re.sub('|'.join(re.escape(key) for key in repls.keys()),
                  lambda k: repls[k.group(0)], str) 


def symarray(a):
    num = len(a)
    for i in range(0,num):
        for j in range(0,num):
            if i >= j :
                continue
            a[j][i] = a[i][j]
    return a


def order_diag(num):   
    """the order of downward spiral on the diagonal(within)"""
    count = 0
    length = int((num * num- num)/2 + num)
    order = np.empty([length,2],dtype = int)
    for i in range(0,2*num-1):
        if i%2 == 0:
            a = int(i/2)
            b = int((i+1)/2)
        else:
            a = int((i-1)/2)
            b = int((i+2)/2)
        while a>=0 and b<=(num-1):
            order[count][0] = a
            order[count][1] = b
            count = count + 1
            a = a - 1
            b = b + 1
    #print(order)
    return order


def mkdir(dir_name):
    if os.path.exists(dir_name):
        print("There exists a directory named "+dir_name+" , it's going be deleted.")
        delete = 'rm -r '+dir_name
        os.system(delete)
    mkdir = 'mkdir ' + dir_name
    print('A directory named '+dir_name+' has being created.')
    os.system(mkdir)


def fake_generate(file_name,dir_clt,num):
    '''Using permutation generate all the fake cell lineage tree'''
    startTime = datetime.datetime.now()
    result = pd.read_csv(file_name,delimiter='\t')['Class'].value_counts().to_frame()
    list_index = result.index.tolist()[0:num]####
    string_letter0 = string.ascii_uppercase[0:num]
    list_letter0 = list(string_letter0)
    mapping0 = dict(zip(list_letter0,list_index))
    mkdir(dir_clt)
    text = open(file_name).read()
    count_ftree = 0
    for i in itertools.permutations(string_letter0, num):
        print(count_ftree)
        string_letter1 = ' '.join(i)
        transform_mapping = replace_all(mapping0,string_letter1)
        list_letter_trans = transform_mapping.split()
        mapping1 = dict(zip(list_index,list_letter_trans))
        print(mapping1)
        transform_result = replace_all(mapping1,text)
        str_open = './'+dir_clt+'/'+dir_clt+'_%d.alm' % count_ftree
        count_ftree +=1
        f_in = open(str_open, "w")
        f_in.write(transform_result)
        f_in.close()
    time = datetime.datetime.now() - startTime
    print(time)


def fake_select_generate(file_name,dir_clt,list_cell_type):
    '''Using permutation generate all the fake cell lineage trees of selcted cell types '''
    startTime = datetime.datetime.now()
    clt_name = str(file_name.split('.')[0])
    result = pd.read_csv(file_name,delimiter='\t')['Class'].value_counts().to_frame()
    sort_cell_type = dict(zip(list(string.digits[0:len(result.index)]),result.index))
    num = len(list_cell_type)
    list_index = list_mapping(sort_cell_type,list_cell_type)
    string_letter0 = string.ascii_uppercase[0:num]
    list_letter0 = list(string_letter0)
    mapping0 = dict(zip(list_letter0,list_index))
    mkdir(dir_clt)
    text = open(file_name).read()
    count_ftree = 0
    for i in itertools.permutations(string_letter0, num):
        string_letter1 = ' '.join(i)
        transform_mapping = replace_all(mapping0,string_letter1)
        list_letter_trans = transform_mapping.split()
        mapping1 = dict(zip(list_index,list_letter_trans))
        transform_result = replace_all(mapping1,text)
        str_open = dir_clt+'/'+clt_name+'_%d.alm' % count_ftree
        count_ftree +=1
        f_in = open(str_open, "w")
        f_in.write(transform_result)
        f_in.close()
    time = datetime.datetime.now() - startTime
    print(time)
    return count_ftree


def DataFrame2vector(result0,result1,matrix,matrix_name):
    '''convert a 2-d m*m pandas DataFrame to a 1-d （m*m） vector'''
    num0 = int(len(result0.index))
    num1 = int(len(result1.index))
    matrix_pd = pd.DataFrame(matrix,index=result0.index,columns=result1.index)
    #print(matrix_pd)
    list_index = [[] for j in range(num0 * num1)]
    #myList = [([0] * 3) for i in range(4)] 
    ##list * n—>n shallow copies of list concatenated
    i = 0
    for index in matrix_pd.index:
        for columns in matrix_pd.columns:
            list_index[i].append(index)
            list_index[i].append(columns)
            i = i + 1
    df_index = pd.DataFrame(list_index)
    df_value = pd.DataFrame(matrix.reshape(num0 * num1, 1))
    df_m2v = pd.concat([df_index,df_value],axis=1)
    #print(df_m2v)
    df_m2v.to_csv(matrix_name, sep = ' ', header=False, index = False)
    return df_m2v


def HSA_g(treeS,treeT,cost,result,prune):
    code_HSA_global = "./HSA -treeS " + treeS+ " -treeT " + treeT + " -cost " + cost + " \
    -method g -test 1 -outfile "+ result + " -prune %d\n" % prune
    return code_HSA_global

def grep_score(result):
    Score = int(open(result,"r").readline().split(':')[1])
    #Score = int(os.popen("grep 'Score' "+result).read().replace('\n','').replace('Score:',''))
    # time is same
    return Score 


def list_mapping(dict_mapping,list_current):
    lst = []
    for item in list_current:
        lst.append(replace_all(dict_mapping,str(item)))
    return lst


def np_vsplit(np_order, num_split):
    length = len(np_order)
    turns = int(length/num_split)
    if length%num_split == 0:
        turns -= 1
    turn = 0
    np_order_splited = []
    while turn < turns:
        np_splited = np_order[turn*num_split: (turn+1)*num_split]
        np_order_splited.append(np_splited)
        turn += 1
    if turn == turns:
        np_splited = np_order[length-num_split: length]
        np_order_splited.append(np_splited)
    return np_order_splited


def order_asym_new2(m, n):
    length = int(m * n)
    order = np.empty([length, 2], dtype=int)

    def distance(x, y):
        d = math.sqrt(x**2 + y**2)
        return d

    lst2 = []
    for y in range(n):
        y = float(y) + 0.5
        for x in range(m):
            x = float(x) + 0.5
            d = distance(x, y)
            lst3 = []
            a = x - 0.5
            b = y - 0.5
            lst3.append(a)
            lst3.append(b)
            lst3.append(d)
            lst2.append(lst3)
    #bubble_sort
    count = len(lst2)
    for i in range(0, count):
        for j in range(i + 1, count):
            if lst2[i][2] > lst2[j][2]:
                lst2[i], lst2[j] = lst2[j], lst2[i]

    count = 0
    for item2 in lst2:
            order[count][0] = item2[0]
            order[count][1] = item2[1]
            count = count + 1
    return order

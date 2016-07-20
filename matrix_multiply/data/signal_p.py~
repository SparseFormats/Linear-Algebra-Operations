import numpy as np
import random

f_in_a = open("rand_maxINT_sub_1000.txt","r")
#f_in_a = open("alaki.txt","r")
#f_in_a = open("rand2_10.txt","r")

rows_a= (map(int, (f_in_a.readline()).split( )))[0]
cols_a = (map(int, (f_in_a.readline()).split( )))[0]
a= np.zeros((rows_a, cols_a))

i = 0
j = 0
p_a = 0
for row in f_in_a:
    j = 0
    row_int = np.array(map(int, row.split( )))
    for item in row_int:
        if (item != 0):
            a[ i, j] = item
            p_a = p_a + pow(a[i, j], 2)
        j = j + 1
    i = i + 1

f_in_a.close()
size_a = i * j
p_a = p_a /size_a
print (pow(p_a ,0.5))

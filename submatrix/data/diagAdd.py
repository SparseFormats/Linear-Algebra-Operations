import numpy as np
import random

max_d = pow(2,10)
f_in_a = open("rand1_maxINT_1000.txt","r")
#f_in_a = open("a_small.txt","r")
#f_in_a = open("rand2_10.txt","r")

rows_a= (map(int, (f_in_a.readline()).split( )))[0]
cols_a = (map(int, (f_in_a.readline()).split( )))[0]
a= np.zeros((rows_a, cols_a))

i = 0
j = 0
for row in f_in_a:
    j = 0
    row_int = np.array(map(int, row.split( )))
    for item in row_int:
        if (item != 0):
            a[ i, j] = item
        j = j + 1
    i = i + 1

print a[0,0]
print a[0,1]
print a[0,2]

f_in_a.close()

#----------- READ IN PERMUTATION

#f_in_p = open("permute_order.txt","r")
#p = np.loadtxt(f_in_p)
#f_in_p.close()
d = np.zeros(cols_a)
d[0:cols_a - 3] = np.random.randint(0 , max_d, cols_a- 3)
d[cols_a -3] = 0
d[cols_a -2] = 0
d[cols_a -1] = 0

print d
d= np.array(d)
f_in_d = open("vec_diag.txt","w")
np.savetxt(f_in_d, d, fmt='%d')
f_in_d.close()

#-------------- CALCULATE C
for i in range (0, rows_a):
    a[i][i] = a[i][i] + d[i]

#print a

print a[0,0]
print a[0,1]
print a[0,2]


outfile = open('rand_permute_1000.txt', 'w')
#outfile = open('rand2_diag_add_10.txt', 'w')
np.savetxt(outfile , np.array([rows_a]), fmt = '%d')
np.savetxt(outfile , np.array([cols_a]), fmt = '%d')
np.savetxt(outfile, a, fmt = '%d')
outfile.close()

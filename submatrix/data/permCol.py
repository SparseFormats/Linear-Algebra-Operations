import numpy as np
import random

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
p = range(0, cols_a)
random.shuffle(p)

f_in_p = open("permute_order.txt","w")
np.savetxt(f_in_p, p, fmt='%d')
f_in_p.close()

#-------------- CALCULATE C
c = np.zeros((rows_a, cols_a))
for i in range (0, rows_a):
    for j in range (0, cols_a):
        p_j = p[j]
        c[i][j] = a[i][p_j]

#print c

print c[0,0]
print c[0,1]
print c[0,2]


outfile = open('rand_permute_1000.txt', 'w')
#outfile = open('rand2_permute_10.txt', 'w')
np.savetxt(outfile , np.array([rows_a]), fmt = '%d')
np.savetxt(outfile , np.array([cols_a]), fmt = '%d')
np.savetxt(outfile, c, fmt = '%d')
outfile.close()

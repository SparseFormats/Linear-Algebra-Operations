import numpy as np

f_in_a = open("/Users/Pareesa/Desktop/sparse-cortexProject/LinAlgebra/long-sim/March27/data/a_large.txt","r")

length_a= (map(int, (f_in_a.readline()).split( )))[0]
width_a = (map(int, (f_in_a.readline()).split( )))[0]

i = 0
j = 0
a= np.zeros((length_a, width_a))
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

f_in_b = open("/Users/Pareesa/Desktop/sparse-cortexProject/LinAlgebra/long-sim/March27/data/b_large.txt","r")

length_b= (map(int, (f_in_b.readline()).split( )))[0]
width_b = (map(int, (f_in_b.readline()).split( )))[0]

i = 0
b = np.zeros((length_b, width_b))
for row in f_in_b:
    j = 0
    row_int = np.array(map(int, row.split( )))
    for item in row_int:
        if (item != 0):
            b[ i, j] = item
        j = j + 1
    i = i + 1
            
f_in_b.close()
print b[0,0]
print b[1,0]
print b[2,0]

c = np.dot (a,b)
print c

print c[0,0]
print c[0,1]
print c[0,2]


outfile = open('ab.txt', 'w')
#np.savetxt(outfile , [width_a])
#np.savetxt(outfile , [length_b])
np.savetxt(outfile, c, fmt = '%d')
outfile.close()

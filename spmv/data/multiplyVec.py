import numpy as np

f_in_a = open("rand_maxINT_1000.txt","r")

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

f_in_b = open("vec_max_1000.txt","r")
b = np.loadtxt(f_in_b)            
f_in_b.close()

print b[0]
print b[1]
print b[2]

c = np.dot (a,b)
#print c

print c[0]
print c[1]
print c[2]


outfile = open('rand_spmv_1000.txt', 'w')
np.savetxt(outfile, c, fmt='%d')
outfile.close()

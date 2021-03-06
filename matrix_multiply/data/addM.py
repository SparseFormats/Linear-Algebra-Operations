import numpy as np

f_in_a = open("rand1_maxINT_100.txt","r")

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

#----------- READ IN B

f_in_b = open("rand2_maxINT_100.txt","r")

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

#-------------- CALCULATE C
c = a + b
#print c

print c[0,0]
print c[0,1]
print c[0,2]


outfile = open('rand_maxINT_add_100.txt', 'w')
np.savetxt(outfile , np.array([width_a]), fmt = '%d')
np.savetxt(outfile , np.array([length_a]), fmt = '%d')
np.savetxt(outfile, c, fmt = '%d')
outfile.close()

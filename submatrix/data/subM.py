import numpy as np
i1= 100
i2 = 300
j1 = 500
j2 = 800

f_in_a = open("rand1_maxINT_1000.txt","r")

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

#-------------- CALCULATE C
c = a[i1: (i2 + 1), j1:(j2 + 1)]
width_c = j2 - j1 + 1
length_c = i2 - i1 + 1


outfile = open('rand_maxINT_sub_1000.txt', 'w')
np.savetxt(outfile , np.array([length_c]), fmt = '%d')
np.savetxt(outfile , np.array([width_c]), fmt = '%d')
np.savetxt(outfile, c, fmt = '%d')
outfile.close()


outfile = open('sub_range.txt', 'w')
np.savetxt(outfile , np.array([i1, i2, j1, j2]), fmt = '%d')
outfile.close()

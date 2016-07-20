import numpy as np
import random as rnd

length_a= 100
width_a = 100
#max= 44
max = pow(2, 11)

i = 0
j = 0
a= np.zeros((length_a, width_a))
for i in range (0 , length_a):
    for j in range (0, width_a):
        p = rnd.randint(1, 11)
        if (p == 11):
            a[ i ][ j ] = rnd.randint(1, max + 1)

print a[0,0]
print a[0,1]
print a[0,2]

outfile = open('rand1_maxINT_100.txt', 'w')
np.savetxt(outfile , np.array([width_a]), fmt = '%d')
np.savetxt(outfile , np.array([length_a]), fmt = '%d')
np.savetxt(outfile, a, fmt = '%d')
outfile.close()

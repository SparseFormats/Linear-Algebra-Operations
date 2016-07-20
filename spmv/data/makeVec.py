import numpy as np
import random as rnd

Np= 2;
length_a= 1000
max = pow(2, 8)

i = 0
a= np.zeros(length_a)
for i in range (0 , length_a):
    p = rnd.randint(1, Np)
    if (p == Np):
        a[ i ] = rnd.randint(1, max + 1)

print a[0]
print a[1]
print a[2]

outfile = open('vec_max_1000.txt', 'w')
np.savetxt(outfile, a, fmt = '%d')
outfile.close()

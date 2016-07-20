import numpy as np

#f_in = open("/Users/Pareesa/Desktop/sparse-cortexProject/LinAlgebra/long-sim/March27/data/large.txt","r")
f_in = open("/Users/Pareesa/Desktop/sparse-cortexProject/LinAlgebra/long-sim/March30/data/aPb_large.txt","r")

#length= map(int, (f_in.readline()).split( ))
#width = map(int, (f_in.readline()).split( ))
length = [1000]
width = [1000]
nz = 0
nzRow = 0
nzRowMax = 0

for row in f_in:
    if (nzRow > nzRowMax):
        nzRowMax = nzRow
    nzRow = 0
    row_int = np.array(map(int, row.split( )))
    for item in row_int:
        if (item != 0):
            nz = nz+1
            nzRow = nzRow + 1
print nz/float(width[0] * length[0])
print "nz: "
print nz
print "nz in row: "
print nzRowMax
f_in.close()

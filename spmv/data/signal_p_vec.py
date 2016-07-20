import numpy as np


f_in_a = open("rand_spmv_1000.txt","r")
#f_in_a = open("alaki.txt","r")
a = np.loadtxt(f_in_a)            
f_in_a.close()

p_a= np.dot(a, a)
p_a = p_a / a.size
print a.size
print (pow(p_a, 0.5))

import matplotlib.pyplot as plt
import numpy as np
import random as rnd

N = 9
n0 = 4

def read_data(sprs):
    a = np.zeros(N - n0 + 1)
    rmse = np.zeros(N - n0 + 1)
    err_num = np.zeros(N - n0 + 1)

    str= 'OUT_'+sprs+'.txt'
    outfile = open(str, 'r')
    outfile.seek(0)
    outfile.readline()

    i = 0
    for line in outfile:  #-----read the err_pr, err_num, and rmse
        b= line.split(',')
        err_pr = float (((b[ 1 ]).split(": "))[1])
        this_rmse = float (((b[ 0 ]).split(": "))[1])
        this_err_num = int (((b[ 2 ]).split(": "))[1])
        if (err_pr == pow(10, -4)):
            i = 0
        elif (err_pr == pow(10, -5)):
            i = 1
        elif (err_pr == pow(10, -6)):
            i = 2
        elif (err_pr == pow(10, -7)):
            i = 3
        elif (err_pr == pow(10, -8)):
            i = 4
        elif (err_pr == pow(10, -9)):
            i = 5
        
        rmse[i] = rmse[i] + this_rmse
        err_num[i] = err_num[i] + this_err_num
        a[i] = a[i] + 1

    err_pr = np.zeros(N - n0 + 1)
    for i in range (0 , N - n0):
        rmse[i] = rmse[i]/a[i]
        err_num[i] = err_num[i]/a[i] 
        err_pr[i] = pow(10, -i - n0)

    outfile.close()
    return rmse, err_num, err_pr
#---------Plot the data
rmse_CRS, err_num_CRS, err_pr = read_data('CRS_PROTECTED_0') 
#rmse_CRS_1, err_num_CRS_1, err_pr = read_data('CRS_PROTECTED_1') 
rmse_COO, err_num_COO, err_pr = read_data('COO_PROTECTED_0') 
rmse_SLL, err_num_SLL, err_pr = read_data('SLL_PROTECTED_0') 
rmse_ELLPACK, err_num_ELLPACK, err_pr = read_data('ELLPACK_PROTECTED_0') 

p1, =plt.plot(err_pr, rmse_CRS)
#p2, =plt.plot(err_pr, rmse_CRS_1)
p2, =plt.plot(err_pr, rmse_COO)
p3, =plt.plot(err_pr, rmse_SLL)
p4, =plt.plot(err_pr, rmse_ELLPACK)

plt.legend([p1,p2,p3,p4],["CRS 0","COO 0", "SLL 0", "ELLPACK 0"], loc=2)
plt.ylabel('RMSE')
plt.xlabel('Log of Error Probability')
#plt.xlabel('Error Probability')
plt.xscale('log')
#plt.yscale('log')
plt.title('Matrix Multiplication')
plt.show()


p1, =plt.plot(err_pr, err_num_CRS)
#p2, =plt.plot(err_pr, err_num_CRS_1)
p2, =plt.plot(err_pr, err_num_COO)
p3, =plt.plot(err_pr, err_num_SLL)
p4, =plt.plot(err_pr, err_num_ELLPACK)

plt.legend([p1,p2,p3,p4],["CRS 0","COO 0", "SLL 0", "ELLPACK 0"], loc=2)
plt.ylabel('ERRNUM')
plt.xlabel('Log of Error Probability')
#plt.xlabel('Error Probability')
plt.xscale('log')
#plt.yscale('log')
plt.title('Matrix Multiplication')
plt.show()

#outfile = open('rand1_maxINT_1000.txt', 'w')
#np.savetxt(outfile , np.array([width_a]), fmt = '%d')
#np.savetxt(outfile , np.array([length_a]), fmt = '%d')
#np.savetxt(outfile, a, fmt = '%d')
#outfile.close()

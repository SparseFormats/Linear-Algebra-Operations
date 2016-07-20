import os
spr = 'CRS_PROTECTED_0'
err_pro =pow(10,-7)
for spr in ['CRS_PROTECTED_0', 'SLL_PROTECTED_0', 'COO_PROTECTED_0', 'ELLPACK_PROTECTED_0']:
#for spr in ['CRS_PROTECTED_1']:
    for err_pro in [pow(10,-9),pow(10,-8),pow(10,-7),pow(10,-6),pow(10,-5),pow(10,-4) ]:
            cmd = "make compile err_pr="+ str(err_pro) + " sparse=" + str(spr)+"\n"
            os.system(cmd)
            os.system("./sc")

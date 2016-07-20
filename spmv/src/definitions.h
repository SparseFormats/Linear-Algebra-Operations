//int err_num = 0;

//----- Decide the parameters
//#define ERR_PR 0
/*#define NZmax 196032
#define NZrowMax 843
#define NZcolMax 1000//---dont know
#define ROWS 1000
#define COLS 1000
#define VEC_MAX 196032
*/

//#define ERR_PR 0.0
/*#define NZmax 932
#define NZrowMax 17
#define NZcolMax 100//---dont know
#define ROWS 100
#define COLS 100
#define VEC_MAX 932
*/
/*
#define NZmax 929
#define NZrowMax 18
#define NZcolMax 100//---dont know
#define ROWS 100
#define COLS 100
#define VEC_MAX 929
*/

/*
#define NZmax 90902
#define NZrowMax 119
#define NZcolMax 1000//---dont know
#define ROWS 1000
#define COLS 1000
#define VEC_MAX 90902
*/
#define NZmax 90386
#define NZrowMax 120
#define NZcolMax 1000//---dont know
#define ROWS 1000
#define COLS 1000
#define VEC_MAX 90386

/*
#define NZmax 100
#define NZrowMax 10
#define NZcolMax 10//---dont know
#define ROWS 10
#define COLS 10
#define VEC_MAX 100
*/
#define ACCEPTABLE_ERR 0.00001

//----- files
/*
#define INPUT_FILE_1 "/u/pareesa/LibLin/longSim/March27/data/a_large.txt"
#define INPUT_FILE_2 "/u/pareesa/LibLin/longSim/March27/data/b_large.txt"
#define INPUT_FILE_MUL "/u/pareesa/LibLin/longSim/March27/data/ab.txt"*/

#define INPUT_FILE_1 "../data/rand_maxINT_1000.txt"
#define INPUT_FILE_2 "../data/vec_max_1000.txt"
#define INPUT_FILE_MUL "../data/rand_spmv_1000.txt"
//#define INPUT_FILE_2 "../data/rand2_maxINT_1000.txt"
//#define INPUT_FILE_MUL "../data/rand_permute_1000.txt"

//#define PERMUTE_FILE "../data/permute_order.txt"

/*
#define INPUT_FILE_1 "../data/rand2_10.txt"
#define INPUT_FILE_2 "../data/rand2_4.txt"
#define INPUT_FILE_MUL "../data/rand2_permute_10.txt"

/*
#define INPUT_FILE_1 "../data/a_small.txt"
#define INPUT_FILE_2 "../data/b_small.txt"
#define INPUT_FILE_MUL "../data/a_p_small.txt"
*/
//--- Assign fixed values
#define DENSE 0
#define DENSE_PROTECTED_0 100
#define DENSE_PROTECTED_1 1000
#define CRS 1
#define CRS_PROTECTED_0 10
#define CRS_PROTECTED_1 11
#define SLL 2
#define SLL_PROTECTED_0 20
#define SLL_PROTECTED_1 21
#define COO 3
#define COO_PROTECTED_0 30
#define COO_PROTECTED_1 31
#define ELLPACK 4
#define ELLPACK_PROTECTED_0 40
#define ELLPACK_PROTECTED_1 41
#define LIL 5
#define LIL_PROTECTED_0 50
#define LIL_PROTECTED_1 51
#define JAD 6
#define JAD_PROTECTED_0 60
#define JAD_PROTECTED_1 61

#define INT 0
#define VAL 1
#define COL 2
#define nxtPOINTER 3
#define VCN_POINTER 4
#define INT_POINTER 5

//---- functions
#define min(x , y) ((x > y) ? y : x )
#define max(x , y) ((x > y) ? x : y )
#define abs(x) ((x > 0) ? x : -x)
#define RAND rand_twister()
#define offsetof(st, m) ((size_t)&(((st *)0)->m))
#define offsetofint(st, m) ((int)&(((st *)0)->m))
//==================== STRUCTS =================//
struct val_col_node {
  int val;
  int col;
  struct val_col_node * next;
};
typedef struct val_col_node val_col_node;

typedef struct Jn {
  int i;
  int nz;
} JAD_node;


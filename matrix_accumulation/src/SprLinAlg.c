/*********************************************************************/
/* Linear Algebra for Sparse Matrices

   Author:
   Pareesa Golnari
   Phone:        512 201 5949
   email:        amene@princeton.edu
   P. Golnari Mar/9/2016 Princeton                                   */   
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "definitions.h"
#include "sparse_fun.h"

//------- Global values
extern int err_num;
extern long long read_num;
extern long long write_num;
extern long long out_read;
extern long long out_write;

int Nspr = 0;
int * start_spr_addr[20];
int * end_spr_addr[20];    

//---- DENSE:
int a_dense[ ROWS ][ COLS ], b_dense[ ROWS ][ COLS ], c_dense[ ROWS ][ COLS ];
  //---- CRS:
int a_val[ NZmax ], b_val[ NZmax ], c_val[ NZmax ];
int a_col[ NZmax ], b_col[ NZmax ], c_col[ NZmax ];
int a_rowPtr[ ROWS + 1 ], b_rowPtr[ ROWS + 1 ], c_rowPtr[ ROWS + 1 ];

  //---- SLL:
  int a_row[ NZmax + 1 ], b_row[NZmax + 1], c_row[NZmax + 1];

  //---- COO:
int a_val_col_row[ NZmax + 1 ][ 3 ], b_val_col_row[ NZmax + 1 ][ 3 ], c_val_col_row[ NZmax + 1 ][ 3 ];

  //---- ELLPACK:
int a_val_row[ ROWS ][ NZrowMax ], b_val_row[ ROWS ][ NZrowMax ], c_val_row[ ROWS ][ NZrowMax ];
int a_index_row[ ROWS ][ NZrowMax + 1], b_index_row[ ROWS ][ NZrowMax + 1], c_index_row[ ROWS ][ NZrowMax + 1];
  int a_NZ_row[ ROWS ], b_NZ_row[ ROWS ];//--- will be removed later
  
  //---- LIL
val_col_node * a_row_list[ ROWS ], * b_row_list[ ROWS ], * c_row_list[ ROWS ];
val_col_node a_LIL_nodes[ ROWS ][ NZrowMax + 1], b_LIL_nodes[ ROWS ][ NZrowMax + 1 ], c_LIL_nodes[ ROWS ][ NZrowMax + 1 ];

//---- JAD
int a_JADptr[ NZcolMax + 1], b_JADptr[ NZcolMax + 1], c_JADptr[ NZcolMax + 1];
int a_NZ_row[ ROWS ], b_NZ_row[ ROWS ], c_NZ_row[ ROWS ];
int a_row_order[ ROWS ], b_row_order[ ROWS ], c_row_order[ ROWS ];

  int a_rowMax, a_colMax, b_rowMax, b_colMax, c_rowMax = 0, c_colMax = 0, ab_rowMax, ab_colMax;
  

/*
int inboundry( int * intaddress){
  return 1;}

int inboundryf( float * floataddress){
  return 1;}
*/
float compareMatrix(int ** a, int **b, int rows, int cols, char op){
  int i, j;
  float sum = 0;
  float diff = 0;
  switch(op){
  case 'r':
    for (i = 0; i < rows; i++)
      for (j = 0; j < cols; j++){
	sum += pow ((a[i][j] - b[i][j]), 2);
	/*
	if (a[i][j] != b[i][j] && i == 0)
	  printf("c[%d][%d] = %d, ab = %d\n", i, j, a[i][j], b[i][j]);
	*/
      }
    return (sqrt(sum/ (rows * cols)));
    break;
  case 'd':
    for (i = 0; i < rows; i++){
      for (j = 0; j < cols; j++){
	diff = a[i][j] - b[i][j];
      sum += abs(diff);
      }
    }
  if (sum < ACCEPTABLE_ERR)
    printf("equal\n");
  else
    printf("NOT equal. Diff = %f\n", diff);
  return sum;
  }
}

void printMatrix(int ** symmat, int n, int m, int bias){
  int i, j;
  for (i = bias; i < (n + bias); i++){
    for (j = bias; j < (m + bias); j++){
      printf("%d ", symmat[i][j]);
    }
    printf("\n");
  }
}


void compare(int ** symmat1, int** symmat2, int m){
  int i, j;
  float diff = 0;
  float this_diff = 0;
  for (i = 1; i <= m; i++){
    for (j = 1; j <= m; j++){
      this_diff = symmat1[i][j] - symmat2[i][j];
      diff += abs(this_diff);
    }
  }
  if (diff < ACCEPTABLE_ERR)
    printf("equal\n");
  else
    printf("NOT equal. Diff = %f\n", diff);
  return;
}

double sparsity(int ** a, int rows, int cols, int start_index){
  double sum = 0;
  int i,j;
  for (i = start_index; i < (rows + start_index); i++)
    for (j = start_index; j <= (cols + start_index); j++)
	if (a[i][j] !=0)
	  sum++;
  return (sum /(cols * rows));
}

void printVec(float a[], int aSize){
  int i;
  for (i = 0; i < aSize; i++)
    printf("%f ,", a[i]);
  printf("\n");
}
void printVecI(int a[], int aSize){
  int i;
  for (i = 0; i < aSize; i++)
    printf("%d ,", a[i]);
  printf("\n");
}

int ** matrix(int rows, int cols){
  int i;
  int ** c;
  c = (int **) malloc( (unsigned) (rows) * sizeof(int*));
  for (i = 0; i < rows; i++){
    //    c[i] = (int *) malloc (cols * sizeof(int));
    c[i] = (int *) calloc (cols , sizeof(int));
  }
  return c;
}

void free_matrix(int **mat, int n, int m)
     /* Free a float matrix allocated by matrix(). */
{
  int i;

  for (i = n - 1; i >= 0; i--)
    {
      free ((int*) (mat[i]));
    }
  free ((int*) (mat));
  }

//=================== SPARSE CORES ======================//
void matrix_multiplication(int a_val[], int a_col[], int a_row[], int a_rowPtr[], int a_val_col_row[ NZrowMax + 1 ][ 3 ], int a_val_row[ ROWS ][ NZrowMax ], int a_index_row[ ROWS ][ NZrowMax + 1], val_col_node * a_row_list[], int a_row_max, int a_col_max, int b_val[], int b_col[], int b_row[], int b_rowPtr[], int b_val_col_row[ NZrowMax + 1 ][ 3 ], int b_val_row[ ROWS ][ NZrowMax ], int b_index_row[ ROWS ][ NZrowMax + 1], val_col_node * b_row_list[], int b_row_max, int b_col_max, int **c){
  int i, j, k = a_col_max;
  
  if (a_col_max != b_row_max){
    printf("BAD MATRIX DIMENSION, a_col :%d, b_row : %d\n", a_col_max, b_row_max);
    return;
  }

  //printf("inside mult: b_val[0][0]:%d , b_val[0][1]: %d, b_val[0][2]: %d\n", b_val[0], b_val[1], b_val[2]);


  int vec_val_1[ VEC_MAX ], vec_val_2[ VEC_MAX ], vec_val_3[ VEC_MAX ];
  int vec_index_1[ VEC_MAX ], vec_index_2[ VEC_MAX ], vec_index_3[ VEC_MAX ];
  int vec_size_1, vec_size_2, vec_size_3;
  int vec_size_1_pr, vec_size_2_pr, vec_size_3_pr;
  
    for (i = 0; i < a_row_max; i++){
      //printf("%d\n",i);


    //---- Pick row i of a
      switch (SPARSE){
      case CRS_PROTECTED_0:
	Pick_a_row_CRS_protected_0( a_val, a_col, a_rowPtr, i, vec_val_1, vec_index_1, &vec_size_1); break;
      case CRS_PROTECTED_1:
	Pick_a_row_CRS_protected_1( a_val, a_col, a_rowPtr, i, a_row_max, a_col_max, vec_val_1, vec_index_1, &vec_size_1); break;
      case SLL_PROTECTED_0:   
	Pick_a_row_SLL_protected_0( a_val, a_col, a_row, i, vec_val_1, vec_index_1, &vec_size_1); break;
      case SLL_PROTECTED_1:   
	Pick_a_row_SLL_protected_1( a_val, a_col, a_row, i, a_row_max, a_col_max, vec_val_1, vec_index_1, &vec_size_1); break;
      case COO_PROTECTED_0:       
	Pick_a_row_COO_protected_0( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case COO_PROTECTED_1:       
	//Pick_a_row_COO_protected_1( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); 
	break;
	  case ELLPACK_PROTECTED_1:   
	Pick_a_row_ELLPACK_protected_1( a_val_row, a_index_row, a_col_max, i, vec_val_1, vec_index_1, &vec_size_1); break;
      case ELLPACK_PROTECTED_0:   
	Pick_a_row_ELLPACK_protected_0( a_val_row, a_index_row, a_col_max, i, vec_val_1, vec_index_1, &vec_size_1); break;
	case LIL_PROTECTED_0:       Pick_a_row_LIL_protected_0( a_row_list, i, vec_val_1, vec_index_1, &vec_size_1); break;
	  case LIL_PROTECTED_1:       Pick_a_row_LIL_protected_0( a_row_list, i, vec_val_1, vec_index_1, &vec_size_1); break;
	}
      vec_size_1_pr = max( 0, min( vec_size_1, VEC_MAX ));
      /*
      if (i == 0){
	printf("\nrow: %d\n", i);
	printVecI(vec_val_1, vec_size_1_pr);
	printVecI(vec_index_1, vec_size_1_pr);
	}*/

            for (j = 0; j < b_col_max; j++){
	//----Pick col j of b
      	
	      switch (SPARSE){
	      case CRS_PROTECTED_0:    
		Pick_a_col_CRS_protected_0( b_val, b_col, b_rowPtr, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	      case CRS_PROTECTED_1: 
		Pick_a_col_CRS_protected_1( b_val, b_col, b_rowPtr, b_row_max, j, b_col_max, vec_val_2, vec_index_2, &vec_size_2); break;
	      case SLL_PROTECTED_0:       
		Pick_a_col_SLL_protected_0( b_val, b_col, b_row, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	      case SLL_PROTECTED_1:       
		Pick_a_col_SLL_protected_1( b_val, b_col, b_row, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); 
		break;
	      case COO_PROTECTED_0:
		Pick_a_col_COO_protected_0( b_val_col_row, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); 
		break;
	      case COO_PROTECTED_1:
	      //	    Pick_a_col_COO_protected_1( b_val_col_row, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); 
	      break;
	case ELLPACK_PROTECTED_0:   
	  Pick_a_col_ELLPACK_protected_0( b_val_row, b_index_row, b_row_max, b_col_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	case ELLPACK_PROTECTED_1:   
	  Pick_a_col_ELLPACK_protected_1( b_val_row, b_index_row, b_row_max, b_col_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	case LIL_PROTECTED_0:       
	  //	  Pick_a_col_LIL_protected_0( b_row_list, b_row_max, b_col_max, j, vec_val_2, vec_index_2, &vec_size_2); 
	  Pick_a_col_LIL( b_row_list, b_row_max, b_col_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	case LIL_PROTECTED_1:       
	  Pick_a_col_LIL_protected_1( b_row_list, b_row_max, b_col_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	}
	vec_size_2_pr = max( 0, min( vec_size_2, VEC_MAX));
	
	/*if(i==0 && j == 0){
	  printf("\ncol: %d\n", j);
	  printVecI(vec_val_2, vec_size_2_pr);
	  printVecI(vec_index_2, vec_size_2_pr);
	}*/
	  
	//---- Dot product of a-row[i] and b-col[j] and update c[i][j] in dense format
	spr_vectors_op(vec_val_1, vec_index_1, vec_size_1_pr, vec_val_2, vec_index_2, vec_size_2_pr, vec_val_3, vec_index_3, &vec_size_3_pr, '.');
	float res = spr_vector_reduction( vec_val_3, vec_index_3, vec_size_3_pr, k, 'S');
	//printf("%f\n",res);
	c[ i ][ j ] = res;
	}
    }

}

void matrix_accumulation(int a_val[], int a_col[], int a_row[], int a_rowPtr[], int a_val_col_row[ NZrowMax + 1 ][ 3 ], int a_val_row[ ROWS ][ NZrowMax ], int a_index_row[ ROWS ][ NZrowMax + 1], val_col_node * a_row_list[], int a_row_max, int a_col_max, int b_val[], int b_col[], int b_row[], int b_rowPtr[], int b_val_col_row[ NZrowMax + 1 ][ 3 ], int b_val_row[ ROWS ][ NZrowMax ], int b_index_row[ ROWS ][ NZrowMax + 1], val_col_node * b_row_list[], int b_row_max, int b_col_max, int c_val[], int c_col[], int c_row[], int c_rowPtr[], int c_val_col_row[ NZrowMax + 1 ][ 3 ], int c_val_row[ ROWS ][ NZrowMax ], int c_index_row[ ROWS ][ NZrowMax + 1], val_col_node * c_row_list[], int *c_row_max, int *c_col_max){
  int i, j, k = a_col_max;
  
  if (a_row_max != b_row_max || a_col_max != b_col_max){
    printf("BAD MATRIX DIMENSION, a_row :%d, a_col : %d, b_row :%d, b_col : %d\n", a_row_max, a_col_max, b_row_max, b_col_max);
    return;
  }

  int vec_val_1[ VEC_MAX ], vec_val_2[ VEC_MAX ], vec_val_3[ VEC_MAX ];
  int vec_index_1[ VEC_MAX ], vec_index_2[ VEC_MAX ], vec_index_3[ VEC_MAX ];
  int vec_size_1, vec_size_2, vec_size_3;
  int vec_size_1_pr, vec_size_2_pr, vec_size_3_pr;
  int vec_1[ COLS ], vec_2[ COLS ], vec_res[ COLS ];

    for (i = 0; i < a_row_max; i++){
      //printf("%d\n",i);

    //---- Pick row i of a and b
      switch (SPARSE){
      case DENSE_PROTECTED_0:
	Pick_a_row_DENSE_protected_0( a_dense, i, vec_1);
	Pick_a_row_DENSE_protected_0( b_dense, i, vec_2); 
	break;
      case CRS_PROTECTED_0:
	Pick_a_row_CRS_protected_0( a_val, a_col, a_rowPtr, i, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_CRS_protected_0( b_val, b_col, b_rowPtr, i, vec_val_2, vec_index_2, &vec_size_2); 
	break;
      case CRS_PROTECTED_1:
	//Pick_a_row_CRS_protected_0( a_val, a_col, a_rowPtr, i, vec_val_1, vec_index_1, &vec_size_1);
	//Pick_a_row_CRS_protected_0( b_val, b_col, b_rowPtr, i, vec_val_2, vec_index_2, &vec_size_2); 
		Pick_a_row_CRS_protected_1( a_val, a_col, a_rowPtr, i, a_row_max, a_col_max, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_CRS_protected_1( b_val, b_col, b_rowPtr, i, b_row_max, b_col_max, vec_val_2, vec_index_2, &vec_size_2);
	break;
      case SLL_PROTECTED_0:   
	Pick_a_row_SLL_protected_0( a_val, a_col, a_row, i, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_SLL_protected_0( b_val, b_col, b_row, i, vec_val_2, vec_index_2, &vec_size_2);
	break;
      case SLL_PROTECTED_1:   
	//Pick_a_row_SLL_protected_0( a_val, a_col, a_row, i, vec_val_1, vec_index_1, &vec_size_1);
	//Pick_a_row_SLL_protected_0( b_val, b_col, b_row, i, vec_val_2, vec_index_2, &vec_size_2);
	Pick_a_row_SLL_protected_1( a_val, a_col, a_row, i, a_row_max, a_col_max, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_SLL_protected_1( b_val, b_col, b_row, i, b_row_max, b_col_max, vec_val_2, vec_index_2, &vec_size_2);
	break;
      case COO_PROTECTED_0:       
	Pick_a_row_COO_protected_0( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); 
	Pick_a_row_COO_protected_0( b_val_col_row, i, vec_val_2, vec_index_2, &vec_size_2); 
	break;
	  //  case COO_PROTECTED_1:       Pick_a_row_COO_protected_1( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); break;
      case ELLPACK_PROTECTED_1:   
	Pick_a_row_ELLPACK_protected_1( a_val_row, a_index_row, a_col_max, i, vec_val_1, vec_index_1, &vec_size_1); 
	Pick_a_row_ELLPACK_protected_1( b_val_row, b_index_row, b_col_max, i, vec_val_2, vec_index_2, &vec_size_2); 
	break;
      case ELLPACK_PROTECTED_0:   
	Pick_a_row_ELLPACK_protected_0( a_val_row, a_index_row, a_col_max, i, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_ELLPACK_protected_0( b_val_row, b_index_row, b_col_max, i, vec_val_2, vec_index_2, &vec_size_2);
	break;
      case LIL_PROTECTED_0:
	printf("%d\n", i);       
	Pick_a_row_LIL_protected_0( a_row_list, i, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_LIL_protected_0( b_row_list, i, vec_val_2, vec_index_2, &vec_size_2);
	break;
      case LIL_PROTECTED_1:       
	Pick_a_row_LIL_protected_0( a_row_list, i, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_LIL_protected_0( b_row_list, i, vec_val_2, vec_index_2, &vec_size_2);
	break;
	}
      vec_size_1_pr = max( 0, min( vec_size_1, VEC_MAX ));
      vec_size_2_pr = max( 0, min( vec_size_2, VEC_MAX ));
      /*
      if (i == 0){
	printf("\nrow: %d\n", i);
	printVecI(vec_val_1, vec_size_1_pr);
	printVecI(vec_index_1, vec_size_1_pr);
	}*/
	  
      //---- Dot plus of a-row[i] and b-row[i]
      switch(SPARSE){
      case DENSE_PROTECTED_0:
        spr_vectors_op_DENSE(vec_1, vec_2, vec_res, a_col_max, '+');
	break;
      default:
        spr_vectors_op(vec_val_1, vec_index_1, vec_size_1_pr, vec_val_2, vec_index_2, vec_size_2_pr, vec_val_3, vec_index_3, &vec_size_3_pr, '+');
	break;
      }
 
      //      spr_vectors_op(vec_val_1, vec_index_1, vec_size_1_pr, vec_val_2, vec_index_2, vec_size_2_pr, vec_val_3, vec_index_3, &vec_size_3, '+');

      vec_size_3_pr = max( 0, min( vec_size_3, VEC_MAX ));

	//---- Append row i to c
	*c_col_max = a_col_max;
	switch (SPARSE){
	case DENSE_PROTECTED_0:
	  Append_a_row_DENSE_protected_0(c_dense, vec_res, c_row_max);
	  break;
	case CRS_PROTECTED_0:
	  Append_a_row_CRS_protected_0(c_val, c_col, c_rowPtr, vec_val_3, vec_index_3, vec_size_3_pr, c_row_max);
	  break;
	case CRS_PROTECTED_1:
	  Append_a_row_CRS_protected_1(c_val, c_col, c_rowPtr, vec_val_3, vec_index_3, vec_size_3_pr, c_row_max);
	  break;
	case SLL_PROTECTED_0:   
	  Append_a_row_SLL_protected_0( c_val, c_col, c_row, vec_val_3, vec_index_3, vec_size_3_pr, c_row_max);
	  break;
	case SLL_PROTECTED_1:   
	  Append_a_row_SLL_protected_0( c_val, c_col, c_row, vec_val_3, vec_index_3, vec_size_3_pr, c_row_max);
	  //Append_a_row_SLL_protected_1( c_val, c_col, c_row, i, vec_val_3, vec_index_3, vec_size_3, &c_row_max);
	  break;
	case COO_PROTECTED_0:       
	  Append_a_row_COO_protected_0( c_val_col_row, vec_val_3, vec_index_3, vec_size_3_pr, c_row_max); break;
	case COO_PROTECTED_1:      // Pick_a_row_COO_protected_1( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); 
	  break;
	case ELLPACK_PROTECTED_0:   
	  Append_a_row_ELLPACK( c_val_row, c_index_row, vec_val_3, vec_index_3, vec_size_3_pr, (*c_col_max), c_row_max);
	  /*int k;
	  for (k = 0; k < (*c_row_max); k++){
	    printf("row:%d\n",k);
	    printVecI(c_val_row[k], (*c_col_max));
	    printVecI(c_index_row[k], (*c_col_max) + 1);
	  }
	  */
	break;
      case ELLPACK_PROTECTED_1:   
	//Append_a_row_ELLPACK_protected_1( c_val_row, c_index_row, i, vec_val_3, vec_index_3, &vec_size_3, &c_row_max); 
	break;
	case LIL_PROTECTED_0:       
	Append_a_row_LIL_protected_0( c_row_list, c_LIL_nodes, vec_val_3, vec_index_3, vec_size_3_pr, c_row_max);/*
	int k1, k2;
	  for (k1 = 0; k1 < (*c_row_max); k1++){
	    printf("\nrow:%d\n",k);
	    for(k2 =0; k2 < (*c_col_max); k2++)
	      printf("%d ,", c_LIL_nodes[k1][k2].val);
	    printf("\n");
	    for(k2 =0; k2 < (*c_col_max)+ 1; k2++)
	      printf("%d ,", c_LIL_nodes[k1][k2].col);
	  }
														 */
	break;
      case LIL_PROTECTED_1:       
	//Append_a_row_LIL_protected_1( c_row_list, c_LIL_nodes, c_col_max, i, vec_val_3, vec_index_3, &vec_size_3, *c_row_max);
	break;
     }
    }
}


//====================== MAIN ===========================//
void main(){
  
  FILE * a_stream = fopen(INPUT_FILE_1, "r");
  FILE * b_stream = fopen(INPUT_FILE_2, "r");
  FILE * ab_stream = fopen(INPUT_FILE_MUL, "r");
  
  // --------- Initialize random generator --------
  init_genrand(time(NULL));   

 //--- Read matrix a and b
  fscanf(a_stream, "%d\n", &a_rowMax);
  fscanf(a_stream, "%d\n", &a_colMax);
  switch(SPARSE){
  case DENSE_PROTECTED_0:
  case DENSE_PROTECTED_1:
    processLineDense( a_dense, a_rowMax, a_colMax, a_stream);

    //----- Add a_dense to the sparse addresses list
    start_spr_addr[ Nspr ] = &(a_dense[0][0]);  end_spr_addr[ Nspr ] = &(a_dense[ROWS - 1][ COLS - 1 ]);
    Nspr ++;
    start_spr_addr[ Nspr ] = &(b_dense[0][0]);  end_spr_addr[ Nspr ] = &(b_dense[ROWS - 1][ COLS - 1 ]);
    Nspr ++;
    start_spr_addr[ Nspr ] = &(c_dense[0][0]);  end_spr_addr[ Nspr ] = &(c_dense[ROWS - 1][ COLS - 1 ]);
    Nspr ++;
    break;
  case CRS_PROTECTED_0:
  case CRS_PROTECTED_1:
    processLineCRS( a_val, a_col, a_rowPtr, a_rowMax, a_colMax, a_stream); 
    
    //----- Add val, col, and rowPtr to the sparse addresses list
    start_spr_addr[ Nspr ] = &(a_val[0]);  end_spr_addr[ Nspr ] = &(a_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &(a_col[0]);  end_spr_addr[ Nspr + 1 ] = &(a_col[ NZmax - 1]);
    start_spr_addr[ Nspr + 2 ] = &(a_rowPtr[0]);  end_spr_addr[ Nspr + 2 ] = &(a_rowPtr[ ROWS ]);
    Nspr += 3;
    start_spr_addr[ Nspr ] = &(b_val[0]);  end_spr_addr[ Nspr ] = &(b_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &(b_col[0]);  end_spr_addr[ Nspr + 1 ] = &(b_col[ NZmax - 1]);
    start_spr_addr[ Nspr + 2 ] = &(b_rowPtr[0]);  end_spr_addr[ Nspr + 2 ] = &(b_rowPtr[ ROWS ]);
    Nspr += 3;
    start_spr_addr[ Nspr ] = &(c_val[0]);  end_spr_addr[ Nspr ] = &(c_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &(c_col[0]);  end_spr_addr[ Nspr + 1 ] = &(c_col[ NZmax - 1]);
    start_spr_addr[ Nspr + 2 ] = &(c_rowPtr[0]);  end_spr_addr[ Nspr + 2 ] = &(c_rowPtr[ ROWS ]);
    Nspr += 3;
    break;
  
  case SLL_PROTECTED_0:
  case SLL_PROTECTED_1:
    processLineSLL( a_val, a_col, a_row, a_rowMax, a_colMax, a_stream); 

    //------- Add val, col, and row to the sparse addresses list
    start_spr_addr[ Nspr ] = &( a_val[0] );  end_spr_addr[ Nspr ] = &( a_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &( a_col[0] );  end_spr_addr[ Nspr + 1 ] = &( a_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &( a_row[0] );  end_spr_addr[ Nspr + 2 ] = &( a_row[ NZmax ]);
    Nspr += 3;
    start_spr_addr[ Nspr ] = &( b_val[0] );  end_spr_addr[ Nspr ] = &( b_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &( b_col[0] );  end_spr_addr[ Nspr + 1 ] = &( b_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &( b_row[0] );  end_spr_addr[ Nspr + 2 ] = &( b_row[ NZmax ]);
    Nspr += 3;
    start_spr_addr[ Nspr ] = &( c_val[0] );  end_spr_addr[ Nspr ] = &( c_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &( c_col[0] );  end_spr_addr[ Nspr + 1 ] = &( c_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &( c_row[0] );  end_spr_addr[ Nspr + 2 ] = &( c_row[ NZmax ]);
    Nspr += 3;

    //----- Initialize
  break;
  
  case COO_PROTECTED_0:
  case COO_PROTECTED_1:
    processLineCOO( a_val_col_row, a_rowMax, a_colMax, a_stream); 
    //----- Add val_col_row to the sparse addresses
    start_spr_addr[Nspr] = &(a_val_col_row[0][0]);  end_spr_addr[Nspr] = &(a_val_col_row[ NZmax ][2]);
    Nspr++;
    start_spr_addr[Nspr] = &(b_val_col_row[0][0]);  end_spr_addr[Nspr] = &(b_val_col_row[ NZmax ][2]);
    Nspr++;
     start_spr_addr[Nspr] = &(c_val_col_row[0][0]);  end_spr_addr[Nspr] = &(c_val_col_row[ NZmax ][2]);
       Nspr++;
    break;
  
  case ELLPACK_PROTECTED_0:
  case ELLPACK_PROTECTED_1:
    processLineELLPACK( a_val_row, a_index_row, a_NZ_row, a_rowMax, a_colMax, a_stream); 

    //----- Add val_row and index_row to the sparse addresses list
    start_spr_addr[ Nspr ] = &(a_val_row[0][0]);  end_spr_addr[ Nspr ] = &(a_val_row[ ROWS - 1 ][ NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = &(a_index_row[0][0]);  end_spr_addr[ Nspr + 1 ] = &(a_index_row[ ROWS - 1 ][ NZrowMax ]);
    Nspr += 2;
    start_spr_addr[ Nspr ] = &(b_val_row[0][0]);  end_spr_addr[ Nspr ] = &(b_val_row[ ROWS - 1 ][ NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = &(b_index_row[0][0]);  end_spr_addr[ Nspr + 1 ] = &(b_index_row[ ROWS - 1 ][ NZrowMax ]);
    Nspr += 2;
    start_spr_addr[ Nspr ] = &(c_val_row[0][0]);  end_spr_addr[ Nspr ] = &(c_val_row[ ROWS - 1 ][ NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = &(c_index_row[0][0]);  end_spr_addr[ Nspr + 1 ] = &(c_index_row[ ROWS - 1 ][ NZrowMax ]);
    Nspr += 2;
    break;
  
  case LIL_PROTECTED_0:
  case LIL_PROTECTED_1:     
    processLineLIL( a_row_list, a_LIL_nodes, a_NZ_row, a_rowMax, a_colMax, a_stream); 
  
    //----- Add row_list and LIL_nodes to the sparse addresses
    start_spr_addr[ Nspr ] = (int *) &(a_LIL_nodes[0][0]);
    end_spr_addr[ Nspr ] = (int *) &(a_LIL_nodes[ROWS - 1][NZrowMax - 1]);
    //printf("a_lil.val end:%d \n", end_spr_addr[ Nspr ]);
        
start_spr_addr[ Nspr + 1 ] = (int *) &(a_row_list[ 0 ]);
    end_spr_addr[ Nspr + 1 ] = (int *) &(a_row_list[ ROWS - 1 ]);
    Nspr += 2;
  
    start_spr_addr[ Nspr ] = (int *) &(b_LIL_nodes[0][0]);
    end_spr_addr[ Nspr ] = (int *) &(b_LIL_nodes[ROWS - 1][NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = (int *) &(b_row_list[ 0 ]);
    end_spr_addr[ Nspr + 1 ] = (int *) &(b_row_list[ ROWS - 1 ]);
    Nspr += 2;
    
    start_spr_addr[ Nspr ] = (int *) &(c_LIL_nodes[0][0]);
    end_spr_addr[ Nspr ] = (int *) &(c_LIL_nodes[ROWS - 1][NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = (int *) &(c_row_list[ 0 ]);
    end_spr_addr[ Nspr + 1 ] = (int *) &(c_row_list[ ROWS - 1 ]);
    Nspr += 2;
    break;
  case JAD_PROTECTED_0:
  case JAD_PROTECTED_1:
    processLineJAD( a_JADptr, a_val, a_col, a_val_row, a_index_row, a_NZ_row, a_rowMax, a_colMax, a_row_order, a_stream);

    //----- Add val_row and index_row to the sparse addresses list
    start_spr_addr[ Nspr ] = &(a_JADptr[0]);  end_spr_addr[ Nspr ] = &(a_JADptr[ NZcolMax ]);
    start_spr_addr[ Nspr + 1 ] = &(a_val[0]);  end_spr_addr[ Nspr + 1 ] = &(a_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &(a_col[0]);  end_spr_addr[ Nspr + 2 ] = &(a_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 3 ] = &(a_row_order[0]);  end_spr_addr[ Nspr + 3 ] = &(a_row_order[ ROWS - 1]);
    Nspr += 4;

    start_spr_addr[ Nspr ] = &(b_JADptr[0]);  end_spr_addr[ Nspr ] = &(b_JADptr[ NZcolMax ]);
    start_spr_addr[ Nspr + 1 ] = &(b_val[0]);  end_spr_addr[ Nspr + 1 ] = &(b_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &(b_col[0]);  end_spr_addr[ Nspr + 2 ] = &(b_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 3 ] = &(b_row_order[0]);  end_spr_addr[ Nspr + 3 ] = &(b_row_order[ ROWS - 1]); \
											      
    Nspr += 4;

    start_spr_addr[ Nspr ] = &(c_JADptr[0]);  end_spr_addr[ Nspr ] = &(c_JADptr[ NZcolMax ]);
    start_spr_addr[ Nspr + 1 ] = &(c_val[0]);  end_spr_addr[ Nspr + 1 ] = &(c_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &(c_col[0]);  end_spr_addr[ Nspr + 2 ] = &(c_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 3 ] = &(c_row_order[0]);  end_spr_addr[ Nspr + 3 ] = &(c_row_order[ ROWS - 1]); \
											       
    Nspr += 4;
    break;
  }

  //printf("err_pr2:%d\n", ERR_PR);

  fscanf(b_stream, "%d\n", &b_rowMax);
  fscanf(b_stream, "%d\n", &b_colMax);
  switch(SPARSE){
  case DENSE_PROTECTED_0:
  case DENSE_PROTECTED_1:
    processLineDense( b_dense, b_rowMax, b_colMax, b_stream); break;
  case CRS_PROTECTED_0:
  case CRS_PROTECTED_1:
    processLineCRS( b_val, b_col, b_rowPtr, b_rowMax, b_colMax, b_stream); break;
  
  case SLL_PROTECTED_0:
  case SLL_PROTECTED_1:
    processLineSLL( b_val, b_col, b_row, b_rowMax, b_colMax, b_stream); break;
  
  case COO_PROTECTED_0:
  case COO_PROTECTED_1:
    processLineCOO( b_val_col_row, b_rowMax, b_colMax, b_stream); break;
  
  case ELLPACK_PROTECTED_0:
  case ELLPACK_PROTECTED_1:
    processLineELLPACK( b_val_row, b_index_row, b_NZ_row, b_rowMax, b_colMax, b_stream); break;
  
  case LIL_PROTECTED_0:
  case LIL_PROTECTED_1:
    processLineLIL( b_row_list, b_LIL_nodes, b_NZ_row, b_rowMax, b_colMax, b_stream); break;
  case JAD_PROTECTED_1:
    processLineJAD( b_JADptr, b_val, b_col, b_val_row, b_index_row, b_NZ_row, b_rowMax, b_colMax, b_row_order, b_stream);
    break;  
  }
  
  //---- Read the result ab = a * b or ab= a + b
  fscanf(ab_stream, "%d\n", &ab_rowMax);
  fscanf(ab_stream, "%d\n", &ab_colMax);
  int ** ab_dense = matrix( ab_rowMax, ab_colMax);
  processLine( ab_dense, ab_rowMax, ab_colMax, ab_stream);
  
  //--- Matrix Multiplication : c = a * b
  int ** c = matrix( ab_rowMax, ab_colMax);
 
    matrix_accumulation(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, a_rowMax, a_colMax, b_val, b_col, b_row, b_rowPtr, b_val_col_row, b_val_row, b_index_row, b_row_list, b_rowMax, b_colMax, c_val, c_col, c_row, c_rowPtr, c_val_col_row, c_val_row, c_index_row, c_row_list, &c_rowMax, &c_colMax);

    //matrix_multiplication(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, a_rowMax, a_colMax, b_val, b_col, b_row, b_rowPtr, b_val_col_row, b_val_row, b_index_row, b_row_list, b_rowMax, b_colMax, c);
  
  
switch(SPARSE){
  case DENSE_PROTECTED_0:
  case DENSE_PROTECTED_1:
      Sprs_to_dense_DENSE(c_dense, c_rowMax, c_colMax, c); 
    break; 
  case CRS_PROTECTED_0:
  case CRS_PROTECTED_1:
      Sprs_to_dense_CRS(c_val, c_col, c_rowPtr, c_rowMax, c_colMax, c); 
    break; 
  case SLL_PROTECTED_0:
  case SLL_PROTECTED_1:
    Sprs_to_dense_SLL( c_val, c_col, c_row, c_rowMax, c_colMax, c); break;
  
  case COO_PROTECTED_0:
  case COO_PROTECTED_1:
    Sprs_to_dense_COO( c_val_col_row, c_rowMax, c_colMax, c); break;
   break;
  
  case ELLPACK_PROTECTED_0:
  case ELLPACK_PROTECTED_1:
    Sprs_to_dense_ELLPACK( c_val_row, c_index_row, c_rowMax, c_colMax, c); 
    break;
  
  case LIL_PROTECTED_0:
  case LIL_PROTECTED_1:
     Sprs_to_dense_LIL( c_row_list, c_rowMax, c_colMax, c); 
    break;

 case JAD_PROTECTED_0:
 case JAD_PROTECTED_1:
   Sprs_to_dense_JAD( c_val, c_col, c_row_order, c_JADptr, c_rowMax, c_colMax, c);
   break;
  }
  

/*
  printMatrix(c, c_rowMax, c_colMax, 0);
  
printf("\nC:\n");
 printVecI(c_val, NZmax);
 printVecI(c_col, NZmax);
 printVecI(c_rowPtr, c_rowMax + 1);
 //printVecI(c_row, NZmax);
 */ 


 float rmse = compareMatrix(c, ab_dense, ab_rowMax, ab_colMax, 'r');
 //float rmse = compareMatrix(c, ab_dense, 1, 5, 'r');
  
 printf("rmse: %f\n", rmse);
 printf("err_pr: %f\n", ERR_PR);
 printf("erNum: %d\n", err_num);
 printf("out read: %lld out of %lld\nout write: %lld out of %lld\n", out_read, read_num, out_write, write_num);
 //preparing output file
 char str[80];
 char sparseStr[50];
 strcpy(str, "../out/OUT_");
 switch (SPARSE){
 case DENSE_PROTECTED_0:
   strcpy(sparseStr,"DENSE_PROTECTED_0"); break;
 case CRS_PROTECTED_0:
   strcpy(sparseStr,"CRS_PROTECTED_0"); break;
 case CRS_PROTECTED_1:
   strcpy(sparseStr,"CRS_PROTECTED_1"); break;
 case SLL_PROTECTED_0:
   strcpy(sparseStr,"SLL_PROTECTED_0"); break;
 case SLL_PROTECTED_1:
   strcpy(sparseStr,"SLL_PROTECTED_1"); break;
 case COO_PROTECTED_0:
   strcpy(sparseStr,"COO_PROTECTED_0"); break;
 case COO_PROTECTED_1:
   strcpy(sparseStr,"COO_PROTECTED_1"); break;
 case LIL_PROTECTED_0:
   strcpy(sparseStr,"LIL_PROTECTED_0"); break;
 case LIL_PROTECTED_1:
   strcpy(sparseStr,"LIL_PROTECTED_1"); break;
 case ELLPACK_PROTECTED_0:
   strcpy(sparseStr,"ELLPACK_PROTECTED_0"); break;
 case ELLPACK_PROTECTED_1:
   strcpy(sparseStr,"ELLPACK_PROTECTED_1"); break;
 case JAD_PROTECTED_0:
   strcpy(sparseStr,"JAD_PROTECTED_0"); break;
 case JAD_PROTECTED_1:
   strcpy(sparseStr,"JAD_PROTECTED_1"); break;
 default:
   break;
 }
 strcat(str, sparseStr);

 strcat(str, ".txt");
 printf("%s\n",str);


 //-------- Write RMSE in a file- Pareesa
 FILE *outfile = fopen(str, "r");   //check if the file exists
 if (outfile==NULL){// it does not exist
   outfile = fopen(str, "a");
   fprintf(outfile, "SPARSE: %s, ERROR_PR: %.10f, ROWS: %d, COLS: %d, NZmax: %d\n",sparseStr, ERR_PR, ROWS, COLS, NZmax);
 }
 else{
   fclose(outfile);
   outfile = fopen(str, "a");
 }
 fprintf(outfile, "RMSE: %f, ERR_PR: %.10f, NUM_ERROR: %d, READ_NUM: %lld, OUT_READ: %lld, WRITE_NUM: %lld, OUT_WRITE: %lld\n", rmse ,ERR_PR ,err_num, read_num, out_read, write_num, out_write);

fclose(outfile);

 //---- Free allocated memory
 free_matrix(c, c_rowMax, c_colMax);
 free_matrix(ab_dense, ab_rowMax, ab_colMax);
 fclose(a_stream);
 fclose(b_stream);
 fclose(ab_stream);
  
 return;
}

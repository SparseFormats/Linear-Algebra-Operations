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
#include <assert.h>

//void Append_a_col_LIL_protected_0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize);
//void Append_a_col_SLL_protected_0(int val[], int col[], int row[], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize);
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

/*
void Remove_a_col_LIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int colNum, int *colMax){

  int k = 0, i;
  val_col_node * this_node;
  val_col_node * removing_node;

  for( i = 0; i < rowMax; i++){
    //---- remove node with j = colNum
    //    remove_node(row_list, LIL_nodes, i, colNum);
    removing_node = NULL;
    this_node = row_list[i];
    if (this_node != NULL){
      while (this_node->next != NULL){
        if (this_node->col > colNum) //----- j = j - 1 for the columns after colNum
          this_node->col = this_node->col - 1;
        else if (this_node->col == colNum)//---- remove it
          removing_node = this_node;
        this_node = this_node->next;
      }


      //---replace it with the last item in the row
      if (removing_node != NULL){
        removing_node->col = this_node->col;
        removing_node->val = this_node->val;

        //--- remove the last item
        this_node = NULL;
      }
    }
  }
  *colMax = *colMax - 1;
  return;
}
*/
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
/*
void Append_a_col_CRS(int val[], int col[], int rowPtr[], int rowMax, int * colMax, int valSpr[], int indexSpr[], int vecSize){
  int i = 0, i_start, i_end = 0, nz = 0, k, j = 0;

  if (vecSize <= 0){
    * colMax = *colMax + 1;
    return;
  }

  //printf("rowMax: %d, colMax:%d\n", rowMax, *colMax);
  //--- Find the size of the matrix:
  int matrix_size = rowPtr[rowMax];
  matrix_size += vecSize;
  //printf("matrix size: %d\n", matrix_size);

  //--- Initialize new val and col
  int col_new[ matrix_size + 1];
  int val_new[ matrix_size + 1];

  for ( k = 0; k < vecSize ; k++){

    //update rowPtr until a row with new nonzero value
    i_start = i_end;
    while (indexSpr[ k ] > i){
      rowPtr[ i ] += k;
      i++;
    }
    //printf("i:%d\n",i);
    //--- copy the current part of the row
    i_end = rowPtr[ i + 1 ];
    nz = i_end - i_start;

    // printf("j: %d, nz:%d, i_start:%d\n", j, nz, i_start);
    memcpy(&( val_new[ j ] ), &( val[ i_start ]), nz * sizeof(int) );
    memcpy(&( col_new[ j ] ), &( col[ i_start ]), nz * sizeof(int) );
    j += nz;

    //--- add the new value
    //printf("in loop j %d, matrix_size %d\n",j, matrix_size);
   
 val_new[ j ] = valSpr[ k ];
 col_new[ j ] = *colMax;
    j++;
    rowPtr[ i ] += k;
    i++;
    //    printf("k: %d\n",k);
    //printVecI(val_new,4);
    //printVecI(col_new,4);
    //printVecI(rowPtr, 11);    
}

  //--- update the rest of the rowPtr and vec and col
  int remaining = matrix_size - vecSize - rowPtr[ i ];
  remaining = max(remaining , 0);
  //printf("remaining: %d\n", remaining);
    memcpy(&( val_new[ j ] ), &( val[ i_start + nz ]), remaining * sizeof(int) );
    memcpy(&( col_new[ j ] ), &( col[ i_start + nz ]), remaining * sizeof(int) );

while (i <= rowMax){
    rowPtr[ i ] += vecSize;
    i++;
  }

  //--- update the return values
  
  //printf("c inside append:\n");
  //printVecI(val_new, matrix_size);
  //printVecI(col_new, matrix_size);
  
 //printVecI(val_new,matrix_size);
   // printVecI(col_new,matrix_size);
   // printVecI(rowPtr, 11);    


  memcpy(val, val_new, matrix_size * sizeof (int));
  memcpy(col, col_new, matrix_size * sizeof (int));
    
  //printf("j %d, matrix_size %d\n",j, matrix_size);
  
* colMax = *colMax + 1;

  return;
}
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
	if (a[i][j] != b[i][j] && j == 0)
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
	Pick_a_row_SLL_protected_1( a_val, a_col, a_row, i, vec_val_1, vec_index_1, &vec_size_1); break;
	case COO_PROTECTED_0:       Pick_a_row_COO_protected_0( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); break;
	  //	  case COO_PROTECTED_1:       Pick_a_row_COO_protected_1( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); break;
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
	  Pick_a_col_SLL_protected_1( b_val, b_col, b_row, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	  /*case COO_PROTECTED_0:
	    Pick_a_col_COO_protected_0( b_val_col_row, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); break;
	    case COO_PROTECTED_1:
	    Pick_a_col_COO_protected_1( b_val_col_row, b_row_max, j, vec_val_2, vec_index_2, &vec_size_2); break;*/
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
  
    for (i = 0; i < a_row_max; i++){
      //printf("%d\n",i);

    //---- Pick row i of a and b
      switch (SPARSE){
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
	Pick_a_row_SLL_protected_1( a_val, a_col, a_row, i, vec_val_1, vec_index_1, &vec_size_1);
	Pick_a_row_SLL_protected_1( b_val, b_col, b_row, i, vec_val_2, vec_index_2, &vec_size_2);
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
      spr_vectors_op(vec_val_1, vec_index_1, vec_size_1_pr, vec_val_2, vec_index_2, vec_size_2_pr, vec_val_3, vec_index_3, &vec_size_3, '+');

      vec_size_3_pr = max( 0, min( vec_size_3, VEC_MAX ));

	//---- Append row i to c
	*c_col_max = a_col_max;
	switch (SPARSE){
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

void matrix_permute_rows(int a_val[], int a_col[], int a_row[], int a_rowPtr[], int a_val_col_row[ NZrowMax + 1 ][ 3 ], int a_val_row[ ROWS ][ NZrowMax ], int a_index_row[ ROWS ][ NZrowMax + 1], val_col_node * a_row_list[], int a_row_max, int a_col_max, int c_val[], int c_col[], int c_row[], int c_rowPtr[], int c_val_col_row[ NZrowMax + 1 ][ 3 ], int c_val_row[ ROWS ][ NZrowMax ], int c_index_row[ ROWS ][ NZrowMax + 1], val_col_node * c_row_list[], int *c_row_max, int *c_col_max, int perm_order[]){
  int i, p_i;
  
  int vec_val_1[ VEC_MAX ];
  int vec_index_1[ VEC_MAX ];
  int vec_size_1;
  int vec_size_1_pr;
  
    for (i = 0; i < a_row_max; i++){
      //printf("%d\n",i);

      p_i = perm_order[i];//---- mem accecss? or all functional

      //---- Pick row p_i of a
      switch (SPARSE){
      case CRS_PROTECTED_0:
	Pick_a_row_CRS_protected_0( a_val, a_col, a_rowPtr, p_i, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case CRS_PROTECTED_1:
	Pick_a_row_CRS_protected_1( a_val, a_col, a_rowPtr, p_i, a_row_max, a_col_max, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case SLL_PROTECTED_0:   
	Pick_a_row_SLL_protected_0( a_val, a_col, a_row, p_i, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case SLL_PROTECTED_1:   
	Pick_a_row_SLL_protected_1( a_val, a_col, a_row, p_i, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case COO_PROTECTED_0:       
	Pick_a_row_COO_protected_0( a_val_col_row, p_i, vec_val_1, vec_index_1, &vec_size_1); 
	break;
	  //  case COO_PROTECTED_1:       Pick_a_row_COO_protected_1( a_val_col_row, p_i, vec_val_1, vec_index_1, &vec_size_1); break;
      case ELLPACK_PROTECTED_1:   
	Pick_a_row_ELLPACK_protected_1( a_val_row, a_index_row, a_col_max, p_i, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case ELLPACK_PROTECTED_0:   
	Pick_a_row_ELLPACK_protected_0( a_val_row, a_index_row, a_col_max, p_i, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case LIL_PROTECTED_0:       
	Pick_a_row_LIL_protected_0( a_row_list, p_i, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case LIL_PROTECTED_1:       
	Pick_a_row_LIL_protected_0( a_row_list, p_i, vec_val_1, vec_index_1, &vec_size_1);
	break;
	}
      vec_size_1_pr = max( 0, min( vec_size_1, VEC_MAX ));
      /*
      if (i == 0){
	printf("\nrow: %d\n", i);
	printVecI(vec_val_1, vec_size_1_pr);
	printVecI(vec_index_1, vec_size_1_pr);
	}*/
	  
      
      //---- Append row p_i to c
	*c_col_max = a_col_max;
	switch (SPARSE){
	case CRS_PROTECTED_0:
	  Append_a_row_CRS_protected_0(c_val, c_col, c_rowPtr, vec_val_1, vec_index_1, vec_size_1_pr, c_row_max);
	  break;
	case CRS_PROTECTED_1:
	  Append_a_row_CRS_protected_1(c_val, c_col, c_rowPtr, vec_val_1, vec_index_1, vec_size_1_pr, c_row_max);
	  break;
	case SLL_PROTECTED_0:   
	  Append_a_row_SLL_protected_0( c_val, c_col, c_row, vec_val_1, vec_index_1, vec_size_1_pr, c_row_max);
	  break;
	case SLL_PROTECTED_1:   
	  //Append_a_row_SLL_protected_1( c_val, c_col, c_row, i, vec_val_1, vec_index_1, vec_size_1, &c_row_max);
	  break;
	case COO_PROTECTED_0:       
	  Append_a_row_COO_protected_0( c_val_col_row, vec_val_1, vec_index_1, vec_size_1_pr, c_row_max); break;
	case COO_PROTECTED_1:      // Pick_a_row_COO_protected_1( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); 
	  break;
	case ELLPACK_PROTECTED_0:   
	  Append_a_row_ELLPACK( c_val_row, c_index_row, vec_val_1, vec_index_1, vec_size_1_pr, (*c_col_max), c_row_max);
	  /*int k;
	  for (k = 0; k < (*c_row_max); k++){
	    printf("row:%d\n",k);
	    printVecI(c_val_row[k], (*c_col_max));
	    printVecI(c_index_row[k], (*c_col_max) + 1);
	  }
	  */
	break;
      case ELLPACK_PROTECTED_1:   
	//Append_a_row_ELLPACK_protected_1( c_val_row, c_index_row, i, vec_val_1, vec_index_1, &vec_size_1, &c_row_max); 
	break;
	case LIL_PROTECTED_0:       
	Append_a_row_LIL_protected_0( c_row_list, c_LIL_nodes, vec_val_1, vec_index_1, vec_size_1_pr, c_row_max);/*
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
	//Append_a_row_LIL_protected_1( c_row_list, c_LIL_nodes, c_col_max, i, vec_val_1, vec_index_1, &vec_size_1, *c_row_max);
	break;
     }
    }
}

void matrix_permute_cols(int a_val[], int a_col[], int a_row[], int a_rowPtr[], int a_val_col_row[ NZrowMax + 1 ][ 3 ], int a_val_row[ ROWS ][ NZrowMax ], int a_index_row[ ROWS ][ NZrowMax + 1], val_col_node * a_row_list[], int a_row_max, int a_col_max, int c_val[], int c_col[], int c_row[], int c_rowPtr[], int c_val_col_row[ NZrowMax + 1 ][ 3 ], int c_val_row[ ROWS ][ NZrowMax ], int c_index_row[ ROWS ][ NZrowMax + 1], val_col_node * c_row_list[], int *c_row_max, int *c_col_max, int perm_order[]){
  int j, p_j;
  
  int vec_val_1[ VEC_MAX ];
  int vec_index_1[ VEC_MAX ];
  int vec_size_1;
  int vec_size_1_pr;
  
    for (j = 0; j < a_col_max; j++){
      printf("%d\n",j);

      p_j = perm_order[j];//---- mem accecss? or all functional

      //---- Pick col p_j of a
      switch (SPARSE){
      case CRS_PROTECTED_0:
	Pick_a_col_CRS_protected_0( a_val, a_col, a_rowPtr, a_row_max ,p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case CRS_PROTECTED_1:
	//Pick_a_col_CRS_protected_1( a_val, a_col, a_rowPtr, a_row_max,  p_j,  a_col_max, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case SLL_PROTECTED_0:   
	Pick_a_col_SLL_protected_0( a_val, a_col, a_row, a_row_max, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case SLL_PROTECTED_1:   
	//Pick_a_col_SLL_protected_1( a_val, a_col, a_row, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case COO_PROTECTED_0:       
	Pick_a_col_COO_protected_0( a_val_col_row, a_row_max, p_j, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case COO_PROTECTED_1:       
	//Pick_a_row_COO_protected_1( a_val_col_row, p_i, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case ELLPACK_PROTECTED_0:   
	Pick_a_col_ELLPACK_protected_0( a_val_row, a_index_row, a_row_max, a_col_max, p_j, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case ELLPACK_PROTECTED_1:   
	//	Pick_a_col_ELLPACK_protected_1( a_val_row, a_index_row, a_col_max, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case LIL_PROTECTED_0:       
	Pick_a_col_LIL_protected_0( a_row_list, a_row_max, a_col_max, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case LIL_PROTECTED_1:       
	//Pick_a_col_LIL_protected_0( a_row_list, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
	}
      vec_size_1_pr = max( 0, min( vec_size_1, VEC_MAX ));
      
      /* //if (i == 0){
      printf("\ncol: %d\n", p_j);
	printVecI(vec_val_1, vec_size_1_pr);
	printVecI(vec_index_1, vec_size_1_pr);
	//}
	*/  
      
      //---- Append col p_j to c
	*c_row_max = a_row_max;
	switch (SPARSE){
	case CRS_PROTECTED_0:
	  Append_a_col_CRS_protected_0(c_val, c_col, c_rowPtr, *c_row_max, c_col_max, vec_val_1, vec_index_1, vec_size_1_pr);
	  break;
	case CRS_PROTECTED_1:
	  //Append_a_col_CRS_protected_1(c_val, c_col, c_rowPtr, vec_val_1, vec_index_1, vec_size_1_pr, c_row_max);
	  break;
	case SLL_PROTECTED_0:   
	  Append_a_col_SLL_protected_0( c_val, c_col, c_row, *c_row_max, c_col_max, vec_val_1, vec_index_1, vec_size_1_pr);
	  break;
	case SLL_PROTECTED_1:   
	  //Append_a_col_SLL_protected_1( c_val, c_col, c_row, i, vec_val_1, vec_index_1, vec_size_1, &c_row_max);
	  break;
	case COO_PROTECTED_0:       
	  Append_a_col_COO_protected_0( c_val_col_row, *c_row_max, c_col_max, vec_val_1, vec_index_1, vec_size_1_pr); break;
	case COO_PROTECTED_1:      // Pick_a_row_COO_protected_1( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); 
	  break;
	case ELLPACK_PROTECTED_0:   
	  Append_a_col_ELLPACK_protected_0( c_val_row, c_index_row, *c_row_max, c_col_max, vec_val_1, vec_index_1, vec_size_1_pr);
	  /*int k;
	  for (k = 0; k < (*c_row_max); k++){
	    printf("row:%d\n",k);
	    printVecI(c_val_row[k], (*c_col_max));
	    printVecI(c_index_row[k], (*c_col_max) + 1);
	  }
	  */
	  break;
	case ELLPACK_PROTECTED_1:   
	  //Append_a_col_ELLPACK_protected_1( c_val_row, c_index_row, i, vec_val_1, vec_index_1, &vec_size_1, &c_row_max); 
	  break;
	case LIL_PROTECTED_0:       
	  Append_a_col_LIL_protected_0( c_row_list, c_LIL_nodes, *c_row_max, c_col_max, vec_val_1, vec_index_1, vec_size_1_pr);/*
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
	  /*break;
      case LIL_PROTECTED_1:       
	//Append_a_col_LIL_protected_1( c_row_list, c_LIL_nodes, c_col_max, i, vec_val_1, vec_index_1, &vec_size_1, *c_row_max);
	break;*/
	}
	/*
	int i;
	printf("c:\n");
	for(i = 0; i < 10; i++){
	  printVecI(c_val_col_row[i], 3);
	  printf("\n");
	}
	*/
    }
}

void sub_matrix(int a_val[], int a_col[], int a_row[], int a_rowPtr[], int a_val_col_row[ NZrowMax + 1 ][ 3 ], int a_val_row[ ROWS ][ NZrowMax ], int a_index_row[ ROWS ][ NZrowMax + 1], val_col_node * a_row_list[], int *a_row_max_ptr, int *a_col_max_ptr, int sub_range[]){
  int i, j, k;
  
  int i_s = sub_range[0];
  int i_e = sub_range[1];
  int j_s = sub_range[2];
  int j_e = sub_range[3];

  for (i = ROWS - 1; i >=0 ; i--){
    if ( i_s <= i && i <= i_e)
      continue;
    printf("%d\n",i);
    
    //---- remove row i of a
    switch (SPARSE){
    case DENSE_PROTECTED_0:
      Remove_a_row_DENSE_protected_0( a_dense, i, a_row_max_ptr);
      break;
    case CRS_PROTECTED_0:
      Remove_a_row_CRS_protected_0( a_val, a_col, a_rowPtr, i, a_row_max_ptr);
      //printf("i is %d\n",i);
      //printVecI(a_val, a_rowPtr[*a_row_max_ptr]);
      //printVecI(a_col, a_rowPtr[*a_row_max_ptr]);
      //printVecI(a_rowPtr, *a_row_max_ptr);
  
      break;
    case CRS_PROTECTED_1:
      break;
    case SLL_PROTECTED_0:   
      Remove_a_row_SLL_protected_0( a_val, a_col, a_row, i, a_row_max_ptr);
      /*
      printf("i is %d\n",i);
      printVecI(a_val, 9);
      printVecI(a_col, 9);
      printVecI(a_row, 9);*/
   break;
    case SLL_PROTECTED_1:   
      break;
    case COO_PROTECTED_0:       
      Remove_a_row_COO_protected_0( a_val_col_row, i, a_row_max_ptr); 
      break;
    case COO_PROTECTED_1:       
      break;
    case ELLPACK_PROTECTED_0:   
      Remove_a_row_ELLPACK_protected_0( a_val_row, a_index_row, i, a_row_max_ptr, *a_col_max_ptr); 
      /*
      printf("i is %d\n",i);
      for (k = 0; k < *a_row_max_ptr; k++){
	printVecI(a_val_row[k], *a_col_max_ptr);
	printVecI(a_index_row[k], *a_col_max_ptr);
	}*/
      break;
    case ELLPACK_PROTECTED_1:   
      break;
    case LIL_PROTECTED_0:       
      Remove_a_row_LIL_protected_0( a_row_list, a_LIL_nodes, i, a_row_max_ptr);
      break;
    case LIL_PROTECTED_1:       
      break;
    }
  }
  
  //------- remove the columns ------------//
  for (j = COLS - 1; j >= 0; j--){
    if (j_s <= j && j <= j_e)
      continue;
    printf("j: %d\n",j);
    
    //---- remove col j of a
    switch (SPARSE){
    case DENSE_PROTECTED_0:
      Remove_a_col_DENSE_protected_0( a_dense, *a_row_max_ptr, j, a_col_max_ptr);
      break;
    case CRS_PROTECTED_0:
      Remove_a_col_CRS_protected_0( a_val, a_col, a_rowPtr, *a_row_max_ptr, j, a_col_max_ptr);
      /*
      printf("j is %d\n",j);
      printVecI(a_val, a_rowPtr[*a_row_max_ptr]);
      printVecI(a_col, a_rowPtr[*a_row_max_ptr]);
      printVecI(a_rowPtr, *a_row_max_ptr);*/
      break;
    case CRS_PROTECTED_1:
      break;
    case SLL_PROTECTED_0:   
      Remove_a_col_SLL_protected_0( a_val, a_col, a_row, *a_row_max_ptr, j, a_col_max_ptr);
      /*
      printf("j is %d\n",j);
      printVecI(a_val, 9);
      printVecI(a_col, 9);
      printVecI(a_row, 9);*/
      break;
    case SLL_PROTECTED_1:   
      break;
    case COO_PROTECTED_0:       
      Remove_a_col_COO_protected_0( a_val_col_row, *a_row_max_ptr, j, a_col_max_ptr); 
      break;
    case COO_PROTECTED_1:       
      break;
    case ELLPACK_PROTECTED_0:   
      Remove_a_col_ELLPACK_protected_0( a_val_row, a_index_row, *a_row_max_ptr, j, a_col_max_ptr); 
      /*
      printf("j is %d\n",j);
      for (k = 0; k < *a_row_max_ptr; k++){
	printVecI(a_val_row[k], *a_col_max_ptr);
	printVecI(a_index_row[k], *a_col_max_ptr);
	}*/
      break;
    case ELLPACK_PROTECTED_1:   
      break;
    case LIL_PROTECTED_0:       
      Remove_a_col_LIL_protected_0( a_row_list, a_LIL_nodes, *a_row_max_ptr, j, a_col_max_ptr);
      break;
    case LIL_PROTECTED_1:       
      break;
    }
  }
}


void matrix_diagonal_add(int a_val[], int a_col[], int a_row[], int a_rowPtr[], int a_val_col_row[ NZrowMax + 1 ][ 3 ], int a_val_row[ ROWS ][ NZrowMax ], int a_index_row[ ROWS ][ NZrowMax + 1], val_col_node * a_row_list[], int a_row_max, int a_col_max, int diag_vec_val[], int diag_vec_index[], int diagSize){

  
  int vec_val_1[ VEC_MAX ], vec_val_3[ VEC_MAX ];
  int vec_index_1[ VEC_MAX ], vec_index_3[ VEC_MAX ];
  int vec_size_1, vec_size_3;
  int vec_size_1_pr, vec_size_3_pr;
  
      //---- Pick diagonal of a
      switch (SPARSE){
      case CRS_PROTECTED_0:
	Pick_diag_CRS_protected_0( a_val, a_col, a_rowPtr, a_row_max , vec_val_1, vec_index_1, &vec_size_1);
	break;
      case CRS_PROTECTED_1:
	//Pick_a_col_CRS_protected_1( a_val, a_col, a_rowPtr, a_row_max,  p_j,  a_col_max, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case SLL_PROTECTED_0:   
	Pick_diag_SLL_protected_0( a_val, a_col, a_row, a_row_max, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case SLL_PROTECTED_1:   
	//Pick_a_col_SLL_protected_1( a_val, a_col, a_row, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case COO_PROTECTED_0:       
	Pick_diag_COO_protected_0( a_val_col_row, a_row_max, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case COO_PROTECTED_1:       
	//Pick_a_row_COO_protected_1( a_val_col_row, p_i, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case ELLPACK_PROTECTED_0:   
	Pick_diag_ELLPACK_protected_0( a_val_row, a_index_row, a_row_max, a_col_max, vec_val_1, vec_index_1, &vec_size_1); 
	break;
      case ELLPACK_PROTECTED_1:   
	//	Pick_a_col_ELLPACK_protected_1( a_val_row, a_index_row, a_col_max, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case LIL_PROTECTED_0:       
	Pick_diag_LIL_protected_0( a_row_list, a_row_max, a_col_max, vec_val_1, vec_index_1, &vec_size_1);
	break;
      case LIL_PROTECTED_1:       
	//Pick_a_col_LIL_protected_0( a_row_list, p_j, vec_val_1, vec_index_1, &vec_size_1);
	break;
	}
      vec_size_1_pr = max( 0, min( vec_size_1, VEC_MAX ));
      
       //if (i == 0){
      //printf("\ndiag: %d\n");
      //printVecI(vec_val_1, vec_size_1_pr);
      //printVecI(vec_index_1, vec_size_1_pr);
	//}
	
      //---- Vector operation: diag + vec--> vec3
	spr_vectors_op(vec_val_1, vec_index_1, vec_size_1_pr, diag_vec_val, diag_vec_index, diagSize, vec_val_3, vec_index_3, &vec_size_3, '+' );

      vec_size_3_pr = max( 0, min( vec_size_3, VEC_MAX ));
      //printf("file:\n");
      //printVecI(diag_vec_val, diagSize);
      //printVecI(diag_vec_index, diagSize);
	      

      //printVecI(vec_val_3, vec_size_3_pr);
      //printVecI(vec_index_3, vec_size_3_pr);
      int i2 =0;
      //---- Update diagonal 
	
	switch (SPARSE){
	case CRS_PROTECTED_0:
	    Update_diag_CRS_protected_0(a_val, a_col, a_rowPtr, a_row_max, vec_val_3, vec_index_3, vec_size_3_pr);
	  break;
	case CRS_PROTECTED_1:
	  //Append_a_col_CRS_protected_1(c_val, c_col, c_rowPtr, vec_val_1, vec_index_1, vec_size_1_pr, c_row_max);
	  break;
	case SLL_PROTECTED_0:   
	  Update_diag_SLL_protected_0( a_val, a_col, a_row, a_row_max, vec_val_3, vec_index_3, vec_size_3_pr);
	  /*	  printf("OUT returned : \n");
	  printVecI(a_val, 20);
	  printVecI(a_col, 20);
	  printVecI(a_row, 20);
	  */
	  break;
	case SLL_PROTECTED_1:   
	  //Append_a_col_SLL_protected_1( c_val, c_col, c_row, i, vec_val_1, vec_index_1, vec_size_1, &c_row_max);
	  break;
	case COO_PROTECTED_0:       
	  //Update_diag_COO_protected_0( c_val_col_row, *c_row_max, c_col_max, vec_val_1, vec_index_1, vec_size_1_pr); break;
	case COO_PROTECTED_1:      // Pick_a_row_COO_protected_1( a_val_col_row, i, vec_val_1, vec_index_1, &vec_size_1); 
	  break;
	case ELLPACK_PROTECTED_0:
	  Update_diag_ELLPACK_protected_0( a_val_row, a_index_row, a_row_max, a_col_max, vec_val_3, vec_index_3, vec_size_3_pr);
	  /*
	  printf("row i after update :%d\n",i2);
	  printVecI(a_val_row[i2], NZrowMax);
	  printVecI(a_index_row[i2], NZrowMax + 1);
	  */
	  /*int k;
	  for (k = 0; k < (*c_row_max); k++){
	    printf("row:%d\n",k);
	    printVecI(c_val_row[k], (*c_col_max));
	    printVecI(c_index_row[k], (*c_col_max) + 1);
	  }
	  */
	  break;
	case ELLPACK_PROTECTED_1:   
	  //Append_a_col_ELLPACK_protected_1( c_val_row, c_index_row, i, vec_val_1, vec_index_1, &vec_size_1, &c_row_max); 
	  break;
	case LIL_PROTECTED_0:       
	  Update_diag_LIL( a_row_list, a_LIL_nodes, a_row_max, vec_val_3, vec_index_3, vec_size_3_pr);
/*
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
	//Append_a_col_LIL_protected_1( c_row_list, c_LIL_nodes, c_col_max, i, vec_val_1, vec_index_1, &vec_size_1, *c_row_max);
	break;
	}
	/*
	int i;
	printf("c:\n");
	for(i = 0; i < 10; i++){
	  printVecI(c_val_col_row[i], 3);
	  printf("\n");
	}
	*/
    
}


//====================== MAIN ===========================//
void main(){
  
  FILE * a_stream = fopen(INPUT_FILE_1, "r");
  //FILE * b_stream = fopen(INPUT_FILE_2, "r");
  FILE * ab_stream = fopen(INPUT_FILE_MUL, "r");
  
  //FILE * per_file = fopen(PERMUTE_FILE, "r");
  //FILE * diagonal_file = fopen(DIAGONAL_FILE, "r");
  FILE * sub_file = fopen(SUB_FILE, "r");

  //--------- Initialize for Sparse formats  --------------
  
  // --------- Initialize random generator --------
  //init_genrand(time(NULL));   

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
    //start_spr_addr[ Nspr ] = &(c_dense[0][0]);  end_spr_addr[ Nspr ] = &(c_dense[ROWS - 1][ COLS - 1 ]);
    //Nspr ++;
    break;
  case CRS_PROTECTED_0:
  case CRS_PROTECTED_1:
    processLineCRS( a_val, a_col, a_rowPtr, a_rowMax, a_colMax, a_stream); 
    
    //----- Add val, col, and rowPtr to the sparse addresses list
    start_spr_addr[ Nspr ] = &(a_val[0]);  end_spr_addr[ Nspr ] = &(a_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &(a_col[0]);  end_spr_addr[ Nspr + 1 ] = &(a_col[ NZmax - 1]);
    start_spr_addr[ Nspr + 2 ] = &(a_rowPtr[0]);  end_spr_addr[ Nspr + 2 ] = &(a_rowPtr[ ROWS ]);
    Nspr += 3;
    /*start_spr_addr[ Nspr ] = &(b_val[0]);  end_spr_addr[ Nspr ] = &(b_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &(b_col[0]);  end_spr_addr[ Nspr + 1 ] = &(b_col[ NZmax - 1]);
    start_spr_addr[ Nspr + 2 ] = &(b_rowPtr[0]);  end_spr_addr[ Nspr + 2 ] = &(b_rowPtr[ ROWS ]);
    Nspr += 3;
    start_spr_addr[ Nspr ] = &(c_val[0]);  end_spr_addr[ Nspr ] = &(c_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &(c_col[0]);  end_spr_addr[ Nspr + 1 ] = &(c_col[ NZmax - 1]);
    start_spr_addr[ Nspr + 2 ] = &(c_rowPtr[0]);  end_spr_addr[ Nspr + 2 ] = &(c_rowPtr[ ROWS ]);
    Nspr += 3;
  
    //----- Initialize
    c_rowPtr[0] = 0;*/
    break;
  
  case SLL_PROTECTED_0:
  case SLL_PROTECTED_1:
    processLineSLL( a_val, a_col, a_row, a_rowMax, a_colMax, a_stream); 

    //------- Add val, col, and row to the sparse addresses list
    start_spr_addr[ Nspr ] = &( a_val[0] );  end_spr_addr[ Nspr ] = &( a_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &( a_col[0] );  end_spr_addr[ Nspr + 1 ] = &( a_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &( a_row[0] );  end_spr_addr[ Nspr + 2 ] = &( a_row[ NZmax ]);
    Nspr += 3;/*
    start_spr_addr[ Nspr ] = &( b_val[0] );  end_spr_addr[ Nspr ] = &( b_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &( b_col[0] );  end_spr_addr[ Nspr + 1 ] = &( b_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &( b_row[0] );  end_spr_addr[ Nspr + 2 ] = &( b_row[ NZmax ]);
    Nspr += 3;
    start_spr_addr[ Nspr ] = &( c_val[0] );  end_spr_addr[ Nspr ] = &( c_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 1 ] = &( c_col[0] );  end_spr_addr[ Nspr + 1 ] = &( c_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &( c_row[0] );  end_spr_addr[ Nspr + 2 ] = &( c_row[ NZmax ]);
    Nspr += 3;

    //------- Initialize
    c_row[0] = ROWS;*/
  break;
  
  case COO_PROTECTED_0:
  case COO_PROTECTED_1:
    processLineCOO( a_val_col_row, a_rowMax, a_colMax, a_stream); 
    //----- Add val_col_row to the sparse addresses
    start_spr_addr[Nspr] = &(a_val_col_row[0][0]);  end_spr_addr[Nspr] = &(a_val_col_row[ NZmax ][2]);
    Nspr++;
    /*start_spr_addr[Nspr] = &(b_val_col_row[0][0]);  end_spr_addr[Nspr] = &(b_val_col_row[ NZmax ][2]);
      Nspr++;
    start_spr_addr[Nspr] = &(c_val_col_row[0][0]);  end_spr_addr[Nspr] = &(c_val_col_row[ NZmax ][2]);
    Nspr++;
    //------- Initialize
    c_val_col_row[0][2] = ROWS;*/
    break;
  
  case ELLPACK_PROTECTED_0:
  case ELLPACK_PROTECTED_1:
    processLineELLPACK( a_val_row, a_index_row, a_NZ_row, a_rowMax, a_colMax, a_stream); 

    //----- Add val_row and index_row to the sparse addresses list
    start_spr_addr[ Nspr ] = &(a_val_row[0][0]);  end_spr_addr[ Nspr ] = &(a_val_row[ ROWS - 1 ][ NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = &(a_index_row[0][0]);  end_spr_addr[ Nspr + 1 ] = &(a_index_row[ ROWS - 1 ][ NZrowMax ]);
    Nspr += 2;
    /*start_spr_addr[ Nspr ] = &(b_val_row[0][0]);  end_spr_addr[ Nspr ] = &(b_val_row[ ROWS - 1 ][ NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = &(b_index_row[0][0]);  end_spr_addr[ Nspr + 1 ] = &(b_index_row[ ROWS - 1 ][ NZrowMax ]);
    Nspr += 2;
    start_spr_addr[ Nspr ] = &(c_val_row[0][0]);  end_spr_addr[ Nspr ] = &(c_val_row[ ROWS - 1 ][ NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = &(c_index_row[0][0]);  end_spr_addr[ Nspr + 1 ] = &(c_index_row[ ROWS - 1 ][ NZrowMax ]);
    Nspr += 2;
  
    //------ Initialize 
    int i;
    for (i = 0; i< ROWS; i++)
      c_index_row[ i ][ 0 ] = COLS; 
    */
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
    /*
    start_spr_addr[ Nspr ] = (int *) &(b_LIL_nodes[0][0]);
    end_spr_addr[ Nspr ] = (int *) &(b_LIL_nodes[ROWS - 1][NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = (int *) &(b_row_list[ 0 ]);
    end_spr_addr[ Nspr + 1 ] = (int *) &(b_row_list[ ROWS - 1 ]);
    Nspr += 2;
    
    start_spr_addr[ Nspr ] = (int *) &(c_LIL_nodes[0][0]);
    end_spr_addr[ Nspr ] = (int *) &(c_LIL_nodes[ROWS - 1][NZrowMax - 1]);
    start_spr_addr[ Nspr + 1 ] = (int *) &(c_row_list[ 0 ]);
    end_spr_addr[ Nspr + 1 ] = (int *) &(c_row_list[ ROWS - 1 ]);
    Nspr += 2;*/
    break;
  case JAD_PROTECTED_0:
  case JAD_PROTECTED_1:
    processLineJAD( a_JADptr, a_val, a_col, a_val_row, a_index_row, a_NZ_row, a_rowMax, a_colMax, a_row_order, a_stream);

    //----- Add val_row and index_row to the sparse addresses list
    start_spr_addr[ Nspr ] = &(a_JADptr[0]);  end_spr_addr[ Nspr ] = &(a_JADptr[ NZcolMax ]);
    start_spr_addr[ Nspr + 1 ] = &(a_val[0]);  end_spr_addr[ Nspr + 1 ] = &(a_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &(a_col[0]);  end_spr_addr[ Nspr + 2 ] = &(a_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 3 ] = &(a_row_order[0]);  end_spr_addr[ Nspr + 3 ] = &(a_row_order[ ROWS - 1 ]);
    Nspr += 4;
    /*
    start_spr_addr[ Nspr ] = &(b_JADptr[0]);  end_spr_addr[ Nspr ] = &(b_JADptr[ NZcolMax ]);
    start_spr_addr[ Nspr + 1 ] = &(b_val[0]);  end_spr_addr[ Nspr + 1 ] = &(b_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &(b_col[0]);  end_spr_addr[ Nspr + 2 ] = &(b_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 3 ] = &(b_row_order[0]);  end_spr_addr[ Nspr + 3 ] = &(b_row_order[ ROWS - 1 ]);
    Nspr += 4;
    
    start_spr_addr[ Nspr ] = &(c_JADptr[0]);  end_spr_addr[ Nspr ] = &(c_JADptr[ NZcolMax ]);
    start_spr_addr[ Nspr + 1 ] = &(c_val[0]);  end_spr_addr[ Nspr + 1 ] = &(c_val[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 2 ] = &(c_col[0]);  end_spr_addr[ Nspr + 2 ] = &(c_col[ NZmax - 1 ]);
    start_spr_addr[ Nspr + 3 ] = &(c_row_order[0]);  end_spr_addr[ Nspr + 3 ] = &(c_row_order[ ROWS - 1 ]);
    Nspr += 4;*/
    break;
  }

  //printf("err_pr2:%d\n", ERR_PR);
  /*
  fscanf(b_stream, "%d\n", &b_rowMax);
  fscanf(b_stream, "%d\n", &b_colMax);
  switch(SPARSE){
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
  }
  */
  //---- Read the result ab = a * b or ab= a + b
  fscanf(ab_stream, "%d\n", &ab_rowMax);
  fscanf(ab_stream, "%d\n", &ab_colMax);
  int ** ab_dense = matrix( ab_rowMax, ab_colMax);
  processLine( ab_dense, ab_rowMax, ab_colMax, ab_stream);
  
  //--- Matrix Multiplication : c = a * b
  int ** c = matrix( ab_rowMax, ab_colMax);
 
  //  matrix_accumulation(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, a_rowMax, a_colMax, b_val, b_col, b_row, b_rowPtr, b_val_col_row, b_val_row, b_index_row, b_row_list, b_rowMax, b_colMax, c_val, c_col, c_row, c_rowPtr, c_val_col_row, c_val_row, c_index_row, c_row_list, &c_rowMax, &c_colMax);

  //  matrix_multiplication(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, a_rowMax, a_colMax, b_val, b_col, b_row, b_rowPtr, b_val_col_row, b_val_row, b_index_row, b_row_list, b_rowMax, b_colMax, c);

  //int diag_val_vec[ROWS];
  //int diag_index_vec[ROWS];
  //int diag_size = 0;
  //  processLineSprVec(diag_val_vec, diag_index_vec, &diag_size,  diagonal_file);

  int sub_range [ 4 ];
  processLineVec(sub_range, 4 ,sub_file);
  //  matrix_permute_rows(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, a_rowMax, a_colMax, c_val, c_col, c_row, c_rowPtr, c_val_col_row, c_val_row, c_index_row, c_row_list, &c_rowMax, &c_colMax, per_order);
  //matrix_permute_cols(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, a_rowMax, a_colMax, c_val, c_col, c_row, c_rowPtr, c_val_col_row, c_val_row, c_index_row, c_row_list, &c_rowMax, &c_colMax, per_order);
  //  matrix_diagonal_add(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, a_rowMax, a_colMax, diag_val_vec, diag_index_vec, diag_size);
  
  sub_matrix(a_val, a_col, a_row, a_rowPtr, a_val_col_row, a_val_row, a_index_row, a_row_list, &a_rowMax, &a_colMax, sub_range);
  
 printf("a_rowMax: %d, a_colMax:%d\n", a_rowMax, a_colMax);
//void matrix_permute_cols(int a_val[], int a_col[], int a_row[], int a_rowPtr[], int a_val_col_row[ NZrowMax + 1 ][ 3 ], int a_val_row[ ROWS ][ NZrowMax ], int a_index_row[ ROWS ][ NZrowMax + 1], val_col_node * a_row_list[], int a_row_max, int a_col_max, int c_val[], int c_col[], int c_row[], int c_rowPtr[], int c_val_col_row[ NZrowMax + 1 ][ 3 ], int c_val_row[ ROWS ][ NZrowMax ], int c_index_row[ ROWS ][ NZrowMax + 1], val_col_node * c_row_list[], int *c_row_max, int *c_col_max, int perm_order[]){
/*
  printf("MAIN returned : \n");
  printVecI(a_val, 20);
  printVecI(a_col, 20);
  printVecI(a_row, 20);
*/
switch(SPARSE){
 case DENSE_PROTECTED_0:
 case DENSE_PROTECTED_1:
   Sprs_to_dense_DENSE(a_dense, a_rowMax, a_colMax, c);
   break;
  case CRS_PROTECTED_0:
  case CRS_PROTECTED_1:
      Sprs_to_dense_CRS(a_val, a_col, a_rowPtr, ab_rowMax, ab_colMax, c); 
    break; 
  case SLL_PROTECTED_0:
  case SLL_PROTECTED_1:
    Sprs_to_dense_SLL( a_val, a_col, a_row, ab_rowMax, ab_colMax, c); 
    break;
  case COO_PROTECTED_0:
  case COO_PROTECTED_1:
    Sprs_to_dense_COO( a_val_col_row, ab_rowMax, ab_colMax, c); break;
   break;
  
  case ELLPACK_PROTECTED_0:
  case ELLPACK_PROTECTED_1:
    Sprs_to_dense_ELLPACK( a_val_row, a_index_row, a_rowMax, a_colMax, c); 
    break;
  
  case LIL_PROTECTED_0:
  case LIL_PROTECTED_1:
     Sprs_to_dense_LIL( a_row_list, a_rowMax, a_colMax, c); 
    break;
  
 case JAD_PROTECTED_0:
 case JAD_PROTECTED_1:
   Sprs_to_dense_JAD( a_val, a_col, a_row_order, a_JADptr, a_rowMax, a_colMax, c); 
   break;
  }
  

//printf("c:\n");
//printMatrix(c, a_rowMax, a_colMax, 0);
  /*  
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
 //fclose(b_stream);
 fclose(ab_stream);
 // fclose(diagonal_file);
fclose(sub_file);

 return;
}

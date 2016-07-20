#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <math.h>
#include <definitions.h>
#include<inttypes.h>
//========== GLOBAL VARIABLES =============//
double rand_twister(void);

int err_num = 0;

int last_read = 0;
long long last_read_long = 0;
int last_write = 0;
long long last_write_long = 0;

long long read_num = 0;
long long write_num = 0;
long long out_read = 0;
long long out_write = 0;



extern int Nspr;
extern int * start_spr_addr[20];
extern int * end_spr_addr[20];
//========= ERROR INJECTION =============//
int in_spr_bound( int * ptr){
  
  int n;
  for (n = 0; n < Nspr; n++){
    if ( (start_spr_addr[n] <= ptr) && (ptr <= end_spr_addr[n]))
      return 1;
  }
  return 0;
}


int inboundry( int * intaddress){
  if (intaddress != NULL)  
    return 1;
  else 
    return 0;
}

int inboundryf( float * fladdress){
  if (fladdress != NULL)  
    return 1;
  else 
    return 0;
}


int inject_error(int A[], int **Aptr, val_col_node A_node[], val_col_node **row_list_node , int size_A, double err_probability, int node_type){

  int i;
  int injected_err = 0;
  double R;
  int is_err;
  
  if (err_probability ==0)
    return 0;

  for (i = 0 ; i < size_A ; i++){

    //------- Rand decides err or not to A[i] --------//
    R = RAND;
    is_err = (R < err_probability)? 1: 0;
    
   if (is_err){

     //-----  Rand decides which bit of A[i] to flip-> bit_n ------//    
     int bit_n =(int) (RAND * (sizeof(int) * 8 + 1));
     int bit_n_64 = (int) (RAND * (65));
     //----- Flip bit_n of A[i] --------------------//
      injected_err ++;

      if (node_type == INT_POINTER){
	long long temp_node = (long long) (* Aptr);
        //long long temp_node2 = temp_node;
	temp_node = temp_node ^ (1LLU << bit_n_64);
	*Aptr = (int *)temp_node;
	//if (temp_node != temp_node2)
	//printf("tm changed\n");
	//else
	//printf("no change %lld, %d\n", (1LLU << bit_n_64), bit_n_64);
        }
      else if (node_type == INT)
        A[i] = A[i] ^ (1 << bit_n);
    }
  }
  return injected_err;
}

int inject_error_old(int A[], int **Aptr, val_col_node A_node[], val_col_node **row_list_node , int size_A, double err_probability, int node_type){

  int i;
  int injected_err = 0;
  double R;
  int is_err;
  
  if (err_probability ==0)
    return;

  for (i = 0 ; i < size_A ; i++){

    //------- Rand decides err or not to A[i] --------//
    R = RAND;
    is_err = (R < err_probability)? 1: 0;
    
   if (is_err){

     //-----  Rand decides which bit of A[i] to flip-> bit_n ------//    
     int bit_n =(int) (RAND * (sizeof(int) * 8 + 1));
     int bit_n_64 = (int) (RAND * (65));
     //----- Flip bit_n of A[i] --------------------//
      injected_err ++;

      if (node_type == VAL)
        A_node[i].val = (A_node[i]).val ^ (1 << bit_n);
      else if (node_type == COL)
        A_node[i].col = (A_node[i]).col ^ (1 << bit_n);
      else if (node_type == nxtPOINTER) {
        //A_node[i].next = NULL;//reconsider***
        long temp_node;
	temp_node = (long) (A_node[i].next);
        temp_node = temp_node ^ (1LLU << bit_n_64);
        A_node[i].next = (struct val_col_node *) temp_node;
      }
      else if (node_type == VCN_POINTER){
        //row_list_node[i] = NULL;//reconsider
        long temp_node;
        temp_node = (long) (row_list_node[i]);
        temp_node = temp_node ^ (1LLU << bit_n_64);
        row_list_node[i] = (struct val_col_node *) temp_node;
      }
      else if (node_type == INT_POINTER){
	/*int * temp_A = *Aptr;
	long temp_node;
        temp_node = (long) (temp_A);
        temp_node = temp_node ^ (one_64 << bit_n_64);
	temp_A = (int *) temp_node;
        *Aptr = temp_A;*/
        long long temp_node = (long long) (* Aptr);
        long long temp_node2 = temp_node;
	temp_node = temp_node ^ (1LLU << bit_n_64);
	//if (temp_node != temp_node2)
	//printf("tm changed\n");
	//else
	//printf("no change %lld, %d\n", (1LLU << bit_n_64), bit_n_64);
        *Aptr = (int *)temp_node;

      }
      else if (node_type == INT)
        A[i] = A[i] ^ (1 << bit_n);
    }//if is_err
  }
  return injected_err;
}

void mem_read_int( int * dest_address_int, int * start_address_int, int N_address){
  int * start_address_int_er = start_address_int;
  int * dest_address_int_inc = dest_address_int;

  if (N_address <= 0)
    return;

  read_num += N_address;

  //--- address is read from address generator => error injection
  err_num += inject_error(NULL, &start_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, read the data
    if (in_spr_bound( start_address_int_er )){//---- protection 0
      *(dest_address_int_inc) = *( start_address_int_er);
      last_read_long =(int) *( start_address_int_er);
    }
    else{
      *(dest_address_int_inc) = (int)last_read_long;
      out_read++;
    }
    //---- 3. error bc of READ. (data is passed to functional block => error)
    err_num += inject_error( dest_address_int_inc, NULL, NULL, NULL, 1, ERR_PR, INT);

    start_address_int_er = start_address_int_er + 1;
    dest_address_int_inc = dest_address_int_inc + 1;

  }
}

void mem_write_int( int * dest_address_int, int * start_address_int, int N_address){
  int * dest_address_int_er = dest_address_int;
  int * start_address_int_inc = start_address_int;

  if (N_address <= 0)
    return;
  write_num += N_address;
   
  //--- destination address is read from address generator => error injection 
  err_num += inject_error(NULL, &dest_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, write the data
    if (in_spr_bound( dest_address_int_er )){//---- protection 0
    //if (1){  
      *(dest_address_int_er) = *( start_address_int_inc);
      last_write = *( start_address_int_inc);

      //---- 3. error bc of WRITE. (data is written to memory => error)
      err_num += inject_error( dest_address_int_er, NULL, NULL, NULL, 1, ERR_PR, INT);
    }
    else
      out_write++;

    start_address_int_inc = start_address_int_inc + 1;
    dest_address_int_er = dest_address_int_er + 1;

  }
}

void mem_write_int_p1_sch2( int * dest_address_int, int * start_address_int, int N_address, int data_min, int data_max){
  int * dest_address_int_er = dest_address_int;
  int * start_address_int_inc = start_address_int;

  if (N_address <= 0)
    return;
  
  //--- destination address is read from address generator => error injection 
  err_num += inject_error(NULL, &dest_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, write the data
    if (in_spr_bound( dest_address_int_er )){//---- protection 0
      *(dest_address_int_er) = *( start_address_int_inc);
  
    //---- 3. protection 1 
      if (*(dest_address_int_er) > data_max || *(dest_address_int_er) < data_min)
	*(dest_address_int_er) = data_min;
      else
	last_write = *( dest_address_int_er);

    //---- 4. error bc of WRITE. (data is written to memory => error)
      err_num += inject_error( dest_address_int_er, NULL, NULL, NULL, 1, ERR_PR, INT);

    }
    start_address_int_inc = start_address_int_inc + 1;
    dest_address_int_er = dest_address_int_er + 1;
  }
}

void mem_write_int_p1( int * dest_address_int, int * start_address_int, int N_address, int data_min, int data_max){
  int * dest_address_int_er = dest_address_int;
  int * start_address_int_inc = start_address_int;

  if (N_address <= 0)
    return;
  
  //--- destination address is read from address generator => error injection 
  err_num += inject_error(NULL, &dest_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, write the data
    if (in_spr_bound( dest_address_int_er )){//---- protection 0
      *(dest_address_int_er) = *( start_address_int_inc);
  
    //---- 3. protection 1 
      if (*(dest_address_int_er) > data_max || *(dest_address_int_er) < data_min)
      	*(dest_address_int_er) = last_write;
      else
	last_write = *( dest_address_int_er);

    //---- 4. error bc of WRITE. (data is written to memory => error)
      err_num += inject_error( dest_address_int_er, NULL, NULL, NULL, 1, ERR_PR, INT);

    }
    start_address_int_inc = start_address_int_inc + 1;
    dest_address_int_er = dest_address_int_er + 1;
  }
}

void mem_write_int_p1_sch3( int * dest_address_int, int * start_address_int, int N_address, int data_min, int data_max){
  
  if (N_address <= 0)
    return;

  int * dest_address_int_er;
  int * start_address_int_inc;
  int i, k;

  for (k = 0; k < 2; k++){

    //--- destination address is read from address generator => error injection 
    dest_address_int_er = dest_address_int;
    start_address_int_inc = start_address_int;

    err_num += inject_error(NULL, &dest_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

    for (i = 0; i < N_address; i++){
      //---- 2. if in bound, write the data
      if (in_spr_bound( dest_address_int_er )){//---- protection 0
	*(dest_address_int_er) = *( start_address_int_inc);
     
	//---- 3. protection 1 
	if (*(dest_address_int_er) > data_max || *(dest_address_int_er) < data_min)
	  *(dest_address_int_er) = data_min;
	else{
	  last_write = *( dest_address_int_er);
	  k = 2;
	}

	//---- 4. error bc of WRITE. (data is written to memory => error)
	err_num += inject_error( dest_address_int_er, NULL, NULL, NULL, 1, ERR_PR, INT);
      }
      start_address_int_inc = start_address_int_inc + 1;
      dest_address_int_er = dest_address_int_er + 1;
    }
  }
}

int * address_generator_int(int i, int row_length, int j, int * Addr){
  int er_i = i;
  int er_j = j;
  int er_row_length = row_length;
  int * start_address_int = NULL;
  
  //--- index is read from the functional block=> error injection
  err_num += inject_error(&er_i, NULL, NULL, NULL, 1, ERR_PR, INT);
  err_num += inject_error(&er_j, NULL, NULL, NULL, 1, ERR_PR, INT);
  err_num += inject_error(&er_row_length, NULL, NULL, NULL, 1, ERR_PR, INT);
  
  start_address_int = Addr + er_i * row_length + er_j;
  return start_address_int;  
}

void mem_read_int_p1_sch1( int * dest_address_int, int * start_address_int, int N_address, int data_min, int data_max){
int * start_address_int_er = start_address_int;
  int * dest_address_int_inc = dest_address_int;

  if (N_address <= 0)
    return;

  //--- address is read from address generator => error injection
  err_num += inject_error(NULL, &start_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, read the data
    if (in_spr_bound( start_address_int_er )){//---- protection 0
      *(dest_address_int_inc) = *( start_address_int_er);
      last_read = *( start_address_int_er);
    }
    else
      *(dest_address_int_inc) = last_read;
  
    //---- 3. error bc of READ. (data is passed to functional block => error)
    err_num += inject_error( dest_address_int_inc, NULL, NULL, NULL, 1, ERR_PR, INT);

    //---- 4. protection 1 and update the last_read
    if (*(dest_address_int + i) > data_max || *(dest_address_int + i) < data_min)
      *(dest_address_int + i) = last_read;
    else
      last_read = *(dest_address_int + i);
    
    start_address_int_er = start_address_int_er + 1;
    dest_address_int_inc = dest_address_int_inc + 1;

  }
}

void mem_read_int_p1( int * dest_address_int, int * start_address_int, int N_address, int data_min, int data_max){
int * start_address_int_er = start_address_int;
  int * dest_address_int_inc = dest_address_int;

  if (N_address <= 0)
    return;

  //--- address is read from address generator => error injection
  err_num += inject_error(NULL, &start_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, read the data
    if (in_spr_bound( start_address_int_er )){//---- protection 0
      *(dest_address_int_inc) = *( start_address_int_er);
      last_read = *( start_address_int_er);
    }
    else
      *(dest_address_int_inc) = last_read;
  
    //---- 3. error bc of READ. (data is passed to functional block => error)
    err_num += inject_error( dest_address_int_inc, NULL, NULL, NULL, 1, ERR_PR, INT);

    //---- 4. protection 1 and update the last_read
    if (*(dest_address_int + i) > data_max || *(dest_address_int + i) < data_min)
      *(dest_address_int + i) = data_min;
    else
      last_read = *(dest_address_int + i);
    
    start_address_int_er = start_address_int_er + 1;
    dest_address_int_inc = dest_address_int_inc + 1;

  }
}

void mem_read_int_p1_sch3( int * dest_address_int, int * start_address_int, int N_address, int data_min, int data_max){
  int * start_address_int_er; 
  int * dest_address_int_inc; 
  int i, k;

  if (N_address <= 0)
    return;

  for (k = 0; k<2; k++){//---protection 1
//--- address is read from address generator => error injection    
  start_address_int_er = start_address_int;
  dest_address_int_inc = dest_address_int;

  err_num += inject_error(NULL, &start_address_int_er, NULL, NULL, 1, ERR_PR, INT_POINTER);

  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, read the data
    if (in_spr_bound( start_address_int_er )){//---- protection 0
      *(dest_address_int_inc) = *( start_address_int_er);
      last_read = *( start_address_int_er);
    }
    else
      *(dest_address_int_inc) = last_read;
  
    //---- 3. error bc of READ. (data is passed to functional block => error)
    err_num += inject_error( dest_address_int_inc, NULL, NULL, NULL, 1, ERR_PR, INT);

    //---- 4. protection 1 and update the last_read
    if (*(dest_address_int + i) > data_max || *(dest_address_int + i) < data_min)
      *(dest_address_int + i) = data_min;
    else
      k = 2;
    
    start_address_int_er = start_address_int_er + 1;
    dest_address_int_inc = dest_address_int_inc + 1;
  }
  }
}


int * address_generator_int_p1(int i, int i_min, int i_max, int row_length, int j, int j_min, int j_max, int * Addr){
  int er_i = i;
  int er_j = j;
  int er_row_length = row_length;
  int * start_address_int = NULL;
  int row_length_max = i_max - i_min + 1;
  
  //--- index is read from the functional block=> error injection
  err_num += inject_error(&er_i, NULL, NULL, NULL, 1, ERR_PR, INT);
  err_num += inject_error(&er_j, NULL, NULL, NULL, 1, ERR_PR, INT);
  err_num += inject_error(&er_row_length, NULL, NULL, NULL, 1, ERR_PR, INT);
  /*
  //--- protection 1
  er_i  = max( i_min, min (i_max, er_i));
  er_j  = max( j_min, min (j_max, er_j));
  er_row_length  = max( 0, min (row_length_max, er_row_length));
  */
  //---- Calculate the address
  if ( i_min <= er_i && er_i <= i_max && j_min <= er_j && er_j <= j_max && 0 <= row_length && row_length <= ( j_max - j_min + 1) )
    start_address_int = Addr + er_i * row_length + er_j;
  else {
    start_address_int = NULL;
    //    printf("no address p1\n");
  }
  return start_address_int;  
}

/*
int * address_generator_int_from_vcn(int offs, val_col_node * vcn_Addr){
  int er_offs = offs;
  int * start_address_int = NULL;
  
  //--- index is read from the functional block=> error injection
  err_num += inject_error(&er_offs, NULL, NULL, NULL, 1, ERR_PR, INT);
    
  start_address_int = (int *) (vcn_Addr + er_offs);
  return start_address_int;  
}
*/

void mem_read_ptrptr_vcn( val_col_node ** dest_address_ptrptr_vcn, val_col_node ** start_address_ptrptr_vcn, int N_address){

val_col_node ** start_address_inc = start_address_ptrptr_vcn;
  val_col_node ** dest_address_er = dest_address_ptrptr_vcn;

  if (N_address <= 0)
    return;

  read_num +=N_address;

  //--- address is read from address generator => error injection
  err_num += inject_error(NULL,(int **) &start_address_inc, NULL, NULL, 1, ERR_PR, INT_POINTER);
  
  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, write the data
    if (in_spr_bound( (int *) start_address_inc )){// reconsider
      *dest_address_er = *( (val_col_node **)start_address_inc);
      last_read_long = (long long)(*dest_address_er);
    
      //---- 3. error bc of READ. (data is read from memory => error)
      long long this_read = (long long)*dest_address_er;
      err_num += inject_error( NULL,(int **)&this_read, NULL, NULL, 1, ERR_PR, INT_POINTER);
      *dest_address_er = (val_col_node *)this_read;
    }
    else{
      *dest_address_er = (val_col_node *) last_read_long; 
      out_read++;
    }
    start_address_inc = start_address_inc + 1;
    dest_address_er = dest_address_er + 1;
  }
}

void mem_write_ptrptr_vcn( val_col_node ** dest_address_ptrptr_vcn, val_col_node ** start_address_ptrptr_vcn, int N_address){
   
  val_col_node ** start_address_inc = start_address_ptrptr_vcn;
  val_col_node ** dest_address_er = dest_address_ptrptr_vcn;

  if (N_address <= 0)
    return;

  write_num +=N_address;

  //--- address is read from address generator => error injection
  err_num += inject_error(NULL,(int **) &dest_address_er, NULL, NULL, 1, ERR_PR, INT_POINTER);
  
  int i;
  for (i = 0 ; i < N_address; i++){
    //---- 2. if in bound, write the data
    if (in_spr_bound( (int *) dest_address_er )){// reconsider
      *dest_address_er = *( (val_col_node **)start_address_inc);
      last_write_long = (long long)(*dest_address_er);
    
      //---- 3. error bc of WRITE. (data is written to memory => error)
      long long this_write = (long long)*dest_address_er;
      err_num += inject_error( NULL,(int **)&this_write, NULL, NULL, 1, ERR_PR, INT_POINTER);
      *dest_address_er = (val_col_node *)this_write;
    }
    else{
      out_write++;
      //pareesa
    }
    start_address_inc = start_address_inc + 1;
    dest_address_er = dest_address_er + 1;
  }
}



val_col_node ** address_generator_ptr_vcn(int i,  val_col_node ** ptr_Addr){
  int er_i = i;
  val_col_node ** start_address_ptr_vcn = NULL;
  
  //--- index is read from the functional block=> error injection
  err_num += inject_error(&er_i, NULL, NULL, NULL, 1, ERR_PR, INT);
  
  start_address_ptr_vcn = ptr_Addr + er_i;
  return start_address_ptr_vcn;  
}

//=========== FUNCs for STRUCTS ========================//
//-- if the node with the same i and j already exists, it updates the value
//-- other wise does nothing

void update_node(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1], int row_id, int old_col_id, int new_col_id){
 
  val_col_node * this_node = row_list[row_id];
  while (this_node->next != NULL){
    //---- if this value already exists, just update it
    if (this_node->col == old_col_id){
      //      this_node->val = new_in_data;
      this_node->col = new_col_id;
      return;
    }
    this_node = this_node->next;
  }
}

//-- if the node with the same i and j already exists, it updates the value
//-- other wise adds a node
void add_node(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1], int row_id, int col_id, int in_data){
  //val_col_node new_node;
  //new_node.val = rating;
  //new_node.col = col_id;
  //new_node.next = NULL;
  
  int nz_row = 0;
  if (row_list[row_id] == NULL){
    row_list[row_id] = &LIL_nodes[ row_id ][0];
  }
  else{
    nz_row++;
    val_col_node * this_node = row_list[row_id];
    while (this_node->next != NULL){
      //---- if this value already exists, just update it
      if (this_node->col == col_id){
	this_node->val = in_data;
	return;
      }
      this_node = this_node->next;
      nz_row++;
    }
    this_node->next = &(LIL_nodes[row_id][nz_row]);
  }
  (LIL_nodes[row_id][nz_row]).val = in_data;
  (LIL_nodes[row_id][nz_row]).col = col_id;
  (LIL_nodes[row_id][nz_row]).next = NULL;
}

void add_node_p0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1], int row_id, int col_id, int in_data){
  int nz_row = 0;
  val_col_node ** start_address_ptrptr_vcn;
  val_col_node * this_row_list;
  val_col_node * this_node, *this_node_next;

  int N_address, this_node_val, this_node_col;
  int * start_address_int;

  start_address_ptrptr_vcn = row_list + row_id;
  if (in_spr_bound( (int *) start_address_ptrptr_vcn ))
    this_node = *start_address_ptrptr_vcn;

  if (this_node == NULL){
    //if (row_list[row_id] == NULL){   
    // row_list[row_id] = &LIL_nodes[ row_id ][0];//--- this is just initialization
    if (in_spr_bound( (int *) start_address_ptrptr_vcn ))
      *start_address_ptrptr_vcn = &LIL_nodes[ row_id ][ 0 ];
    }
  else{
    //++++ address generator
    start_address_ptrptr_vcn = address_generator_ptr_vcn( row_id , row_list);
    N_address = 1;
    //++++ memory access
    mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);

    //++++ functional
    nz_row++;
    //this_node = row_list[row_id];
    //while (this_node->next != NULL){
    start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4 , (int *)this_node));
    N_address = 1;
    //++++ memory access
    mem_read_ptrptr_vcn( &this_node_next, start_address_ptrptr_vcn, N_address);
    while (this_node_next != NULL){
      //---- if this value already exists, just update it
      if (nz_row >= NZrowMax)
        break;
      //if (this_node->col == col_id){
      start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, col) / 4 , (int *)this_node);
      N_address = 1;
      //++++ memory access
      mem_read_int( &this_node_col, start_address_int, N_address);

      if (this_node_col == col_id){
	//this_node->val = in_data;
	start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, val) / 4 , (int *)this_node);
        N_address = 1;
        //++++ memory access
        mem_write_int(start_address_int, &in_data, N_address);

        return;
      }
      //------------ updating this_node -------------//
      //  this_node = this_node->next;
      start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next)/ 4 , (int *)this_node));
      N_address = 1;
      //++++ memory access
      mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);

      //---------- read this_node->next ------------//
      start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next)/ 4 , (int *)this_node));
      N_address = 1;
      //++++ memory access
      mem_read_ptrptr_vcn( &this_node_next, start_address_ptrptr_vcn, N_address);

      //++++ functional
      nz_row++;
    }
    //this_node->next = &(LIL_nodes[row_id][nz_row]);//---these are parts of initializations
    //start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next)/ 4 , (int *)this_node));
start_address_ptrptr_vcn = (struct val_col_node **)((int*)this_node + offsetof(val_col_node, next)/ 4);
if (in_spr_bound( (int *) start_address_ptrptr_vcn ))
  *start_address_ptrptr_vcn = &(LIL_nodes[row_id][nz_row]);

  }
//  (LIL_nodes[row_id][nz_row]).val = in_data;
  start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, val) / 4 , (int *)&(LIL_nodes[row_id][nz_row]));
  N_address = 1;
//++++ memory access
mem_write_int( start_address_int, &in_data, N_address);

// (LIL_nodes[row_id][nz_row]).col = col_id;
 start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, col) / 4 , (int *)&(LIL_nodes[row_id][nz_row]));
N_address = 1;
//++++ memory access
mem_write_int( start_address_int, &col_id, N_address);

//(LIL_nodes[row_id][nz_row]).next = NULL;
start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4, (int *) &LIL_nodes[row_id][nz_row]));
N_address = 1;
//++++ memory access
this_node_next = NULL;
mem_write_ptrptr_vcn(start_address_ptrptr_vcn, &this_node_next, N_address);
}


//--- it removes the node with given indexes if it exists, otherwise does nothing
void remove_node(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1], int row_id, int col_id){

  if (row_list[row_id] == NULL)
    return;

  val_col_node * this_node = row_list[row_id];
  val_col_node * removing_node = NULL;
    while (this_node->next!=NULL){
      if (this_node->col == col_id){//---- remove it
	removing_node = this_node;
      }
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

void remove_node_p0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1], int row_id, int col_id){

  int k = 0, N_address = 1;
  int this_node_val, this_node_col;
  int *start_address_int;
  val_col_node * this_node, * removing_node = NULL, *this_node_next, *temp_vcn;
  val_col_node **start_address_ptrptr_vcn;

    //++++ address generator
    start_address_ptrptr_vcn = address_generator_ptr_vcn( row_id , row_list);
    //++++ memory access
    mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);


  if (this_node == NULL)
    return;

  //    while (this_node->next!=NULL){
  //++++ address generator
  start_address_ptrptr_vcn =(struct val_col_node **) address_generator_int(0, 0, offsetof(val_col_node, next)/4 , (int *)this_node);
  //++++ memory access
  mem_read_ptrptr_vcn( &this_node_next, start_address_ptrptr_vcn, N_address);

  k = 0;
  while (this_node_next != NULL){
    k++;
    if(k > VEC_MAX)//---protection 0
      break;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, offsetof(val_col_node, col)/4 , (int *)this_node);
    //++++ memory access
    mem_read_int( &this_node_col, start_address_int, N_address);
  if (this_node_col == col_id){//---- remove it
	removing_node = this_node;
      }
  //---- Update this_node and this_node_next
  //this_node = this_node->next;
    //++++ address generator
  start_address_ptrptr_vcn =(struct val_col_node **) address_generator_int(0, 0, offsetof(val_col_node, next)/4 , (int *)this_node);
  //++++ memory access
  mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);
  
      //++++ address generator
      start_address_ptrptr_vcn =(struct val_col_node **) address_generator_int(0, 0, offsetof(val_col_node, next)/4 , (int *)this_node);
      //++++ memory access
      mem_read_ptrptr_vcn( &this_node_next, start_address_ptrptr_vcn, N_address);
    }

    //---replace it with the last item in the row
    if (removing_node != NULL){
      //removing_node->col = this_node->col;
      //++++ address generator
      start_address_int = address_generator_int(0, 0, offsetof(val_col_node, col)/4 , (int *)this_node);
      //++++ memory access
      mem_read_int( &this_node_col, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(0, 0, offsetof(val_col_node, col)/4 , (int *)removing_node);
      //++++ memory access
      mem_write_int( start_address_int, &this_node_col, N_address);

      //removing_node->val = this_node->val;
      //++++ address generator
      start_address_int = address_generator_int(0, 0, offsetof(val_col_node, val)/4 , (int *)this_node);
      //++++ memory access
      mem_read_int( &this_node_val, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(0, 0, offsetof(val_col_node, val)/4 , (int *)removing_node);
      //++++ memory access
      //++++ memory access
      mem_write_int( start_address_int, &this_node_val, N_address);

      //--- remove the last item
      //this_node = NULL;
      temp_vcn = NULL;
      //++++ address generator
      start_address_ptrptr_vcn = &this_node;
      //++++ memory access
      mem_write_ptrptr_vcn( start_address_ptrptr_vcn, &temp_vcn, N_address);
    }

}

//==================SORT ===============//
void sort_vectors_by_other(int a[], int b[], int c[], int size_vec){

  JAD_node sorting_vec[size_vec];
  int a_s[size_vec], b_s[size_vec];
  int r; 
  for (r = 0; r < size_vec; r++){
    (sorting_vec[r]).nz = c[r];
    (sorting_vec[r]).i = r;
  }

//----- sort C
 int comp (const void* a1, const void* a2){
   return ( (*(JAD_node*)a1).nz - (*(JAD_node*)a2).nz);
 }
 qsort(sorting_vec, size_vec, sizeof (JAD_node), comp);

 //---- sort a and b based on sorted C
 for (r = 0; r < size_vec; r++){
   a_s[r] = a[ (sorting_vec[r]).i ];
   b_s[r] = b[ (sorting_vec[r]).i ];
 }

 memcpy(a, a_s, size_vec * sizeof(int));
 memcpy(b, b_s, size_vec * sizeof(int));
} 

//==================== READ INPUT ====================//

void processLineSprVec(int a[], int a_i[], int *vecSize, FILE * stream){

  int r, k = 0, fs;
  for (r = 0; r < VEC_MAX; r++){
    int in_data = 0;
    fs = fscanf(stream, "%d", &in_data);
    if (in_data != 0){
      a[k] = in_data;
      a_i[k] = r;
      k++;
    }
  }
  *vecSize = k;
}//sparse

void processLineVec(int a[], int vecSize, FILE * stream){
  
  int r, c, fs;
  for (r = 0; r < vecSize; r++){
    int in_data = 0;
    fs = fscanf(stream, "%d", &in_data);
    a[r] = in_data;
  }
}//dense

void processLine(int ** a, int rowMax, int colMax, FILE * stream){
  
  int r, c, fs;
  int nz = 0;
  int nzRow = 0;
  int max_nzRow = 0;
  for (r = 0; r < rowMax; r++){
    max_nzRow = max(max_nzRow, nzRow);
    nzRow = 0;
    for (c = 0; c < colMax; c++){
      int in_data = 0;
      fs = fscanf(stream, "%d", &in_data);
      a[r][c] = in_data;
      if (in_data != 0){
	nz++;
	nzRow++;
	//if (nz > NZmax){
	  //printf("nz: %d > NZmax\n");
	  //exit(1);
	//}
      }
    }
  }

  printf("nz is %d.\nmax_nz in row=%d\n", nz, max_nzRow);
}//dense

void processLineDense(int a[ROWS][COLS], int rowMax, int colMax, FILE * stream){
  
  int r, c, fs;
  int nz = 0;
  int nzRow = 0;
  int max_nzRow = 0;
  for (r = 0; r < rowMax; r++){
    max_nzRow = max(max_nzRow, nzRow);
    nzRow = 0;
    for (c = 0; c < colMax; c++){
      int in_data = 0;
      fs = fscanf(stream, "%d", &in_data);
      a[r][c] = in_data;
      if (in_data != 0){
	nz++;
	nzRow++;
	//if (nz > NZmax){
	  //printf("nz: %d > NZmax\n");
	  //exit(1);
	//}
      }
    }
  }

  printf("nz is %d.\nmax_nz in row=%d\n", nz, max_nzRow);
}//dense

void processLineCRS(int val[],int col[],int rowPtr[], int rowMax, int colMax, FILE * stream){
  
  int fs,r,c,nz_i=0, nz_row=0, max_row=0;
  for (r = 0; r < rowMax; r++){
    rowPtr[r] = nz_i;
    nz_row=0;
    for (c = 0; c < colMax; c++){
      int in_data = 0;
      fs = fscanf(stream, "%d", &in_data);
      if (in_data != 0){// in_data=0 is missing data, otherwise it is NZ
	if (nz_i >= NZmax){
          printf("ERROR! NZ is %d > NZmax: %d\n", nz_i, NZmax);
          exit(1);
        }
        val[nz_i] = in_data;
        col[nz_i] = c;
        nz_i++;
        nz_row++;
      }// in_data >0
    }//c
    if (nz_row==0){
      printf("row %d is empty\n",r);
      //exit(1);
    }
    if (nz_row> max_row)
      max_row = nz_row;
  }//r
  rowPtr[rowMax] = nz_i; //NZ
  printf("nzs are:%d, max nz:%d\n",nz_i, max_row);

}//CRS

void processLineSLL(int val[],int col[],int row[], int rowMax, int colMax, FILE * stream)
{
  int fs,r,c,nz_i=0;
  for (r = 0; r < rowMax; r++){
    for (c = 0; c < colMax; c ++){
      int in_data = 0;
      fs = fscanf(stream, "%d", &in_data);
      if (in_data != 0){// in_data = 0 is missing data
	//printf("in_data: %f\n", in_data);
	if (nz_i >= NZmax){
	  printf("ERROR! NZ is %d > NZmax: %d\n", nz_i, NZmax);
	  exit(1);
	}
	val[nz_i] = in_data;
	col[nz_i] = c;
	row[nz_i] = r;
	nz_i++;
      }
    }//c
  }//r

  //---- set the last element in row
  row[nz_i] = rowMax;
  printf("nzs are:%d\n",nz_i);
}

void processLineCOO(int val_col_row[NZmax + 1][3], int rowMax, int colMax, FILE * stream){
  int fs, r,c,nz_i=0;

  for (r = 0; r < rowMax; r++){
    for (c = 0; c < colMax; c ++){
      int in_data = 0;
      fs = fscanf(stream, "%d", &in_data);
      if (in_data != 0){// in_data=0 is missing data
	//printf("COO in_data:%f \n", in_data);
	if (nz_i >= NZmax){
	  printf("ERROR! NZ is %d > NZmax: %d\n", nz_i, NZmax);
	  exit(1);
	}
	val_col_row[nz_i][0] = in_data; //val
	val_col_row[nz_i][1] = c; //col
	val_col_row[nz_i][2] = r; //row
	nz_i++;
      }
    }//c
  }//r
  val_col_row[nz_i][2] = rowMax;// to make the end distinguishable
  printf("nzs are:%d\n",nz_i);
}

void processLineELLPACK(int val_row[ ROWS ][ NZrowMax ],int index_row[ ROWS ][ NZrowMax + 1 ], int NZ_row[], int rowMax, int colMax, FILE * stream){
  
  int fs, r,c,nz_i=0, nzR_i=0;
  int max_NZ_row=0;
  for (r = 0; r < rowMax; r++){
    if (nzR_i > max_NZ_row)
      max_NZ_row = nzR_i;
    nzR_i=0;
    for (c = 0; c < colMax; c++){
      int in_data = 0;
      fs=fscanf(stream, "%d", &in_data);
      if (in_data != 0){//--- in_data=0 is missing data
	if (nzR_i >= NZrowMax){
	  printf("ERROR! NZ_row is %d > NZrowMax: %d\n", nzR_i, NZrowMax);
	  exit(1);
	}
	else{
	  val_row[r][nzR_i] = in_data;
	  index_row[r][nzR_i] = c;
	}
	nz_i++;
	nzR_i++;
      }
    }//c
    index_row[r][nzR_i] = colMax;// to distinguish the end
    NZ_row[r]= nzR_i;
  }//r
  //memcpy(NZ_row_protected, NZ_row, ROWS*sizeof(int));
  printf("nzs are:%d\n",nz_i);
  printf("NZ max in row is %d\n",max_NZ_row);
}//ELLPACK

void processLineLIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int NZ_row[], int rowMax, int colMax, FILE * stream)
{
  //--- Here I fill NZ_row just to be used for error injection---//
  int r, fs, nz_i=0, c, nzR_i=0;
  for (r = 0; r < rowMax; r++){
    row_list[r] = NULL;
    nzR_i=0;
    for (c = 0; c < colMax; c ++){
      int in_data = 0;
      fs = fscanf(stream, "%d", &in_data);
      if (in_data != 0){// in_data=0 is missing data
	//printf("LIL in_data: %f\n", in_data);
	add_node(row_list, LIL_nodes, r,c,in_data);
	nz_i++;
	nzR_i++;
      }
    }//c
    NZ_row[r]= nzR_i;
    add_node(row_list, LIL_nodes, r, colMax, 0);
  }//r
  printf("nzs are:%d\n",nz_i);
  printf("max nz in rows is:%d\n",nzR_i);
}//LIL

void processLineJAD( int JADptr[], int val[], int col[], int val_row[ ROWS ][ NZrowMax ], int index_row[ ROWS ][ NZrowMax + 1 ], int NZ_row[], int rowMax, int colMax, int row_order[], FILE * stream){
  //JAD_node sorted_row[USERS];
  JAD_node sorted_row[ROWS];
  int r = 0, c,nz_i=0;
  processLineELLPACK(val_row, index_row, NZ_row,rowMax, colMax, stream);//number of nz in NZ_row

  //printf("rowMax: %d\n", rowMax);

  for (r = 0; r < rowMax; r++){
    (sorted_row[r]).nz = NZ_row[r];
    (sorted_row[r]).i = r;
  }
//----- sort the rows
 int comp (const void* a1, const void* a2){
   return ( (*(JAD_node*)a2).nz - (*(JAD_node*)a1).nz);
 }
 qsort(sorted_row, rowMax, sizeof (JAD_node), comp);
 
 int nz_in_row = 0;
 int nz = 0;
 int column = 0;
 int i;
for (r = 0; r < rowMax; r++){
     row_order[ r ] = sorted_row[r].i;
     //printf("row_order[%d]: %d\n", r, row_order[r]);
 }

 for (column = 0; column < NZrowMax; column++){
   JADptr[column] = nz;
   for (r = 0; r < rowMax; r++){
     i = sorted_row[r].i;
     nz_in_row = sorted_row[r].nz;
     if (nz_in_row <= column)
       break;
     val[nz] = val_row [i][column];
     col[nz] = index_row[i][column];
     nz++;
     if (nz> NZmax)
       printf("nz: %d, NZmax:%d\n",nz, NZmax);
   }
 }//column
 JADptr[NZrowMax] = nz;
}//JAD

//====================== PICK A ROW ========================//
void Pick_a_row_DENSE_protected_0(int a[ ROWS ][ COLS ], int rowNum, int row_vec[]){
  int N_address;
  int *start_address_int;
  int temp[COLS];
  //memcpy(row_vec, &(a[ rowNum ][ 0 ]), COLS * sizeof (int));
  
  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(row_vec[0]);  end_spr_addr[ Nspr ] = &(row_vec[ COLS - 1 ]);
  Nspr++;

  //++++ address generator
  start_address_int = address_generator_int(rowNum, COLS, 0,&(a[0][0]));
  N_address = COLS;
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, COLS, 0, &(row_vec[0]));
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);

  Nspr--;
  return;
}

void Pick_a_row_CRS2(float val[], int col[], int rowPtr[], int rowNum, float valSpr[], int indexSpr[], int * NZsize){
  int i_start, i_end;
  
  i_start = rowPtr[rowNum];
  i_end = rowPtr[rowNum + 1];  
  *NZsize = i_end - i_start;

  memcpy(valSpr, &(val[i_start]), (*NZsize) * sizeof (float));
  memcpy(indexSpr, &(col[i_start]), (*NZsize) * sizeof (int));
}//CRS

void Pick_a_row_CRS_protected_0(int val[], int col[], int rowPtr[], int rowNum, int valSpr[], int indexSpr[], int * NZsize){
  int i_start, i_end;
  int * start_address_int;
  int N_address = 0;
  int index;

  //------- 1. Find the row range -----------//

  //++++ address generator
  start_address_int = address_generator_int(0, 0, rowNum, rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &i_start, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, (rowNum + 1), rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &i_end, start_address_int, N_address);
  
  //++++ functional block
  *NZsize = i_end - i_start;
  
  
  //----- 2. Pick the row ---------------//
  int NZsize_pr = max( 0, min( (*NZsize), VEC_MAX));//---- Protection 0
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start, val);
  N_address = NZsize_pr;
  //++++ memory access

  //  printf("i_start:%d, NZspr:%d\n", i_start, NZsize_pr);
  mem_read_int( valSpr, start_address_int, NZsize_pr);
  //printf("valSpr[0]: %d\n", valSpr[0]);
  err_num += inject_error( valSpr, NULL, NULL, NULL, 1, ERR_PR, INT);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start, col);
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int( indexSpr, start_address_int, NZsize_pr);
  //printf("index0:%d\n", indexSpr[0]);  
  err_num += inject_error( indexSpr, NULL, NULL, NULL, 1, ERR_PR, INT);
  
}//CRS

void Pick_a_row_CRS_protected_1(int val[], int col[], int rowPtr[], int rowNum, int rowMax, int colMax, int valSpr[], int indexSpr[], int * NZsize){
  int i_start, i_end;
  int * start_address_int;
  int N_address = 0;
  int index;

  //------- 1. Find the row range -----------//

  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, 0, rowNum, 0, rowMax, &(rowPtr[0]));
  N_address = 1;
  //++++ memory access
  mem_read_int_p1( &i_start, start_address_int, N_address, 0, NZmax);
  
  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, 0, (rowNum + 1), 0, rowMax, &(rowPtr[0]));
  N_address = 1;
  //++++ memory access
  mem_read_int_p1( &i_end, start_address_int, N_address, 0, NZmax);
  
  //++++ functional block
  *NZsize = i_end - i_start;

  
  //----- 2. Pick the row ---------------//
  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, 0, i_start, 0, NZmax - 1,  &(val[0]));
  N_address = *NZsize;
  //++++ memory access
  int NZsize_pr = max( 0, min( (*NZsize), VEC_MAX));//---- Protection 0
  mem_read_int( valSpr, start_address_int, NZsize_pr);
  
  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, 0, i_start, 0, NZmax - 1, &(col[0]));
  N_address = *NZsize;
  //++++ memory access
  mem_read_int_p1( indexSpr, start_address_int, NZsize_pr, 0, colMax - 1);
}//CRS


/*void Pick_a_row_CRS_protected_1(int val[], int col[], int rowPtr[], int rowNum, int valSpr[], int indexSpr[], int * NZsize){
  int i_start, i_end;
  int * start_address_int;
  int N_address = 0;

  //------- 1. Find the row range -----------//

  //++++ address generator
  start_address_int = &(rowPtr[rowNum]);
  N_address = 1;
  
  //++++ memory access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  if (inboundry( start_address_int ))
    memcpy( &i_start, start_address_int, N_address * sizeof (int));
  err_num += inject_error(&i_start, NULL, NULL, NULL, 1, ERR_PR, INT);

  //++++ address generator
  start_address_int = &(rowPtr[rowNum + 1]);
  N_address = 1;
  
  //++++ memory access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  if (inboundry( start_address_int ))
    memcpy( &i_end, start_address_int, N_address * sizeof (int));
  err_num += inject_error(&i_end, NULL, NULL, NULL, 1, ERR_PR, INT);

  //++++ functional block
  *NZsize = i_end - i_start;
  //err_num += inject_error( NZsize, NULL, NULL, NULL, 1, ERR_PR, INT);


  //----- 2. Pick the row ---------------//

  //++++ address generator
  start_address_int = &(val[i_start]);
  N_address = *NZsize;
  //

  //++++ memory access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  int NZsize_pr = max( 0, min( (*NZsize), VEC_MAX));//---- Protection 0
  printf("NZsizepr: %d\n",NZsize_pr);
  if (inboundry( start_address_int ))
    memcpy( valSpr, start_address_int , NZsize_pr * sizeof (int));
    err_num += inject_error(valSpr, NULL, NULL, NULL, NZsize_pr, ERR_PR, INT);//--- bc of READ
  //err_num += inject_error(valSpr, NULL, NULL, NULL, 1, ERR_PR, INT);//--- bc of READ

  //++++ address generator
  start_address_int = &(col[i_start]);
  //N_address = *NZsize;
  
  //++++ memory access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  if (inboundry( start_address_int ))
    memcpy(indexSpr, start_address_int, NZsize_pr * sizeof (int));
  err_num += inject_error(indexSpr, NULL, NULL, NULL, NZsize_pr, ERR_PR, INT);//---bc of READ
}//CRS
*/

void Pick_a_row_SLL2(int val[], int col[], int row[], int rowNum, int valSpr[], int indexSpr[], int * NZsize){
  int i_start = 0, i_end = 0;
  
  //------ Find i_start
  int row_i_start = row[0];
  while(row_i_start < rowNum){
    if (i_start == NZmax - 1)
      break;
    i_start++;
    row_i_start= row[i_start];
  }

  //---- Find i_end
  i_end = i_start;
  int row_i_end = row[i_end];
  while (row_i_end == rowNum){
      i_end++;
      row_i_end = row[i_end];
  }
  *NZsize = i_end - i_start;

  //---- Copy the row as a sparse vector 
  memcpy(valSpr, &(val[i_start]), (*NZsize) * sizeof (int));
  memcpy(indexSpr, &(col[i_start]), (*NZsize) * sizeof (int));
}

void Pick_a_row_SLL_protected_0(int val[], int col[], int row[], int rowNum, int valSpr[], int indexSpr[], int * NZsize){
  int i_start = 0, i_end = 0, NZsize_pr, N_address, row_i_start, row_i_end, row_i_start_pr, row_i_end_pr;
  int *start_address_int;  
  //------ Find i_start
  /*
  //++++ address generator
  start_address_int = &(row[ 0 ]);
  N_address = 1;

  //++++ mem access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  if (inboundry( start_address_int ))
    memcpy( &row_i_start, start_address_int , N_address * sizeof (int));
  err_num += inject_error(&row_i_start, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ
  */
//++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row);
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i_start, start_address_int, N_address);
  

  while(row_i_start < rowNum){
    if (i_start >= NZmax - 1)//--- protection 0
      break;
    //++++ functional
    i_start++;
   
    //++++ address generator
    start_address_int = address_generator_int( 0, 0, i_start, row );
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i_start, start_address_int, N_address);
  }
  

  //-------- 2. Find i_end --------------//
  //++++ functional
  i_end = i_start;
  /*
  //++++ address generator
  start_address_int = &(row[ i_end ]);
  N_address = 1;
  //++++ mem access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  if (inboundry( start_address_int ))
    memcpy( &row_i_end, start_address_int , N_address * sizeof (int));
  err_num += inject_error(&row_i_end, NULL, NULL, NULL, 1, ERR_PR, INT);//--- bc of READ
  */
  
//++++ address generator
  start_address_int = address_generator_int(0, 0, i_end, row);
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i_end, start_address_int, N_address);
  

  while(row_i_end == rowNum){
    if (i_end >= NZmax + i_start)//---- protection 0
      break;

    //++++ functional
    i_end++;
    /*  
    //++++ address generator
    start_address_int = &(row[ i_end ]);
    N_address = 1;
    //++++ mem access
    err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
    if (inboundry( start_address_int ))
      memcpy( &row_i_end, start_address_int , N_address * sizeof (int));
    err_num += inject_error(&row_i_end, NULL, NULL, NULL, 1, ERR_PR, INT);//--- bc of READ
  
*/
  
//++++ address generator
  start_address_int = address_generator_int(0, 0, i_end, row);
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i_end, start_address_int, N_address);

  }

  //++++ functional
  *NZsize = i_end - i_start;
  NZsize_pr = max( 0, min( *NZsize, VEC_MAX));
  
  //-------- 3. Copy the row as a sparse vector --------//   
  //++++ address generator
  /* start_address_int = &(val[ i_start ]);
  N_address = NZsize_pr;

  //++++ mem access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  if (inboundry( start_address_int ))
    memcpy( valSpr, start_address_int , N_address * sizeof (int));
  err_num += inject_error( valSpr, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ
  */
  
//++++ address generator
  start_address_int = address_generator_int( 0, 0, i_start, val);
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int( valSpr, start_address_int, N_address);


  //++++ address generator
  /*  start_address_int = &(col[ i_start ]);
  N_address = NZsize_pr;

  //++++ mem access
  err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
  if (inboundry( start_address_int ))
    memcpy( indexSpr, start_address_int , N_address * sizeof (int));
  err_num += inject_error(indexSpr, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ
  */
  
//++++ address generator
  start_address_int = address_generator_int( 0, 0, i_start, col);
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int( indexSpr, start_address_int, N_address);
  
}

void Pick_a_row_SLL_protected_1(int val[], int col[], int row[], int rowNum, int rowMax, int colMax, int valSpr[], int indexSpr[], int * NZsize){
  int i_start = 0, i_end = 0, NZsize_pr, N_address, row_i_start, row_i_end, row_i_start_pr, row_i_end_pr;
  int *start_address_int;
  //------ Find i_start
  //++++ address generator
  start_address_int = address_generator_int_p1(0,0,0, VEC_MAX + 1, 0,0,VEC_MAX, row);
  N_address = 1;
  //++++ memory access
  mem_read_int_p1( &row_i_start, start_address_int, N_address,0 ,rowMax);


  while(row_i_start < rowNum){
    if (i_start >= NZmax - 1)//--- protection 0
      break;
    //++++ functional
    i_start++;

    //++++ address generator
    start_address_int = address_generator_int_p1( 0,0,0, VEC_MAX + 1, i_start,0,VEC_MAX, row );
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &row_i_start, start_address_int, N_address, 0, rowMax);
  }
  //-------- 2. Find i_end --------------//
  //++++ functional
  i_end = i_start;

  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, VEC_MAX + 1, i_end,0, VEC_MAX, row);
  N_address = 1;
  //++++ memory access
  mem_read_int_p1( &row_i_end, start_address_int, N_address, 0, rowMax);


  while(row_i_end == rowNum){
    if (i_end >= NZmax + i_start)//---- protection 0
      break;
    //++++ functional
    i_end++;

    //++++ address generator
    start_address_int = address_generator_int_p1(0,0,0, VEC_MAX + 1, i_end,0, VEC_MAX,\
						 row);
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &row_i_end, start_address_int, N_address, 0, rowMax);

  }

  //++++ functional
  *NZsize = i_end - i_start;
  NZsize_pr = max( 0, min( *NZsize, VEC_MAX));

  //-------- 3. Copy the row as a sparse vector --------//
  //++++ address generator
  start_address_int = address_generator_int_p1( 0, 0, 0, VEC_MAX, i_start, 0, VEC_MAX - 1, val);
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int( valSpr, start_address_int, N_address);

  //++++ address generator
  start_address_int = address_generator_int_p1( 0, 0, 0, VEC_MAX, i_start, 0, VEC_MAX \
						-1, col);
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int_p1( indexSpr, start_address_int, N_address, 0, colMax - 1);
}  


void Pick_a_row_COO(int val_col_row[ NZmax + 1 ][ 3 ], int rowNum, int valSpr[], int indexSpr[], int * NZsize){
  int i, i_start = 0, i_end = 0;

  //------ Find i_start
  int row_i_start = val_col_row[ i_start ][ 2 ];
  while(row_i_start < rowNum){
      if (i_start == NZmax - 1)
        break;
      i_start++;
      row_i_start = val_col_row[ i_start ][ 2 ];
  }
  
  //---- Find i_end
  i_end = i_start;
  int row_i_end = val_col_row[ i_end ][ 2 ]; 
  while (row_i_end == rowNum){
    i_end++;
    row_i_end = val_col_row[ i_end ][ 2 ];
  }
   
  *NZsize = i_end - i_start;

  //---- Copy the row as a sparse vector
  for (i = i_start; i < i_end; i++){
    valSpr[ i - i_start ] = val_col_row[ i ][ 0 ];
    indexSpr[ i - i_start ] = (int) val_col_row[ i ][ 1 ];
  }
}

void Pick_a_row_COO_protected_0(int val_col_row[ NZmax + 1 ][ 3 ], int rowNum, int valSpr[], int indexSpr[], int * NZsize){

  int * start_address_int;
  int N_address, row_i_start, i_start = 0, i_end, row_i_end;

  //------ Find i_start
  //++++ address generator
  start_address_int = address_generator_int(0, 3, 2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i_start, start_address_int, N_address);


  while(row_i_start < rowNum){
    if (i_start >= NZmax - 1)//--- protection 0
      break;
    //++++ functional
    i_start++;

    //++++ address generator
    start_address_int = address_generator_int( i_start, 3, 2, &(val_col_row[0][0]) );
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i_start, start_address_int, N_address);

    //row_i_start = val_col_row[i_start][2];
  }
  //  printf("i: %d, i_start:%d\n", rowNum, i_start);


  //-------- 2. Find i_end --------------//
  //++++ functional
  i_end = i_start;
  //++++ address generator
  start_address_int = address_generator_int(i_end, 3, 2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i_end, start_address_int, N_address);


  while(row_i_end == rowNum){
    if (i_end >= NZmax + i_start)//---- protection 0
      break;

    //++++ functional
    i_end++;
    //++++ address generator
    start_address_int = address_generator_int(i_end, 3, 2, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i_end, start_address_int, N_address);
  }

  //++++ functional
  *NZsize = i_end - i_start;
  int NZsize_pr = max( 0, min( *NZsize, VEC_MAX));

  //-------- 3. Copy the row as a sparse vector --------//
  int k;
  for(k = 0; k< NZsize_pr; k++){

    //++++ address generator
    start_address_int = address_generator_int( i_start + k, 3, 0, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &(valSpr[ k ]), start_address_int, N_address);

    //++++ address generator
    start_address_int = address_generator_int( i_start + k, 3, 1, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &(indexSpr[ k ]), start_address_int, N_address);
  }
}

void Pick_a_row_COO_protected_1(int val_col_row[ NZmax + 1 ][ 3 ], int rowNum, int colMax, int rowMax, int valSpr[], int indexSpr[], int * NZsize){

  int * start_address_int;
  int N_address, row_i_start, i_start = 0, i_end, row_i_end;

  //------ Find i_start
  //++++ address generator
  start_address_int = address_generator_int_p1(0,0,NZmax, 3, 2,0,2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_read_int_p1( &row_i_start, start_address_int, N_address, 0, rowMax);


  while(row_i_start < rowNum){
    if (i_start >= NZmax - 1)//--- protection 0
      break;
    //++++ functional
    i_start++;

    //++++ address generator
    start_address_int = address_generator_int_p1( i_start,0, NZmax, 3, 2,0,2, &(val_col_row[0][0]) );
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &row_i_start, start_address_int, N_address, 0, rowMax);

    //row_i_start = val_col_row[i_start][2];
  }
  //  printf("i: %d, i_start:%d\n", rowNum, i_start);


  //-------- 2. Find i_end --------------//
  //++++ functional
  i_end = i_start;
  //++++ address generator
  start_address_int = address_generator_int_p1(i_end,0, NZmax, 3, 2,0,2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_read_int_p1( &row_i_end, start_address_int, N_address, 0, rowMax);


  while(row_i_end == rowNum){
    if (i_end >= NZmax + i_start)//---- protection 0
      break;

    //++++ functional
    i_end++;
    //++++ address generator
    start_address_int = address_generator_int_p1(i_end,0, NZmax, 3, 2,0,2, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &row_i_end, start_address_int, N_address, 0, rowMax);
  }

  //++++ functional
  *NZsize = i_end - i_start;
  int NZsize_pr = max( 0, min( *NZsize, VEC_MAX));

  //-------- 3. Copy the row as a sparse vector --------//
  int k;
  for(k = 0; k< NZsize_pr; k++){

    //++++ address generator
    start_address_int = address_generator_int_p1( i_start + k,0,NZmax, 3, 0,0,2, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &(valSpr[ k ]), start_address_int, N_address);

    //++++ address generator
    start_address_int = address_generator_int_p1( i_start + k,0,NZmax, 3, 1,0,2, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &(indexSpr[ k ]), start_address_int, N_address, 0, colMax-1);
  }
}

void Pick_a_row_ELLPACK2(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int colMax, int rowNum, int valSpr[], int indexSpr[], int * NZsize){
  int i_end = 0;

  //---- Find i_end
  int last_element = 0;
  while (i_end < NZrowMax){// bc of error
    last_element = index_row[ rowNum ][ i_end ];
    if (last_element >= colMax)
      break;
    i_end++;
  }
  *NZsize = i_end;

  //---- Copy the row as a sparse vector
  memcpy(valSpr, &(val_row[ rowNum ][ 0 ]), (*NZsize) * sizeof (float));
  memcpy(indexSpr, &(index_row[ rowNum ][ 0 ]), (*NZsize) * sizeof (int));
}

void Pick_a_row_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int colMax, int rowNum, int valSpr[], int indexSpr[], int * NZsize){
  int i_end = 0, NZsize_pr, N_address, last_element = 0;
  int * start_address_int;

  //---------- 1. Find i_end ---------------//
  //++++ functional
  while (i_end < NZrowMax){// bc of error
      
    //++++ address generator
    start_address_int = address_generator_int( rowNum, NZrowMax + 1, i_end , &(index_row[ 0 ][ 0 ]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &last_element, start_address_int, N_address);
    

    //++++ functional
    if (last_element >= colMax)
      break;
    i_end++;
  }
  *NZsize = i_end;

  //------------- 2. Copy the row as a sparse vector -----------------//
  NZsize_pr = max( 0, min( *NZsize , VEC_MAX ));//---- prorection 0
  //++++ address generator
  start_address_int = address_generator_int( rowNum, NZrowMax, 0, &(val_row[ 0 ][ 0 ]));
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int( valSpr, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int( rowNum, NZrowMax + 1, 0 , &(index_row[ 0 ][ 0 ]));
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int( indexSpr, start_address_int, N_address);
  return;
}


void Pick_a_row_ELLPACK_protected_1(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int colMax, int rowNum,int rowMax,  int valSpr[], int indexSpr[], int * NZsize){
  int i_end = 0, NZsize_pr, N_address, last_element = 0;
  int * start_address_int;

  //---------- 1. Find i_end ---------------//
  //++++ functional
  while (i_end < NZrowMax){// bc of error
      
    //++++ address generator
    start_address_int = address_generator_int_p1( rowNum,0, rowMax-1, NZrowMax + 1, i_end , 0, NZrowMax,&(index_row[ 0 ][ 0 ]));
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &last_element, start_address_int, N_address, 0, colMax);
    

    //++++ functional
    if (last_element >= colMax)
      break;
    i_end++;
  }
  *NZsize = i_end;

  //------------- 2. Copy the row as a sparse vector -----------------//
  NZsize_pr = max( 0, min( *NZsize , VEC_MAX ));//---- prorection 0
  //++++ address generator
  start_address_int = address_generator_int_p1( rowNum,0,rowMax -1, NZrowMax, 0,0, NZrowMax -1, &(val_row[ 0 ][ 0 ]));
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int( valSpr, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int_p1( rowNum,0,rowMax-1, NZrowMax + 1, 0 ,0,NZrowMax, &(index_row[ 0 ][ 0 ]));
  N_address = NZsize_pr;
  //++++ memory access
  mem_read_int_p1( indexSpr, start_address_int, N_address, 0, colMax);
  return;
}

void Pick_a_row_LIL(val_col_node *row_list [], int rowNum, float valSpr[], int indexSpr[], int * NZsize){

  int i_end = 0;
  val_col_node * this_node;

  //----- Find i_end and Copy the row to a sparse vector
  this_node = row_list[ rowNum ];
  while(this_node != NULL){
    valSpr[ i_end ] = this_node->val;
    indexSpr[ i_end ] = this_node->col;
    this_node = this_node->next;
    i_end++;
  }
  *NZsize = i_end - 1;
}

void Pick_a_row_LIL_protected_0(val_col_node *row_list [], int rowNum, int valSpr[], int indexSpr[], int * NZsize){

  int i_end = 0;
  val_col_node * this_node, *this_node_next;
  int N_address, NZsize_pr, this_node_val, this_node_col;
  int * start_address_int;
  val_col_node ** start_address_ptrptr_vcn;
  
  //------------- 1. Find i_end ----------------------// 
  start_address_ptrptr_vcn = address_generator_ptr_vcn( rowNum , row_list);
  N_address = 1;
    //++++ memory access
  mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);
  

  // -------------- 2. Copy the row to a sparse vector -------------------//  
   while(this_node != NULL){
  
     if (i_end >= VEC_MAX )
       break;
    start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, val) / 4 , (int *)this_node);
    N_address = 1;
    //++++ memory access    
    mem_read_int( &(valSpr[i_end]), start_address_int, N_address);

    start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, col) / 4 , (int *)this_node);
    N_address = 1;
    //++++ memory access
    mem_read_int( &(indexSpr[ i_end ]), start_address_int, N_address);
    
    
    start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4 , (int *)this_node));
    N_address = 1;
    //++++ memory access
    mem_read_ptrptr_vcn( &this_node_next, start_address_ptrptr_vcn, N_address);
    
    //++++ functional
    this_node = this_node_next;
    i_end++;
  }
  *NZsize = i_end - 1;    
}

void Pick_a_row_JAD(int JADptr[], int val[], int col[], int row_order[], int rowNum, int valSpr[], int indexSpr[], int * NZsize, int* p_i){

  int k, k_next, jc;
  *NZsize = 0;
  *p_i = row_order[rowNum];

  for ( jc = 0; jc < NZrowMax ; jc++){
    //--- find the item in row=rowNum in jad-column jc
    k = JADptr[jc] + rowNum;
    k_next = JADptr[jc + 1];
    if (k >= k_next)//-----  i_next is the next column
      break;

    indexSpr[jc] = col[k];
    valSpr[jc] = val[k];
    *NZsize = *NZsize + 1;
  }
}


void Pick_a_row_JAD_protected_0(int JADptr[], int val[], int col[], int row_order[], int rowNum, int valSpr[], int indexSpr[], int * NZsize, int* p_i){

  int k, k_next, jc;
  *NZsize = 0;
  int * start_address_int;
  int N_address = 1;

  //---- find perimary row number
  //*p_i = row_order[rowNum];
  start_address_int = address_generator_int( 0, ROWS, rowNum, row_order);
  //++++ memory access
  mem_read_int( p_i, start_address_int, N_address);
  //  p_i = max(min(p_i, ROWS - 1), 0);//----- protection 0
  
  for ( jc = 0; jc < NZrowMax ; jc++){
    //--- find the item in row=rowNum in jad-column jc
    //k = JADptr[jc] + rowNum;
    //++++ address generator
    start_address_int = address_generator_int( 0, COLS + 1, jc, JADptr);
    //++++ memory access
    mem_read_int( &k, start_address_int, N_address);
    k += rowNum;

    //k_next = JADptr[jc + 1];
    //++++ address generator
    start_address_int = address_generator_int( 0, COLS + 1, jc + 1, JADptr);
    //++++ memory access
    mem_read_int( &k_next, start_address_int, N_address);
    
    if (k >= k_next)//-----  i_next is the next column
      break;

    //indexSpr[jc] = col[k];
    //++++ address generator
    start_address_int = address_generator_int( 0, VEC_MAX, k, col);
    //++++ memory access
    mem_read_int( &indexSpr[ jc ], start_address_int, N_address);
    
    //valSpr[jc] = val[k];
    //++++ address generator
    start_address_int = address_generator_int( 0, VEC_MAX, k, val);
    //++++ memory access
    mem_read_int( &valSpr[ jc ], start_address_int, N_address);
    
    //++++ functional
    *NZsize = *NZsize + 1;
  }
}

//====================== PICK A COLUMN ========================//
void Pick_a_col_DENSE_protected_0(int a[ ROWS ][ COLS ], int rowMax, int colNum, int col_vec[]){
  int i, N_address = 1, temp;
  int * start_address_int;
  
  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(col_vec[0]);  end_spr_addr[ Nspr ] = &(col_vec[ ROWS - 1 ]);
  Nspr++;
  
  for ( i = 0; i < rowMax; i++){
    //    col_vec[ i ] = a[ i ][ colNum ];  
    //++++ address generator
    start_address_int = address_generator_int(i, COLS, colNum, &(a[0][0]));
    //++++ memory access
    mem_read_int( &temp, start_address_int, N_address);
    //++++ address generator
    start_address_int = address_generator_int(0, COLS, i, col_vec);
    //++++ memory access
    mem_write_int( start_address_int, &temp, N_address);
    }

  Nspr--;
  return;
}



void Pick_a_col_CRS2(int val[], int col[], int rowPtr[], int rowMax, int colNum, int valSpr[], int indexSpr[], int *NZsize){
  int i, i_start, i_next, j = 0, k;
  
  for ( i = 0; i < rowMax ; i++){
    
    //------- 1. Find the index range for row i --------------//
    i_start = rowPtr[ i ];
    i_next = rowPtr[ i + 1 ];    
    
    //------ 2. Find the items for col = colNum in row i ----------//
    for (k = i_start; k < i_next; k++){
      if (col[ k ] == colNum){
	valSpr[ j ] = val[ k ];
	indexSpr[ j ] = i;
	j++;
	break;//--- go to the next row
      }
      else if(col[ k ] > colNum)
	break;  
    }
   }
  * NZsize = j;
  return;
}
  
void Pick_a_col_CRS_protected_0(int val[], int col[], int rowPtr[], int rowMax, int colNum, int valSpr[], int indexSpr[], int *NZsize){
  int i, i_start, i_next, i_start_pr, i_next_pr, j = 0, k, item_j, N_address;
  int * start_address_int;
  /*printf("rowMax:%d\n", rowMax);
  printf("rowPtr[0]:%d, rowPtr[1]:%d, rowPtr[2]:%d \n",rowPtr[0], rowPtr[1], rowPtr[2]);
  printf("val[0]:%d, val[1]:%d, val[2]:%d \n",val[0], val[1], val[2]);
  printf("col[0]:%d, col[1]:%d, col[2]:%d \n",col[0], col[1], col[2]);
  */
  for ( i = 0; i < rowMax ; i++){
    
    //------- 1. Find the index range for row i --------------//
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &i_start, start_address_int, N_address);

    //++++ address generator
    start_address_int = address_generator_int( 0, 0, i + 1, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &i_next, start_address_int, N_address);


    //------ 2. Find the items for col = colNum in row i ----------//
    /*i_start_pr = max( 0, min( i_start, VEC_MAX));//----Protection 0
    i_next_pr = max( 0, min( i_next, VEC_MAX));//--- Protection 0
    */
    for (k = i_start; k < i_next; k++){
      if ( j >= VEC_MAX)//---- protection 0
	break;

      //++++ address generator
      start_address_int = address_generator_int( 0, 0, k , &(col[0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &item_j, start_address_int, N_address);
  
      //++++ functional block
      if (item_j == colNum){
	//++++ address generator
	start_address_int = address_generator_int( 0, 0, k, &( val[0]));
	N_address = 1;
	//++++ memory access
	mem_read_int( &(valSpr[ j ]), start_address_int, N_address);
  
	//++++ functional block
	indexSpr[ j ] = i;
	j++;
	break;//--- go to the next row
      }
      else if( item_j > colNum )
	break;  
    }
   }

  //++++ functional block
  * NZsize = j;  
  return;
}

void Pick_a_col_CRS_protected_1(int val[], int col[], int rowPtr[], int rowMax, int colNum, int colMax, int valSpr[], int indexSpr[], int *NZsize){
  int i, i_start, i_next, i_start_pr, i_next_pr, j = 0, k, item_j, N_address;
  int * start_address_int;
  
  for ( i = 0; i < rowMax ; i++){
    
    //------- 1. Find the index range for row i --------------//
    //++++ address generator
    start_address_int = address_generator_int_p1(0, 0, 0, 0, i, 0, rowMax, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &i_start, start_address_int, N_address, 0, NZmax);

    //++++ address generator
    start_address_int = address_generator_int_p1( 0, 0, 0, 0, i + 1, 0, rowMax, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int_p1( &i_next, start_address_int, N_address, 0, NZmax);


    //------ 2. Find the items for col = colNum in row i ----------//
    /*i_start_pr = max( 0, min( i_start, VEC_MAX));//----Protection 0
    i_next_pr = max( 0, min( i_next, VEC_MAX));//--- Protection 0
    */

    for (k = i_start; k < i_next; k++){
      if (j >= VEC_MAX)//--- protection 0
	break;

      //++++ address generator
      start_address_int = address_generator_int_p1( 0, 0, 0, 0, k, 0, NZmax - 1, &(col[0]));
      N_address = 1;
      //++++ memory access
      mem_read_int_p1( &item_j, start_address_int, N_address, 0, colMax - 1);
  
      //++++ functional block
      if (item_j == colNum){
	//++++ address generator
	start_address_int = address_generator_int_p1( 0, 0, 0, 0, k, 0, NZmax - 1, &( val[0]));
	N_address = 1;
	//++++ memory access
	mem_read_int( &(valSpr[ j ]), start_address_int, N_address);
  
	//++++ functional block
	indexSpr[ j ] = i;
	j++;
	break;//--- go to the next row
      }
      else if( item_j > colNum )
	break;  
    }
   }

  //++++ functional block
  * NZsize = j;  
  return;
}
/*
  
void Pick_a_col_CRS_protected_1(int val[], int col[], int rowPtr[], int rowMax, int colNum, int valSpr[], int indexSpr[], int *NZsize){
  int i, i_start, i_next, i_start_pr, i_next_pr, j = 0, k, item_j, N_address;
  int * start_address_int;
  
  for ( i = 0; i < rowMax ; i++){
    
    //------- 1. Find the index range for row i --------------//
    //++++ address generator
    start_address_int = &(rowPtr[ i ]);
    N_address = 1;
    
    //++++ memory access
    err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
    if (inboundry( start_address_int ))
      memcpy( &i_start, start_address_int, N_address * sizeof (int));
    err_num += inject_error(&i_start, NULL, NULL, NULL, 1, ERR_PR, INT);

    //++++ address generator
    start_address_int = &(rowPtr[ i + 1 ]);
    N_address = 1;
    
    //++++ memory access
    err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
    if (inboundry( start_address_int ))
      memcpy( &i_next, start_address_int, N_address * sizeof (int));
    err_num += inject_error(&i_next, NULL, NULL, NULL, 1, ERR_PR, INT);
    
    //------ 2. Find the items for col = colNum in row i ----------//
    i_start_pr = max( 0, min( i_start, VEC_MAX));//----Protection 0
    i_next_pr = max( 0, min( i_next, VEC_MAX));//--- Protection 0

    for (k = i_start_pr; k < i_next_pr; k++){

      //++++ address generator
      start_address_int = &( col[ k ]);
      N_address = 1;
     
      //++++ memory access
      err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
      if (inboundry( start_address_int ))
	memcpy( &item_j, start_address_int, N_address * sizeof (int));
      err_num += inject_error(&item_j, NULL, NULL, NULL, 1, ERR_PR, INT);

      //++++ functional block
      if (item_j == colNum){

	//++++ address generator
	start_address_int = &( val[ k ]);
	N_address = 1;
	
	//++++ memory access
	err_num += inject_error(NULL, start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
	if (inboundry( start_address_int )){
	  memcpy( &(valSpr[ j ]), start_address_int, N_address * sizeof (int));
	  err_num += inject_error(&(valSpr[ j ]), NULL, NULL, NULL, 1, ERR_PR, INT);
	}

	//++++ functional block
	indexSpr[ j ] = i;
	j++;
	break;//--- go to the next row
      }
      else if( item_j > colNum )
	break;  
    }
   }

  //++++ functional block
  * NZsize = j;
  //  err_num += inject_error(NZsize, NULL, NULL, NULL, 1, ERR_PR, INT);

  return;
}
*/

void Pick_a_col_SLL2(float val[], int col[], int row[], int rowMax, int colNum, float valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  for (k = 0; k < NZmax; k++){
    if (row[ k ] >= rowMax)//---- It shows the end of the matrix
      break;
    
    //---- Find the items in col = colNum
    if (col[ k ] == colNum){
      valSpr[ i ] = val[ k ];
      indexSpr[ i ] = row[ k ];
      i++;
    }
  }
  *NZsize = i;
  return;
}


void Pick_a_col_SLL_protected_0(int val[], int col[], int row[], int rowMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  int col_k, row_k, NZsize_pr, N_address;
  int *start_address_int;

  //----------- 1. Find the matrix size -------------//
  for (k = 0; k < NZmax; k++){
    //++++ address generator
    start_address_int = address_generator_int(0, 0, k , row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_k, start_address_int, N_address);
    
    //++++ functional
    if (row_k >= rowMax)
      break;
    
    //------------- 2. Find the items in col = colNum -----------//
    //++++ address generator
    start_address_int = address_generator_int( 0, 0, k , col);
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);
    
    //++++ functional
    if (col_k == colNum){
      //++++ address generator
      start_address_int = address_generator_int( 0, 0, k , val);
      N_address = 1;
      //++++ memory access
      mem_read_int( &( valSpr[ i ]), start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int( 0, 0, k , row);
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_k, start_address_int, N_address);

      //++++ funsctional
      indexSpr[ i ] = row_k;
      i++;
    }
  }

  *NZsize = i;
  return;
}

void Pick_a_col_SLL_protected_1(int val[], int col[], int row[], int rowMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  int col_k, row_k, val_k, NZsize_pr, N_address;
  int *start_address_int;

  //----------- 1. Find the matrix size -------------//
  for (k = 0; k < NZmax; k++){
    //++++ address generator
    start_address_int = &(row[ k ]);
    N_address = 1;
    
    //++++ mem access
    err_num += inject_error(NULL, &start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
    if (inboundry( start_address_int ))
      memcpy( &row_k, start_address_int , N_address * sizeof (int));
    err_num += inject_error(&row_k, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ
    
    //++++ functional
    if (row_k >= rowMax)
      break;
    
    //------------- 2. Find the items in col = colNum -----------//
    //++++ address generator
    start_address_int = &(col[ k ]);
    N_address = 1;
    
    //++++ mem access
    err_num += inject_error(NULL, &start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
    if (inboundry( start_address_int ))
      memcpy( &col_k, start_address_int , N_address * sizeof (int));
    err_num += inject_error(&col_k, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ
    
    //++++ functional
    if (col_k == colNum){

      //++++ address generator
      start_address_int = &(val[ k ]);
      N_address = 1;
    
      //++++ mem access
      err_num += inject_error(NULL, &start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
      if (inboundry( start_address_int ))
	memcpy( &val_k, start_address_int , N_address * sizeof (int));
      err_num += inject_error(&val_k, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ

      //++++ funsctional
      valSpr[ i ] = val_k;

      //++++ address generator
      start_address_int = &(row[ k ]);
      N_address = 1;
    
      //++++ mem access
      err_num += inject_error(NULL, &start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
      if (inboundry( start_address_int ))
	memcpy( &row_k, start_address_int , N_address * sizeof (int));
      err_num += inject_error(&row_k, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ

      //++++ funsctional
      indexSpr[ i ] = row_k;
      i++;
    }
  }

  *NZsize = i;
  return;
}

void Pick_a_col_COO_protected_0(int val_col_row[ NZmax + 1 ][ 3 ], int rowMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  int col_k, row_k, NZsize_pr, N_address;
  int *start_address_int;

  //----------- 1. Find the matrix size -------------//
  for (k = 0; k < NZmax; k++){
    //++++ address generator
    start_address_int = address_generator_int(k ,3,2 , &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_k, start_address_int, N_address);
    
    //++++ functional
    if (row_k >= rowMax)
      break;
    
    //------------- 2. Find the items in col = colNum -----------//
    //++++ address generator
    start_address_int = address_generator_int( k ,3, 1 , &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);
    
    //++++ functional
    if (col_k == colNum){
      //++++ address generator
      start_address_int = address_generator_int( k ,3, 0 , &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &( valSpr[ i ]), start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int( k, 3, 2 ,&(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_k, start_address_int, N_address);

      //++++ funsctional
      indexSpr[ i ] = row_k;
      i++;
    }
  }

  *NZsize = i;
  return;
}


void Pick_a_col_COO(float val_col_row[ NZmax + 1 ][ 3 ], int rowMax, int colNum, float valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  for (k = 0; k < NZmax; k++){
    if (val_col_row[ k ][ 2 ] == rowMax)
      break;
    //---- Find the items in col = colNum
    if (val_col_row[ k ][ 1 ] == colNum){
      valSpr[ i ] = val_col_row[ k ][ 0 ];
      indexSpr[ i ] = (int)val_col_row[ k ][ 2 ];
      i++;
    }
  }
  *NZsize = i;
  return;
}

void Pick_a_col_ELLPACK2(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){
  int i, j, k = 0;

  for ( i = 0; i < rowMax ; i++){
    for (j = 0; j < (colNum + 1); j++){
      
      if (index_row[ i ][ j ] >= colMax)//---- End of the row
	break;
      
      //--- find the items for col = colNum in row i
      if (index_row[ i ][ j ] == colNum){
	valSpr[ k ] = val_row[ i ][ j ];
	indexSpr[ k ] = i;
	k++;
	break;//--- End of this row
      }
    }
  }
  * NZsize = k;
}

void Pick_a_col_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){
  int i, j, k = 0, index_row_i_j, N_address;
  int * start_address_int;

  for ( i = 0; i < rowMax ; i++){
    for (j = 0; j < (colNum + 1); j++){

      //------------1. stop if it is end of the row ---------------//
      //++++ address generator
      start_address_int = address_generator_int( i, NZrowMax + 1, j, &(index_row[ 0 ][ 0 ]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &index_row_i_j, start_address_int, N_address);
  
      //++++ functional
      if (index_row_i_j >= colMax)//---- End of the row
	break;
      
      //----------- 2. find the items for col = colNum in row i -----------------//
      if (index_row_i_j == colNum){

	//++++ address generator
	start_address_int = address_generator_int( i, NZrowMax, j , &(val_row[ 0 ][ 0 ]));
	N_address = 1;
	//++++ memory access
	mem_read_int( &(valSpr[ k ]), start_address_int, N_address);

	//++++ functonal
	indexSpr[ k ] = i;
	k++;
	break;//--- End of this row
      }
    }
  }
  * NZsize = k;
}

void Pick_a_col_ELLPACK_protected_1(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){
  int i, j, k = 0, index_row_i_j, N_address;
  int * start_address_int;

  for ( i = 0; i < rowMax ; i++){
    for (j = 0; j < (colNum + 1); j++){

      //------------1. stop if it is end of the row ---------------//
      //++++ address generator
      start_address_int = &(index_row[ i ][ j ]);
      N_address = 1;    
      //++++ mem access
      err_num += inject_error(NULL, &start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
      if (inboundry( start_address_int ))
	memcpy( &index_row_i_j, start_address_int , N_address * sizeof (int));
      err_num += inject_error(&index_row_i_j, NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ
      //++++ functional
      if (index_row_i_j >= colMax)//---- End of the row
	break;
      
      //----------- 2. find the items for col = colNum in row i -----------------//
      if (index_row_i_j == colNum){

      //++++ address generator
      start_address_int = &(val_row[ i ][ j ]);
      N_address = 1;    
      //++++ mem access
      err_num += inject_error(NULL, &start_address_int, NULL, NULL, 1, ERR_PR, INT_POINTER);
      if (inboundry( start_address_int ))
	memcpy( &(valSpr[ k ]), start_address_int , N_address * sizeof (int));
      err_num += inject_error(&(valSpr[ k ]), NULL, NULL, NULL, N_address, ERR_PR, INT);//--- bc of READ

      //++++ functonal
      indexSpr[ k ] = i;
      k++;
      break;//--- End of this row
      }
    }
  }
  * NZsize = k;
}

void Pick_a_col_LIL_protected_0(val_col_node *row_list [], int rowMax, int colMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){

  int i, k = 0, this_node_col, N_address;
  val_col_node * this_node;
  val_col_node ** start_address_ptrptr_vcn;
  int * start_address_int;

  for ( i = 0; i < rowMax ; i++){
    //++++ address generator
    start_address_ptrptr_vcn = address_generator_ptr_vcn( i , row_list);
    N_address = 1;
    //++++ memory access
    mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);

    int l =0;
    while (this_node != NULL){
      l++;
      if ( l >= VEC_MAX )//---- protection 0 
	break;
     
      //--- 4 = sizeof(int) / sizeof(char)
      start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, col) / 4 , (int *)this_node);
      N_address = 1;
      //++++ memory access
      mem_read_int( &this_node_col, start_address_int, N_address);
      //printf("this_node_col:%d\n", this_node_col);

      //++++ functional
      if (this_node_col >= colMax)//----- This is end of the row
	break;

      //--- find the items for col = colNum in row i
      if (this_node_col == colNum){
	start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, val) / 4 , (int *)this_node);
	N_address = 1;
	//++++ memory access
	mem_read_int( &(valSpr[ k ]), start_address_int, N_address);
	//    printf("this_node_val:%d\n", this_node_val);


	//++++ functional
	indexSpr[ k ] = i;
	k++;
	break;//--- go to the next row
      }

	start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4, (int *)this_node));
	//    printf( "address-vcn-2: %d\n", start_address_ptrptr_vcn);
	N_address = 1;
	//++++ memory access
	mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);

    }
  }
  //++++ functional
  * NZsize = k;
}

void Pick_a_col_LIL(val_col_node *row_list [], int rowMax, int colMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){

  int i, k = 0;
  val_col_node * this_node;

  for ( i = 0; i < rowMax ; i++){
    this_node = row_list[ i ];
    while (this_node != NULL){
      if (this_node->col >= colMax)//----- This is end of the row
	break;

    //--- find the items for col = colNum in row i
      if (this_node->col == colNum){
	valSpr[ k ] = this_node->val;
	indexSpr[ k ] = i;
	k++;
	break;//--- go to the next row
      }
      this_node = this_node->next;
    }
  }
  * NZsize = k;
}

void Pick_a_col_JAD_protected_0(int JADptr[], int val[], int col[], int row_order[], int rowMax, int colNum, int colMax, int valSpr[], int indexSpr[], int * NZsize){

  int p_i, i, j, jc, k, l=0;
  int jad_col_start, jad_col_end;
  int last_row_so_far = 0;
  int col_k, lp;
  int *start_address_int;
  int N_address = 1;

  for (jc = 0; jc < NZrowMax; jc++){
    //jad_col_start = JADptr[ jc ];
    //++++ address generator
    start_address_int = address_generator_int(0, NZrowMax + 1, jc, JADptr);
    //++++ memory access
    mem_read_int( &jad_col_start, start_address_int, N_address);

    //jad_col_end = JADptr[ jc + 1 ];
    //++++ address generator
    start_address_int = address_generator_int(0, NZrowMax + 1, jc + 1, JADptr);
    //++++ memory access
    mem_read_int( &jad_col_end, start_address_int, N_address);
    
    last_row_so_far = 0;
    lp = 0;
    for (k = jad_col_start + last_row_so_far; k < jad_col_end; k++){
      lp++;
      if (lp >= ROWS)//-----protection 0
	break;
      //++++ address generator
      start_address_int = address_generator_int(0, NZmax + 1, k, col);
      //++++ memory access
      mem_read_int( &col_k, start_address_int, N_address);
     
      if (col_k  == colNum){
	//valSpr[ l ] = val[ k ];
	//++++ address generator
	start_address_int = address_generator_int(0, NZmax + 1, k, val);
	//++++ memory access
	mem_read_int( &valSpr[ l ], start_address_int, N_address);

	//++++ functional block
	i = k - jad_col_start;

	//p_i = row_order[ i ];
	//++++ address generator
	start_address_int = address_generator_int(0, ROWS, i, row_order);
	//++++ memory access
	mem_read_int( &p_i, start_address_int, N_address);

	//++++ fuctional block
	indexSpr[ l ] = p_i;
	l++;
	last_row_so_far = i;
      }
      else if (last_row_so_far >= rowMax)
	break;
    }

  }
  //------ sort the vector
  sort_vectors_by_other(valSpr, indexSpr, indexSpr, l);

  *NZsize = l;
}

//====================== PICK A DIAGONAL ========================//
void Pick_diag_DENSE_protected_0(int a[ ROWS ][ COLS ], int rowMax, int diag_vec[]){
  int i, N_address = 1, temp;
  int * start_address_int;
  
  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(diag_vec[0]);  end_spr_addr[ Nspr ] = &(diag_vec[ ROWS - 1 ]);
  Nspr++;
  
  for ( i = 0; i < rowMax; i++){  
    //++++ address generator
    start_address_int = address_generator_int(i, COLS, i, &(a[0][0]));
    //++++ memory access
    mem_read_int( &temp, start_address_int, N_address);
    //++++ address generator
    start_address_int = address_generator_int(0, COLS, i, diag_vec);
    //++++ memory access
    mem_write_int( start_address_int, &temp, N_address);
    }

  Nspr--;
  return;
}

void Pick_diag_CRS(float val[], int col[], int rowPtr[], int rowMax, int colNum, float valSpr[], int indexSpr[], int *NZsize){
  int i, i_start, i_next, j = 0, k;
  
  for ( i = 0; i < rowMax ; i++){
    //--- find the index range for row i
    i_start = rowPtr[i];
    i_next = rowPtr[i + 1];
    
    //--- find the items for col = i in row i
    for (k = i_start; k < i_next; k++){
      if (col[ k ] == i){
	valSpr[ j ] = val[ k ];
	indexSpr[ j ] = i;
	j++;
	break;//--- go to the next row
      }
      else if(col[ k ] > i)
	break;  
    }
   }
  * NZsize = j;
  return;
}

void Pick_diag_CRS_protected_0(int val[], int col[], int rowPtr[], int rowMax, int valSpr[], int indexSpr[], int *NZsize){
  int i, i_start, i_next, i_start_pr, i_next_pr, j = 0, k, item_j, N_address;
  int * start_address_int;

  for ( i = 0; i < rowMax ; i++){

    //------- 1. Find the index range for row i --------------//
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &i_start, start_address_int, N_address);

    //++++ address generator
    start_address_int = address_generator_int( 0, 0, i + 1, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &i_next, start_address_int, N_address);


    //------ 2. Find the items for col = colNum in row i ----------//
    /*i_start_pr = max( 0, min( i_start, VEC_MAX));//----Protection 0
    i_next_pr = max( 0, min( i_next, VEC_MAX));//--- Protection 0
    */
    for (k = i_start; k < i_next; k++){
      if ( j >= VEC_MAX)//---- protection 0
        break;

      //++++ address generator
      start_address_int = address_generator_int( 0, 0, k , &(col[0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &item_j, start_address_int, N_address);

      //++++ functional block
      if (item_j == i){////ch
        //++++ address generator
        start_address_int = address_generator_int( 0, 0, k, &( val[0]));
        N_address = 1;
        //++++ memory access
        mem_read_int( &(valSpr[ j ]), start_address_int, N_address);

        //++++ functional block
        indexSpr[ j ] = i;
        j++;
        break;//--- go to the next row
      }
      else if( item_j > i )///ch
        break;
    }
  }

  //++++ functional block
  * NZsize = j;
  return;
}

void Pick_diag_SLL(int val[], int col[], int row[], int rowMax, int colNum, int valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  for (k = 0; k < NZmax; k++){
    if (row[ k ] >= rowMax)//---- It shows the end of the matrix
      break;
    
    //---- Find the items in col = row
    if (col[ k ] == row[ k ]){
      valSpr[ i ] = val[ k ];
      indexSpr[ i ] = row[ k ];
      i++;
    }
  }
  *NZsize = i;
  return;
}

void Pick_diag_SLL_protected_0(int val[], int col[], int row[], int rowMax, int valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  int col_k, row_k, NZsize_pr, N_address;
  int *start_address_int;

  //----------- 1. Find the matrix size -------------//
  for (k = 0; k < NZmax; k++){
    //++++ address generator
    start_address_int = address_generator_int(0, 0, k , row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_k, start_address_int, N_address);

    //++++ functional
    if (row_k >= rowMax)
      break;

    //------------- 2. Find the items in col[k] = row[k] -----------//
    //++++ address generator
    start_address_int = address_generator_int( 0, 0, k , col);
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);

    //++++ functional
    if (col_k == row_k){///ch
      //++++ address generator
      start_address_int = address_generator_int( 0, 0, k , val);
      N_address = 1;
      //++++ memory access
      mem_read_int( &( valSpr[ i ]), start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int( 0, 0, k , row);
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_k, start_address_int, N_address);

      //++++ funsctional
      indexSpr[ i ] = row_k;
      i++;
    }
  }

  *NZsize = i;
  return;
}


void Pick_diag_COO(int val_col_row[ NZmax + 1 ][ 3 ], int rowMax, int valSpr[], int indexSpr[], int * NZsize){
  int k, i = 0;
  for (k = 0; k < NZmax; k++){
    if (val_col_row[ k ][ 2 ] == rowMax)
      break;
    //---- Find the items in col = colNum
    if (val_col_row[ k ][ 1 ] == val_col_row[ k ][ 2 ]){
      valSpr[ i ] = val_col_row[ k ][ 0 ];
      indexSpr[ i ] = val_col_row[ k ][ 2 ];
      i++;
    }
  }
  *NZsize = i;
  return;
}

void Pick_diag_COO_protected_0(int val_col_row[ NZmax + 1 ][ 3 ], int rowMax, int valSpr[], int indexSpr[], int* NZsize){
  int k, i = 0;
  int col_k, row_k, NZsize_pr, N_address;
  int *start_address_int;

  //----------- 1. Find the matrix size -------------//
  for (k = 0; k < NZmax; k++){
    //++++ address generator
    start_address_int = address_generator_int(k ,3,2 , &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_k, start_address_int, N_address);

    //++++ functional
    if (row_k >= rowMax)
      break;

    //------------- 2. Find the items in col = row -----------//
    //++++ address generator
    start_address_int = address_generator_int( k ,3, 1 , &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);

    //++++ functional
    if (col_k == row_k){///ch
      //++++ address generator
      start_address_int = address_generator_int( k ,3, 0 , &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &( valSpr[ i ]), start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int( k, 3, 2 ,&(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_k, start_address_int, N_address);

      //++++ funsctional
      indexSpr[ i ] = row_k;
      i++;
    }
  }

  *NZsize = i;
  return;
}

void Pick_diag_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int valSpr[], int indexSpr[], int * NZsize){
  int i, j, k = 0;

  for ( i = 0; i < rowMax ; i++){
    for (j = 0; j < (i + 1); j++){
      
      if (index_row[ i ][ j ] >= colMax)//---- End of the row
	break;
      
      //--- find the items for col = i in row i
      if (index_row[ i ][ j ] == i){
	valSpr[ k ] = val_row[ i ][ j ];
	indexSpr[ k ] = i;
	k++;
	break;//--- End of this row
      }
    }
  }
  * NZsize = k;
}

void Pick_diag_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int valSpr[], int indexSpr[], int * NZsize){
  int i, j, k = 0, index_row_i_j, N_address;
  int * start_address_int;

  for ( i = 0; i < rowMax ; i++){
    for (j = 0; j < (i + 1); j++){///ch

      //------------1. stop if it is end of the row ---------------//
      //++++ address generator
      start_address_int = address_generator_int( i, NZrowMax + 1, j, &(index_row[ 0 ][ 0 ]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &index_row_i_j, start_address_int, N_address);

      //++++ functional
      if (index_row_i_j >= colMax)//---- End of the row
        break;

      //----------- 2. find the items for col = i in row i -----------------//
      if (index_row_i_j == i){///ch

        //++++ address generator
        start_address_int = address_generator_int( i, NZrowMax, j , &(val_row[ 0 ][ 0 ]));
        N_address = 1;
        //++++ memory access
        mem_read_int( &(valSpr[ k ]), start_address_int, N_address);

        //++++ functonal
        indexSpr[ k ] = i;
        k++;
        break;//--- End of this row
      }
    }
  }
  * NZsize = k;
}

void Pick_diag_LIL(val_col_node *row_list [], int rowMax, int colMax, int colNum, float valSpr[], int indexSpr[], int * NZsize){

  int i, k = 0;
  val_col_node * this_node;

  for ( i = 0; i < rowMax ; i++){
    this_node = row_list[ i ];
    while (this_node != NULL){
      if (this_node->col >= colMax)//----- This is end of the row
	break;

    //--- find the items for col = colNum in row i
      if (this_node->col == colNum){
	valSpr[ k ] = this_node->val;
	indexSpr[ k ] = i;
	k++;
	break;//--- go to the next row
      }
      this_node = this_node->next;
    }
  }
  * NZsize = k;
}

void Pick_diag_LIL_protected_0(val_col_node *row_list [], int rowMax, int colMax, int valSpr[], int indexSpr[], int * NZsize){

  int i, k = 0, this_node_col, N_address;
  val_col_node * this_node;
  val_col_node ** start_address_ptrptr_vcn;
  int * start_address_int;

  for ( i = 0; i < rowMax ; i++){
    //++++ address generator
    start_address_ptrptr_vcn = address_generator_ptr_vcn( i , row_list);
    N_address = 1;
    //++++ memory access
    mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);


    while (this_node != NULL){
      if ( k > VEC_MAX )//---- protection 0
        break;

      //--- 4 = sizeof(int) / sizeof(char)
      start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, col) / 4 , (int *)this_node);
      N_address = 1;
      //++++ memory access
      mem_read_int( &this_node_col, start_address_int, N_address);
      //printf("this_node_col:%d\n", this_node_col);

      //++++ functional
      if (this_node_col >= colMax)//----- This is end of the row
        break;

      //--- find the items for col = i in row i
      if (this_node_col == i){///ch
        start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, val) / 4 , (int *)this_node);
        N_address = 1;
        //++++ memory access
        mem_read_int( &(valSpr[ k ]), start_address_int, N_address);
        //    printf("this_node_val:%d\n", this_node_val);


        //++++ functional
        indexSpr[ k ] = i;
        k++;
        break;//--- go to the next row
      }

      start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4, (int *)this_node));
      //    printf( "address-vcn-2: %d\n", start_address_ptrptr_vcn);
      N_address = 1;
      //++++ memory access
      mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);

    }
  }
  //++++ functional
  * NZsize = k;
}
//====================== UPDATE THE DIAGONAL ========================//
void Update_diag_DENSE_protected_0(int c[ ROWS ][ COLS ], int rowMax, int diag_vec[]){
  int i, N_address = 1, temp;
  int* start_address_int;

  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(diag_vec[0]);  end_spr_addr[ Nspr ] = &(diag_vec[ ROWS - 1 ]);
  Nspr++;
  
  //---- update the matrix  
  //memcpy(&(c[*rowMax][ 0 ]), vec_row, COLS * sizeof (int));
  for ( i = 0; i < rowMax; i++){
    //++++ address generator
    start_address_int = address_generator_int(0, COLS, i, diag_vec);
    //++++ memory access
    mem_read_int( &temp, start_address_int, N_address);
    //++++ address generator
    start_address_int = address_generator_int(i, COLS, i, &(c[0][0]));
    //++++ memory access
    mem_write_int( start_address_int, &temp, N_address);
    }
  
  Nspr--;
  return;
}

void Update_diag_CRS2(float val[], int col[], int rowPtr[], int rowMax, float valSpr[], int indexSpr[], int vecSize){
  int i, i_start, i_next = 0, j = 0, k, v = 0;
  int removed = 0;
  int added = 0;
  
  //--- initialize the new col and val
  int matrix_size = rowPtr[ rowMax ];
  float val_new[ matrix_size + vecSize ];
  int col_new[ matrix_size + vecSize ];

  for ( i = 0; i < rowMax ; i++){
    //--- find the index range for row i
    i_start = i_next;
    i_next = rowPtr[i + 1];
    
    for (k = i_start; k < i_next; k++){
    
      //--- find the items for col = i in row i
      if (col[ k ] == i){
	if (indexSpr[ v ] == i){//---- update the value
	  val_new[ k - removed + added ] = valSpr[ v ];
	  col_new[ k - removed + added ] = i; 	
	  if (v < vecSize - 1)
	    v++;
	}
	else if ((indexSpr[ v ] > i) || (v == vecSize - 1)) //---- zero out the value
	  removed++;
      }

      //---- if not found, add it
      else if( col[ k ] > i || (col[ k ] < i && k == i_next - 1)){
	if (indexSpr[ v ] == i){
	  added++;
	  val_new[ k - removed + added ] = valSpr[ v ];
	  col_new[ k - removed + added ] = i; 	
	  if (v < vecSize - 1)
	    v++;
	}
	while ( k < i_next){
	  val_new[ k - removed + added ] = val[ k ];
	  col_new[ k - removed + added ] = col[ k ]; 	
	  k++;
	}
	break;
      }
      else {
	val_new[ k - removed + added ] = val[ k ];
	col_new[ k - removed + added ] = col[ k ]; 	
      }
    }
    rowPtr[ i + 1 ] += added - removed;
  }//i

  //----update return values
  memcpy(val, val_new, (matrix_size + added - removed) * sizeof (float));
  memcpy(col, col_new, (matrix_size + added - removed) * sizeof (int));

  return;
}

void Update_diag_CRS(float val[], int col[], int rowPtr[], int rowMax, float valSpr[], int indexSpr[], int vecSize){
  int i, i_start, i_next = 0, j = 0, k, v = 0;
  int removed = 0;
  int added = 0;
  
  //--- initialize the new col and val
  int matrix_size = rowPtr[ rowMax ];
  float val_new[ matrix_size + vecSize ];
  int col_new[ matrix_size + vecSize ];

  for (i = 0; i < rowMax; i++){

      //---- find the row range
      i_start = i_next;
      i_next = rowPtr[ i + 1 ];

      //--- find the element in col = i and row i
      for (k = i_start; k < i_next; k++){
	if (col[ k ] == i){
	  if ( v >= vecSize )//---- zero out the element
	    removed ++;
	  else if ( indexSpr[ v ] == i ){//---- just update the value
	    val_new[ k - removed + added ] = valSpr[ v ];
	    col_new[ k - removed + added ] = i;
	    v++;
	  }
	  else//---- zero out
	    removed ++;
	}	
	else if(col[ k ] > i){
	  if ( v < vecSize ){
	    if ( indexSpr[ v ] == i ){//---- add the element
	      val_new[ k - removed + added ] = valSpr[ v ];
	      col_new[ k - removed + added ] = i;
	      added++;
	      v++;
	    }
	  }
	  val_new[ k - removed + added ] = val[ k ];
	  col_new[ k - removed + added ] = col[ k ];
	}
	else if ( k == i_next - 1){
	  val_new[ k - removed + added ] = val[ k ];
	  col_new[ k - removed + added ] = col[ k ];
	  
	  if ( v < vecSize ){
	    if ( indexSpr[ v ] == i ){//---- add the element
	      added++;
	      val_new[ k - removed + added ] = valSpr[ v ];
	      col_new[ k - removed + added ] = i;
	      v++;
	    }
	  }
	}
	else{
	  val_new[ k - removed + added ] = val[ k ];
	  col_new[ k - removed + added ] = col[ k ];
	  }
      }
      rowPtr[ i + 1 ] += added - removed;
  }//i

  //----update return values
  memcpy(val, val_new, (matrix_size + added - removed) * sizeof (float));
  memcpy(col, col_new, (matrix_size + added - removed) * sizeof (int));

  return;
}
void Update_diag_CRS_protected_0(int val[], int col[], int rowPtr[], int rowMax, int valSpr[], int indexSpr[], int vecSize){
  int i, i_start, i_next = 0, j = 0, k, v = 0;
  int removed = 0, added = 0, temp;
  int *start_address_int, N_address, col_k;

  //--- initialize the new col and val
  int matrix_size;
  //++++ address generator
  start_address_int = address_generator_int(0, 0, rowMax, &(rowPtr[0]));
  N_address = 1;
  //++++ memory access
  mem_read_int( &matrix_size, start_address_int, N_address);

  int new_size_pr = min( VEC_MAX, max( matrix_size + vecSize, 0));//---- protection 0
  int val_new[ new_size_pr ];
  int col_new[ new_size_pr ];
  //--- Add them to the sparse formats
  start_spr_addr[ Nspr ] = &(val_new[0]);  end_spr_addr[ Nspr ] = &(val_new[ new_size_pr - 1 ]);
  start_spr_addr[ Nspr + 1 ] = &(col_new[0]);  end_spr_addr[ Nspr + 1 ] = &(col_new[ new_size_pr - 1]);
  Nspr +=2;

  for (i = 0; i < rowMax; i++){

    //---- find the row range
    i_start = i_next;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + 1, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &i_next, start_address_int, N_address);

    int l=0;
    //--- find the element in col = i and row i
    for (k = i_start; k < i_next; k++){

      if (l >= NZmax )//---- protection 0
	break;
      l++;

      //++++ address generator
      start_address_int = address_generator_int(0, 0, k, col);
      N_address = 1;
      //++++ memory access
      mem_read_int( &col_k, start_address_int, N_address);

      if (col_k == i){
	if ( v >= vecSize )//---- zero out the element
	  removed ++;
	else if ( indexSpr[ v ] == i ){//---- just update the value
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed+ added, val_new);
	  N_address = 1;
	  //++++ memory access
	  mem_write_int( start_address_int, &(valSpr[ v ]),N_address);

	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed+ added, col_new);
	  N_address = 1;
	  //++++ memory access
	  mem_write_int( start_address_int, &i,N_address);

	  //++++ functional
	  v++;
	}
	else//---- zero out
	  removed ++;
      }
      else if(col_k  > i){
	if ( v < vecSize ){
	  if ( indexSpr[ v ] == i ){//---- add the element
	    // val_new[ k - removed + added ] = valSpr[ v ];
	    //col_new[ k - removed + added ] = i;
	    //++++ address generator
	    start_address_int = address_generator_int(0, 0, k - removed+ added, val_new);
	    N_address = 1;
	    //++++ memory access
	    mem_write_int( start_address_int, &(valSpr[ v ]),N_address);

	    //++++ address generator
	    start_address_int = address_generator_int(0, 0, k - removed+ added, col_new);
	    N_address = 1;
	    //++++ memory access
	    mem_write_int( start_address_int, &i,N_address);
	    added++;
	    v++;
	  }
	}
	// val_new[ k - removed + added ] = val[ k ];
	//col_new[ k - removed + added ] = col[ k ];
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, val);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	N_address = 1;
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, col);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	N_address = 1;
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);
      }
      else if ( k == i_next - 1){
	// val_new[ k - removed + added ] = val[ k ];
	//col_new[ k - removed + added ] = col[ k ];
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, val);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	N_address = 1;
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, col);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	N_address = 1;
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);

	if ( v < vecSize ){
	  if ( indexSpr[ v ] == i ){//---- add the element
	    added++;
	    //val_new[ k - removed + added ] = valSpr[ v ];
	    //col_new[ k - removed + added ] = i;
	    //++++ address generator
	    start_address_int = address_generator_int(0, 0, k + removed+ added, val_new);
	    N_address = 1;
	    //++++ memory access
	    mem_write_int( start_address_int, &(valSpr[ v ]),N_address);

	    //++++ address generator
	    start_address_int = address_generator_int(0, 0, k + removed+ added, col_new);
	    N_address = 1;
	    //++++ memory access
	    mem_write_int( start_address_int, &i,N_address);

	    //++++ functional
	    v++;
	  }
	}
      }
      else{
	//val_new[ k - removed + added ] = val[ k ];
	//col_new[ k - removed + added ] = col[ k ];
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, val);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	N_address = 1;
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, col);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	N_address = 1;
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);
      }
    }
    //rowPtr[ i + 1 ] += added - removed;
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + 1, rowPtr);
    N_address = 1;
    //++++ memory access
    mem_read_int( &temp, start_address_int, N_address);

    //++++ functional
    temp += added - removed;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + 1, rowPtr);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &temp, N_address);

  }//i

  //----update return values
  //  memcpy(val, val_new, (matrix_size + added - removed) * sizeof (int));
  //memcpy(col, col_new, (matrix_size + added - removed) * sizeof (int));
  int matrix_size_new_pr = min(max( matrix_size + added - removed, 0), VEC_MAX);//---- protection 0

  int temp1[matrix_size_new_pr];
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val_new);
  N_address = matrix_size_new_pr;//matrix_size + added - removed;
  //++++ memory access
  mem_read_int( temp1, start_address_int, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val);
  N_address = matrix_size_new_pr;//matrix_size + added - removed;
  //++++ memory access
  mem_write_int( start_address_int, temp1, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col_new);
  N_address = matrix_size_new_pr;//matrix_size + added - removed;
  //++++ memory access
  mem_read_int( temp1, start_address_int, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col);
  N_address = matrix_size_new_pr;//matrix_size + added - removed;
  //++++ memory access
  mem_write_int( start_address_int, temp1, N_address);

  Nspr -= 2;
  return;
}

void Update_diag_SLL(int val[], int col[], int row[], int rowMax, int valSpr[], int indexSpr[], int vecSize){
  int i, j, k = 0, v = 0;
  int removed = 0;
  int added = 0;
  
  //--- initialize the new col and val
  int val_new[ NZmax + vecSize ];
  int col_new[ NZmax + vecSize ];
  int row_new[ NZmax + vecSize + 1 ];
  
  for ( i = 0; i < rowMax; i++){

    j = col[ k ];
    while (row [ k ] == i){
      
      if (col[ k ] == i){// matrix has it
	if ( v >= vecSize )//---- zero out the element
	  removed ++;
	else if ( indexSpr[ v ] == i ){//---- just update the value
	    val_new[ k - removed + added ] = valSpr[ v ];
	    col_new[ k - removed + added ] = i;
	    row_new[ k - removed + added ] = i;
	    v++;
	  }
	else//---- zero out
	  removed ++;
	
      }	
      else if(col[ k ] > i){// matrix does not have it
	  if ( v < vecSize ){
	    if ( indexSpr[ v ] == i ){//---- add the element
	      val_new[ k - removed + added ] = valSpr[ v ];
	      col_new[ k - removed + added ] = i;
	      row_new[ k - removed + added ] = i;
	      added++;
	      v++;
	    }
	  }
	  val_new[ k - removed + added ] = val[ k ];
	  col_new[ k - removed + added ] = col[ k ];
	  row_new[ k - removed + added ] = row[ k ];
	}

      else if (row[ k + 1 ] > i){//--- it is the last element in the row
	  val_new[ k - removed + added ] = val[ k ];
	  col_new[ k - removed + added ] = col[ k ];
	  row_new[ k - removed + added ] = col[ k ];
	  
	  if ( v < vecSize ){
	    if ( indexSpr[ v ] == i ){//---- add the element
	      added++;
	      val_new[ k - removed + added ] = valSpr[ v ];
	      col_new[ k - removed + added ] = i;
	      row_new[ k - removed + added ] = i;
	      v++;
	    }
	  }
	}
	else{
	  val_new[ k - removed + added ] = val[ k ];
	  col_new[ k - removed + added ] = col[ k ];
	  row_new[ k - removed + added ] = row[ k ];
	  }
      k++;    
    }
  }

  //----update return values
  memcpy(val, val_new, (k + added - removed) * sizeof (int));
  memcpy(col, col_new, (k + added - removed) * sizeof (int));
  memcpy(row, row_new, (k + added - removed) * sizeof (int));
  row[ k + added - removed] = rowMax;
  return;
}


void Update_diag_SLL_protected_0(int val[], int col[], int row[], int rowMax, int valSpr[], int indexSpr[], int vecSize){
  int i, j, k = 0, v = 0;
  int removed = 0, added = 0;
  int *start_address_int, N_address, row_k, row_k_1, col_k, temp;

  //--- initialize the new col and val
  int val_new[ VEC_MAX ];
  int col_new[ VEC_MAX ];
  int row_new[ VEC_MAX + 1 ];
  //---- Add them to the sparse format
  start_spr_addr[ Nspr ] = &( val_new[0] );  end_spr_addr[ Nspr ] = &( val_new[ VEC_MAX - 1 ]);
  start_spr_addr[ Nspr + 1 ] = &( col_new[0] );  end_spr_addr[ Nspr + 1 ] = &( col_new[ VEC_MAX - 1 ]);
  start_spr_addr[ Nspr + 2 ] = &( row_new[0] );  end_spr_addr[ Nspr + 2 ] = &( row_new[ VEC_MAX ]);
  Nspr += 3;

  for ( i = 0; i < rowMax; i++){
    if (i >= VEC_MAX)//---- protection 0
      break;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, k, col);
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);
    j = col_k ;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, k, row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_k, start_address_int, N_address);

    if (row_k  > i){//the row is empty
      if ( v < vecSize){
        if ( indexSpr[ v ] == i ){//---- add the element
          //val_new[ k - removed + added ] = valSpr[ v ];
          //  col_new[ k - removed + added ] = i;
          //  row_new[ k - removed + added ] = i;
	  //++++ address generator
          start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
          N_address = 1;
          //++++ memory access
          mem_write_int( start_address_int, &(valSpr[ v ]), N_address);

          //++++ address generator
          start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
          N_address = 1;
	  //++++ memory access
          mem_write_int( start_address_int, &i,N_address);

          //++++ address generator
          start_address_int = address_generator_int(0, 0, k - removed + added, row_new);
          N_address = 1;
          //++++ memory access
          mem_write_int( start_address_int, &i, N_address);

          //++++ functional
          added++;
          v++;
        }
      }
    }
    int l = 0;
    while (row_k  == i){
      if (l >= VEC_MAX)//---- protection 0
        break;
      l++;

      //++++ address generator
      start_address_int = address_generator_int(0, 0, k, col);
      N_address = 1;
      //++++ memory access
      mem_read_int( &col_k, start_address_int, N_address);

      if (col_k == i){// matrix has it
        if ( v >= vecSize )//---- zero out the element
          removed ++;
        else if ( indexSpr[ v ] == i ){//---- just update the value
          //    val_new[ k - removed + added ] = valSpr[ v ];
          // col_new[ k - removed + added ] = i;
          // row_new[ k - removed + added ] = i;

          //++++ address generator
          start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
          N_address = 1;
          //++++ memory access
          mem_write_int( start_address_int, &(valSpr[ v ]), N_address);

          //++++ address generator
          start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
          N_address = 1;
          //++++ memory access
          mem_write_int( start_address_int, &i,N_address);

          //++++ address generator
          start_address_int = address_generator_int(0, 0, k - removed + added, row_new);
          N_address = 1;
          //++++ memory access
          mem_write_int( start_address_int, &i, N_address);

          //++++ functional
          v++;
        }
        else//---- zero out
          removed ++;
      }
      else if(col_k  > i){// matrix does not have it
	if ( v < vecSize ){
	  if ( indexSpr[ v ] == i ){//---- add the element
	    //val_new[ k - removed + added ] = valSpr[ v ];
	    //col_new[ k - removed + added ] = i;
	    //row_new[ k - removed + added ] = i;

	    //++++ address generator
	    start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	    N_address = 1;
	    //++++ memory access
	    mem_write_int( start_address_int, &(valSpr[ v ]), N_address);

	    //++++ address generator
	    start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	    N_address = 1;
	    //++++ memory access
	    mem_write_int( start_address_int, &i,N_address);

	    //++++ address generator
	    start_address_int = address_generator_int(0, 0, k - removed + added, row_new);
	    N_address = 1;
	    //++++ memory access
	    mem_write_int( start_address_int, &i, N_address);

	    //++++ functional
	    added++;
	    v++;
	  }
	}
	// val_new[ k - removed + added ] = val[ k ];
	//col_new[ k - removed + added ] = col[ k ];
	//row_new[ k - removed + added ] = row[ k ];
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, val);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int,N_address);
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, col);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);

	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, row);
	N_address = 1;
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k - removed + added, row_new);
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);
      }

      else {
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k + l, row);
	N_address = 1;
	//++++ memory access
	mem_read_int( &row_k_1, start_address_int, N_address);

	if (row_k_1 > i){//--- it is the last element in the row
	  //  val_new[ k - removed + added ] = val[ k ];
	  // col_new[ k - removed + added ] = col[ k ];
	  //row_new[ k - removed + added ] = row[ k ];

	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, val);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int( &temp, start_address_int,N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	  //++++ memory access
	  mem_write_int( start_address_int, &temp, N_address);

	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, col);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int( &temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	  //++++ memory access
	  mem_write_int( start_address_int, &temp, N_address);

	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, row);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int( &temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed + added, row_new);
	  //++++ memory access
	  mem_write_int( start_address_int, &temp, N_address);


	  if ( v < vecSize ){
	    if ( indexSpr[ v ] == i ){//---- add the element
	      added++;
	      //      val_new[ k - removed + added ] = valSpr[ v ];
              //col_new[ k - removed + added ] = i;
	      //row_new[ k - removed + added ] = i;
	      //++++ address generator
	      start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	      N_address = 1;
	      //++++ memory access
	      mem_write_int( start_address_int, &(valSpr[ v ]), N_address);

	      //++++ address generator
	      start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	      N_address = 1;
	      //++++ memory access
	      mem_write_int( start_address_int, &i,N_address);

	      //++++ address generator
	      start_address_int = address_generator_int(0, 0, k - removed + added, row_new);
	      N_address = 1;
	      //++++ memory access
	      mem_write_int( start_address_int, &i, N_address);

	      //++++ functional
	      v++;
	    }
	  }
	}
	else{
	  //val_new[ k - removed + added ] = val[ k ];
	  //col_new[ k - removed + added ] = col[ k ];
	  //row_new[ k - removed + added ] = row[ k ];
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, val);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int( &temp, start_address_int,N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed + added, val_new);
	  //++++ memory access
	  mem_write_int( start_address_int, &temp, N_address);

	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, col);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int( &temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed + added, col_new);
	  //++++ memory access
	  mem_write_int( start_address_int, &temp, N_address);

	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, row);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int( &temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k - removed + added, row_new);
	  //++++ memory access
	  mem_write_int( start_address_int, &temp, N_address);
	}
      }
      k++;
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k, row);
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_k, start_address_int, N_address);
    }
    /*printf("i:%d \n", i);
    printVecI(val_new, k-removed+added);
    printVecI(col_new, k-removed+added);
    printVecI(row_new, k-removed+added);
    */
  }

  //----update return values
  int matrix_size_new_pr = min( VEC_MAX, max( 0, k + added - removed));
  int temp1[ matrix_size_new_pr];

  //memcpy(val, val_new, (k + added - removed) * sizeof (int));
  //memcpy(col, col_new, (k + added - removed) * sizeof (int));
  //memcpy(row, row_new, (k + added - removed) * sizeof (int));
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val_new);
  N_address = matrix_size_new_pr;
  //++++ memory access
  mem_read_int( temp1, start_address_int,N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val);
  //++++ memory access
  mem_write_int( start_address_int, temp1, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col_new);
  N_address = matrix_size_new_pr;
  //++++ memory access
  mem_read_int( temp1, start_address_int,N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col);
  //++++ memory access
  mem_write_int( start_address_int, temp1, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row_new);
  N_address = matrix_size_new_pr;
  //++++ memory access
  mem_read_int( temp1, start_address_int,N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row);
  //++++ memory access
  mem_write_int( start_address_int, temp1, N_address);

  //----- Adjust the end of row[]
  //row[ k + added - removed] = rowMax;
  //++++ address generator
  start_address_int = address_generator_int(0, 0, k + added - removed, row);
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &rowMax, N_address);
  Nspr -=3;
  return;
}

void Update_diag_COO_protected_0(int val_col_row[NZmax + 1][ 3 ], int rowMax, int valSpr[], int indexSpr[], int vecSize){
  int i, j, k = 0, v = 0;
  int removed = 0, added = 0;
  int *start_address_int, N_address, row_k, row_k_1, col_k, temp[ 3 ];

  //--- initialize the new col and val
  int val_col_row_new[ VEC_MAX + 1 ][ 3 ];
  //int col_new[ VEC_MAX ];
  //int row_new[ VEC_MAX + 1 ];
  //---- Add them to the sparse format
  start_spr_addr[ Nspr ] = &( val_col_row_new[0][0] );  end_spr_addr[ Nspr ] = &( val_col_row_new[ VEC_MAX ][ 2 ]);
  //  start_spr_addr[ Nspr + 1 ] = &( col_new[0] );  end_spr_addr[ Nspr + 1 ] = &( col_new[ VEC_MAX - 1 ]);
  //start_spr_addr[ Nspr + 2 ] = &( row_new[0] );  end_spr_addr[ Nspr + 2 ] = &( row_new[ VEC_MAX ]);
  //Nspr += 3;
  Nspr++;

  for ( i = 0; i < rowMax; i++){
    if (i >= VEC_MAX)//---- protection 0
      break;

    //++++ address generator
    start_address_int = address_generator_int(k, 3, 1, &val_col_row[0][0] );
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);
    j = col_k ;

    //++++ address generator
    start_address_int = address_generator_int(k, 3, 2, &val_col_row[ 0 ][ 0 ]);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_k, start_address_int, N_address);

    if (row_k  > i){//the row is empty
      if ( v < vecSize){
        if ( indexSpr[ v ] == i ){//---- add the element
          //val_new[ k - removed + added ] = valSpr[ v ];
          //  col_new[ k - removed + added ] = i;
          //  row_new[ k - removed + added ] = i;
	  temp[0] = valSpr[ v ];
	  temp[ 1 ] = i;
	  temp[ 2 ] = i;
	  //++++ address generator
          start_address_int = address_generator_int(k - removed + added, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
          N_address = 3;
          //++++ memory access
          mem_write_int( start_address_int, temp, N_address);

          //++++ functional
          added++;
          v++;
        }
      }
    }
    int l = 0;
    while (row_k  == i){
      if (l >= VEC_MAX)//---- protection 0
        break;
      l++;

      //++++ address generator
      start_address_int = address_generator_int(k, 3, 1, &val_col_row[0][0]);
      N_address = 1;
      //++++ memory access
      mem_read_int( &col_k, start_address_int, N_address);

      if (col_k == i){// matrix has it
        if ( v >= vecSize )//---- zero out the element
          removed ++;
        else if ( indexSpr[ v ] == i ){//---- just update the value
          //    val_new[ k - removed + added ] = valSpr[ v ];
          // col_new[ k - removed + added ] = i;
          // row_new[ k - removed + added ] = i;
	  temp[0] = valSpr[ v ];
	  temp[ 1 ] = i;
	  temp[ 2 ] = i;
	  //++++ address generator
          start_address_int = address_generator_int(k - removed + added, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
          N_address = 3;
          //++++ memory access
          mem_write_int( start_address_int, temp, N_address);

          //++++ functional
          v++;
        }
        else//---- zero out
          removed ++;
      }
      else if(col_k  > i){// matrix does not have it
	if ( v < vecSize ){
	  if ( indexSpr[ v ] == i ){//---- add the element
	    //val_new[ k - removed + added ] = valSpr[ v ];
	    //col_new[ k - removed + added ] = i;
	    //row_new[ k - removed + added ] = i;
	    temp[0] = valSpr[ v ];
	    temp[ 1 ] = i;
	    temp[ 2 ] = i;
	    //++++ address generator
	    start_address_int = address_generator_int(k - removed + added, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
	    N_address = 3;
	    //++++ memory access
	    mem_write_int( start_address_int, temp, N_address);

	    //++++ functional
	    added++;
	    v++;
	  }
	}
	// val_new[ k - removed + added ] = val[ k ];
	//col_new[ k - removed + added ] = col[ k ];
	//row_new[ k - removed + added ] = row[ k ];
	//++++ address generator
	start_address_int = address_generator_int(k, 3, 0, &val_col_row[ 0 ][ 0 ]);
	N_address = 3;
	//++++ memory access
	mem_read_int( temp, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(k - removed + added, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
	//++++ memory access
	mem_write_int( start_address_int, temp, N_address);
      }

      else {
	//++++ address generator
	start_address_int = address_generator_int(k + l, 3, 2, &val_col_row[ 0 ][ 0 ]);
	N_address = 1;
	//++++ memory access
	mem_read_int( &row_k_1, start_address_int, N_address);

	if (row_k_1 > i){//--- it is the last element in the row
	  //  val_new[ k - removed + added ] = val[ k ];
	  // col_new[ k - removed + added ] = col[ k ];
	  //row_new[ k - removed + added ] = row[ k ];
	  //++++ address generator
	  start_address_int = address_generator_int(k, 3, 0, &val_col_row[ 0 ][ 0 ]);
	  N_address = 3;
	  //++++ memory access
	  mem_read_int( temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(k - removed + added, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
	  //++++ memory access
	  mem_write_int( start_address_int, temp, N_address);


	  if ( v < vecSize ){
	    if ( indexSpr[ v ] == i ){//---- add the element
	      added++;
	      //      val_new[ k - removed + added ] = valSpr[ v ];
              //col_new[ k - removed + added ] = i;
	      //row_new[ k - removed + added ] = i;
	      temp[ 0 ] = valSpr[ v ];
	      temp[ 1 ] = i;
	      temp[ 2 ] = i;
	      //++++ address generator
	      start_address_int = address_generator_int(k - removed + added, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
	      N_address = 3;
	      //++++ memory access
	      mem_write_int( start_address_int, temp, N_address);
	     
	      //++++ functional
	      v++;
	    }
	  }
	}
	else{
	  //val_new[ k - removed + added ] = val[ k ];
	  //col_new[ k - removed + added ] = col[ k ];
	  //row_new[ k - removed + added ] = row[ k ];
	  //++++ address generator
	  start_address_int = address_generator_int(k, 3, 0, &val_col_row[ 0 ][ 0 ]);
	  N_address = 3;
	  //++++ memory access
	  mem_read_int( temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(k - removed + added, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
	  //++++ memory access
	  mem_write_int( start_address_int, temp, N_address);
	}
      }
      k++;
      //++++ address generator
      start_address_int = address_generator_int(k, 3, 2,&val_col_row[ 0 ][ 0 ]);
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_k, start_address_int, N_address);
    }
    /*printf("i:%d \n", i);
    printVecI(val_new, k-removed+added);
    printVecI(col_new, k-removed+added);
    printVecI(row_new, k-removed+added);
    */
  }

  //----update return values
  int matrix_size_new_pr = min( VEC_MAX, max( 0, k + added - removed));
  int temp1[ matrix_size_new_pr][ 3 ];

  //memcpy(val, val_new, (k + added - removed) * sizeof (int));
  //memcpy(col, col_new, (k + added - removed) * sizeof (int));
  //memcpy(row, row_new, (k + added - removed) * sizeof (int));
  //++++ address generator
  start_address_int = address_generator_int(0, 3, 0, &val_col_row_new[ 0 ][ 0 ]);
  N_address = 3 * matrix_size_new_pr;
  //++++ memory access
  mem_read_int( &temp1[0][0], start_address_int, N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 3, 0, &val_col_row[ 0 ][ 0 ]);
  //++++ memory access
  mem_write_int( start_address_int, &temp1[0][0], N_address);
	  

  //----- Adjust the end of row[]
  //row[ k + added - removed] = rowMax;
  //++++ address generator
  start_address_int = address_generator_int(k + added - removed, 3, 2, &val_col_row[0][0]);
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &rowMax, N_address);
  Nspr --;
  return;
}


void Update_diag_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int valSpr[], int indexSpr[], int vecSize){
  int i, v = 0;

  for (i = 0 ; i < rowMax; i++){
    int j = 0;
    while ( index_row[ i ][ j ] != colMax){
    
      if (index_row[ i ][ j ] == i){//---- matrix has it
	  if ( v >= vecSize ){//---- zero out the element
	    memcpy(&(index_row[ i ][ j ]), &(index_row[ i ][ j + 1]), (NZrowMax + 1 - j) * sizeof(int) );
	    memcpy(&(val_row[ i ][ j ]), &(val_row[ i ][ j + 1]), (NZrowMax - j) * sizeof(int) );
	  }
	  else if ( indexSpr[ v ] == i ){//---- just update the value
	    val_row[ i ][ j ] = valSpr[ v ];
	    v++;
	  }
	  else{//---- zero out
	    memcpy(&(index_row[ i ][ j ]), &(index_row[ i ][ j + 1]), (NZrowMax + 1 - j) * sizeof(int) );
	    memcpy(&(val_row[ i ][ j ]), &(val_row[ i ][ j + 1]), (NZrowMax - j) * sizeof (int) );

	    }
	  break;
      }	
      else if(index_row[ i ][ j ] > i){//---- matrix did not have it
	if ( v < vecSize ){
	  if ( indexSpr[ v ] == i ){//---- add the element right before it
	    //---- shift all one left
	    memcpy(&(index_row[ i ][ j + 1]), &(index_row[ i ][ j ]), (NZrowMax + 2 - j) * sizeof(int) );
	    memcpy(&(val_row[ i ][ j + 1]), &(val_row[ i ][ j ]), (NZrowMax + 1 - j) * sizeof(int) );

	    //---- add the new element
	    index_row[ i ][ j ] = i;
	    val_row[ i ][ j ] = valSpr[ v ]; 
	    v++;
	  }
	}
	break;
      }
    }
    if ( index_row[ i ][ j ] == colMax){//--- we are at the end of the matrix and it did not have it
	  if ( v < vecSize ){
	    if ( indexSpr[ v ] == i ){//---- add the element right befor this
	      index_row[ i ][ j ] = i;
	      val_row[ i ][ j ] = valSpr[ v ];
	      index_row[ i ][ j + 1 ] = colMax; 
	      v++;
	    }
	  }
    }
  }
  return;
}    

void Update_diag_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int valSpr[], int indexSpr[], int vecSize){
  int i, v = 0;
  int i2 =1;
  int j;
  int N_address, index_row_i_j, row_size_pr;
  int * start_address_int;
  int temp1[NZrowMax + 1];

  for (i = 0 ; i < rowMax; i++){
    j = 0;

    //---- case 1 nothing in the vector
    if (v >= vecSize){

      //++++ address generator
      start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
      N_address = 1;
      //++++ memory access
      mem_read_int( &index_row_i_j, start_address_int,N_address);

      //while(index_row[ i ][ j ] <= i){
      while(index_row_i_j  <= i){
        if (index_row_i_j == i){
	  //---- remove it
          //memcpy(&(index_row[ i ][ j ]), &(index_row[ i ][ j + 1]), (NZrowMax  - j) * sizeof(int) );
          row_size_pr = max(min(NZrowMax - j, NZrowMax ), 0);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax + 1, j + 1, &index_row[0][0]);
          N_address = row_size_pr;
          //++++ memory access
          mem_read_int( temp1, start_address_int,N_address);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
          //++++ memory access
          mem_write_int( start_address_int, temp1, N_address);


          //      memcpy(&(val_row[ i ][ j ]), &(val_row[ i ][ j + 1]), (NZrowMax -1- j) * sizeof(int) );
          row_size_pr = max(min(NZrowMax - j - 1, NZrowMax - 1 ), 0);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax, j + 1, &val_row[0][0]);
          N_address = row_size_pr;
          //++++ memory access
          mem_read_int( temp1, start_address_int,N_address);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax, j, &val_row[0][0]);
          //++++ memory access
          mem_write_int( start_address_int, temp1, N_address);

          break;
        }
        j++;
        if (j >= VEC_MAX)//--- protection 0
          break;

        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
        N_address = 1;
        //++++ memory access
        mem_read_int( &index_row_i_j, start_address_int,N_address);
      }
    }
    //---- case 2 vector does not have it
    else if (indexSpr[ v ] > i){

      //++++ address generator
      start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
      N_address = 1;
      //++++ memory access
      mem_read_int( &index_row_i_j, start_address_int,N_address);

      while(index_row_i_j  <= i){
        if (index_row_i_j == i){
          //---- remove it
	  //memcpy(&(index_row[ i ][ j ]), &(index_row[ i ][ j + 1]), (NZrowMax  - j) * sizeof(int) );
          row_size_pr = max(min(NZrowMax - j, NZrowMax ), 0);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax + 1, j + 1, &index_row[0][0]);
          N_address = row_size_pr;
          //++++ memory access
          mem_read_int( temp1, start_address_int,N_address);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
          //++++ memory access
          mem_write_int( start_address_int, temp1, N_address);


          //      memcpy(&(val_row[ i ][ j ]), &(val_row[ i ][ j + 1]), (NZrowMax -1- j) * sizeof(int) );
          row_size_pr = max(min(NZrowMax - j - 1, NZrowMax - 1 ), 0);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax, j + 1, &val_row[0][0]);
          N_address = row_size_pr;
          //++++ memory access
          mem_read_int( temp1, start_address_int,N_address);
          //++++ address generator
          start_address_int = address_generator_int(i, NZrowMax, j, &val_row[0][0]);
          //++++ memory access
          mem_write_int( start_address_int, temp1, N_address);
          break;
        }
        j++;
        if (j >= VEC_MAX)
          break;

        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
        N_address = 1;
        //++++ memory access
        mem_read_int( &index_row_i_j, start_address_int,N_address);
      }
    }
    //----- case 3 vector has it
    else if (indexSpr[ v ] == i){

      //++++ address generator
      start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
      N_address = 1;
      //++++ memory access
      mem_read_int( &index_row_i_j, start_address_int,N_address);

      while(index_row_i_j  < i){
        j++;
        if (j >= VEC_MAX)//---- protection 0
          break;
	//++++ address generator
        start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
        N_address = 1;
        //++++ memory access
        mem_read_int( &index_row_i_j, start_address_int,N_address);
      }

      //---- case 3.1 array has it=> update it
      if (index_row_i_j == i){
        //val_row[ i ][ j ]= valSpr[ v ];
        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax, j, &val_row[0][0]);
        N_address = 1;
        //++++ memory access
        mem_write_int( start_address_int, &valSpr[ v ], N_address);

	v++;
      }

      //---- case 3.2 array does not have it => add it it
      else{
        //memcpy(&(index_row[ i ][ j + 1 ]), &(index_row[ i ][ j ]), (NZrowMax  - j) * sizeof(int) );
        //memcpy(&(val_row[ i ][ j + 1]), &(val_row[ i ][ j ]), (NZrowMax -1- j) * sizeof(int) );
        row_size_pr = max(min(NZrowMax - j, NZrowMax ), 0);
        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
        N_address = row_size_pr;
        //++++ memory access
        mem_read_int( temp1, start_address_int,N_address);
        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax + 1, j + 1, &index_row[0][0]);
        //++++ memory access
        mem_write_int( start_address_int, temp1, N_address);


        row_size_pr = max(min(NZrowMax - j - 1, NZrowMax - 1 ), 0);
        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax, j, &val_row[0][0]);
        N_address = row_size_pr;
        //++++ memory access
        mem_read_int( temp1, start_address_int,N_address);
        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax, j + 1, &val_row[0][0]);
        //++++ memory access
        mem_write_int( start_address_int, temp1, N_address);


        //index_row[ i ][ j ] = i;
        //val_row[ i ][ j ] = valSpr[ v ];
        //++++ address generator
	start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
        N_address = 1;
        //++++ memory access
        mem_write_int( start_address_int, &i, N_address);

        //++++ address generator
        start_address_int = address_generator_int(i, NZrowMax, j, &val_row[0][0]);
        //++++ memory access
        mem_write_int( start_address_int, &valSpr[ v ], N_address);
        v++;
      }
    }
  }
  return;
}

void Update_diag_LIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int valSpr[], int indexSpr[], int vecSize){
  int v = 0, i;
  printf("rowMax:%d, vecSize:%d\n",rowMax, vecSize);

  for (i = 0; i < rowMax; i++){
    //printf("%d\n", i);
    if (indexSpr[ v ] == i){//---- add or update it
      add_node(row_list, LIL_nodes, i, i, valSpr[ v ]);
      //printf("val[%d]: %d, index:%d\n", v,valSpr[v],i);
      if (v < vecSize - 1)
	v++;
    }
    else//-----remove the node i = j if any
      remove_node(row_list, LIL_nodes, i, i);
  }
  return;
}

void Update_diag_LIL_protected_0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int valSpr[], int indexSpr[], int vecSize){
  int v = 0, i;
  int vecSize_pr = max(min(vecSize, VEC_MAX),0);
  
  for (i = 0; i < rowMax; i++){
    if (indexSpr[ v ] == i){//---- add or update it
      add_node_p0(row_list, LIL_nodes, i, i, valSpr[ v ]);
      if (v < vecSize_pr - 1)
	v++;
    }
    else//-----remove the node i = j if any
      remove_node_p0(row_list, LIL_nodes, i, i);
  }
  return;
}


//===================== OP on SPARSE VECTORS ======================//
//--- This method operates an item-by-item operation on "a" and "b" and updates "c" with the result.
//--- Char op determines the type of the operation:
//--- + add, - minus, . multiplication, % devision

void spr_vectors_op (int a_valSpr[], int a_indexSpr[], int a_size, int b_valSpr[], int b_indexSpr[], int b_size, int c_valSpr[], int c_indexSpr[], int *c_size, char op){
  int ia, ib, ic, i;
  int a_size_protected = min( VEC_MAX, max(0, a_size));
  int b_size_protected = min( VEC_MAX, max(0, b_size));

  //++++++ it is all functional
  switch(op){
  case '+':
    ia = 0;
    ib = 0;
    ic = 0;
    while (ia < a_size_protected && ib < b_size_protected){
      if (ic >= VEC_MAX)//---- protection 0
	break;
      if (a_indexSpr[ia] == b_indexSpr[ib]){
	if ( (a_valSpr[ia] + b_valSpr[ib]) != 0){
	  c_indexSpr[ic] = a_indexSpr[ia];
	  c_valSpr[ic] = a_valSpr[ia] + b_valSpr[ib];
	  ic++;
	}
	ia++;
	ib++;
      }
      else if (a_indexSpr[ia] < b_indexSpr[ib]){
	c_indexSpr[ic] = a_indexSpr[ia];
	c_valSpr[ic] = a_valSpr[ia];
	ia++;
	ic++;
      }
      else if (a_indexSpr[ia] > b_indexSpr[ib]){
	c_indexSpr[ic] = b_indexSpr[ib];
	c_valSpr[ic] = b_valSpr[ib];
	ib++;
	ic++;
      }
    }
    
    //----- If one vector has remaining
    int remaining = 0;
    if (ia < a_size_protected){
      remaining = a_size_protected - ia;
      remaining = max(0, min (remaining , VEC_MAX - 1 - ic));
      //printf("remaining: %d, ic: %d, VEC_MAX:%d\n", remaining, ic, VEC_MAX);
      memcpy (&(c_valSpr[ic]), &(a_valSpr[ia]), remaining * sizeof(int));
      memcpy (&(c_indexSpr[ic]), &(a_indexSpr[ia]), remaining * sizeof(int));
      ic += remaining;
    }
    else if (ib < b_size_protected){
      //printf("ic: %d\n", ic);
      remaining = b_size_protected - ib;
      remaining = max( 0, min (remaining , VEC_MAX - 1 - ic));
      memcpy (&(c_valSpr[ic]), &(b_valSpr[ib]), remaining * sizeof(int));
      memcpy (&(c_indexSpr[ic]), &(b_indexSpr[ib]), remaining * sizeof(int));
      ic += remaining;
    }
    
      //    c_valSpr[ic] = 0;//--- set the last item to zero
    break;
  case '-':
    ia = 0;
    ib = 0;
    ic = 0;
    while (ia < a_size_protected && ib < b_size_protected){
      if (a_indexSpr[ia] == b_indexSpr[ib]){
	if ( (a_valSpr[ia] - b_valSpr[ib]) != 0){
	  c_indexSpr[ic] = a_indexSpr[ia];
	  c_valSpr[ic] = a_valSpr[ia] - b_valSpr[ib];
	  ic++;
	}
	ia++;
	ib++;
      }
      else if (a_indexSpr[ia] < b_indexSpr[ib]){
	c_indexSpr[ic] = a_indexSpr[ia];
	c_valSpr[ic] = a_valSpr[ia];
	ia++;
	ic++;
      }
      else if (a_indexSpr[ia] > b_indexSpr[ib]){
	c_indexSpr[ic] = b_indexSpr[ib];
	c_valSpr[ic] = 0 - b_valSpr[ib];
	ib++;
	ic++;
      }
    }

    //----- If one vector has remaining
    if (ia < a_size_protected){
      memcpy (&(c_valSpr[ic]), &(a_valSpr[ia]), (a_size_protected - ia) * sizeof(int));
      memcpy (&(c_indexSpr[ic]), &(a_indexSpr[ia]), (a_size_protected - ia) * sizeof(int));
      ic += (a_size_protected - ia);
    }
    else if (ib < b_size_protected){
      for (i = 0; i < (b_size_protected - ib ); i++)
	c_valSpr[ic + i] = 0 - b_valSpr[ib + i];
      memcpy (&(c_indexSpr[ic]), &(b_indexSpr[ib]), (b_size_protected - ib) * sizeof(int));
      ic += b_size_protected - ib;
    }
    break;
  case '.':
    ia = 0;
    ib = 0;
    ic = 0;
    while (ia < a_size_protected && ib < b_size_protected){
      if (a_indexSpr[ia] == b_indexSpr[ib]){
	c_indexSpr[ic] = a_indexSpr[ia];
	c_valSpr[ic] = a_valSpr[ia] * b_valSpr[ib];
	ia++;
	ib++;
	ic++;
      }
      else if (a_indexSpr[ia] < b_indexSpr[ib])
	ia++;
      else
	ib++;
    }
    //c_valSpr[ic] = 0;//--- set the last item to zero
    break;
  case '%': //---- it returns 0 for division by zero
    ia = 0;
    ib = 0;
    ic = 0;
    while (ia < a_size_protected && ib < b_size_protected){
      if (a_indexSpr[ia] == b_indexSpr[ib]){
	c_indexSpr[ic] = a_indexSpr[ia];
	c_valSpr[ic] = a_valSpr[ia] / b_valSpr[ib];
	ia++;
	ib++;
	ic++;
      }
      else if (a_indexSpr[ia] < b_indexSpr[ib])
	ia++;
      else
	ib++;
    }
    //c_valSpr[ic] = 0;//--- set the last item to zero
    break;
  }
  *c_size = ic;
  return;
}

void spr_vectors_op_DENSE(int a[], int b[], int c[], int vec_size, char op){
  int N_address = 1, i, temp1, temp2;
  int * start_address_int;

  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(a[0]);  end_spr_addr[ Nspr ] = &(a[ COLS - 1 ]);
  start_spr_addr[ Nspr + 1 ] = &(b[0]);  end_spr_addr[ Nspr + 1 ] = &(b[ COLS - 1 ]);
  start_spr_addr[ Nspr + 2 ] = &(c[0]);  end_spr_addr[ Nspr + 2 ] = &(c[ COLS - 1 ]);
  Nspr += 3;

    for (i = 0; i < vec_size; i++){
      //temp1 = a[ i ];
      //++++ address generator
      start_address_int = address_generator_int(0, ROWS, i, a);
      //++++ memory access
      mem_read_int( &temp1, start_address_int, N_address);
  
      //temp2 = b[ i ];
      //++++ address generator
      start_address_int = address_generator_int(0, ROWS, i, b);
      //++++ memory access
      mem_read_int( &temp2, start_address_int, N_address);

      switch(op){
      case '+':
	temp1 = temp1 + temp2;
	break;
      case '-':
	temp1 = temp1 - temp2;
	break;
      case '.':
	temp1 = temp1 * temp2;
	break;
      }

      //c[ i ] = temp1 OP temp2;
      //++++ address generator
      start_address_int = address_generator_int(0, ROWS, i, c);
      //++++ memory access
      mem_write_int( start_address_int, &temp1, N_address);
    }

  Nspr -= 3;
  return;
}

//============== REDUCTION of SPARSE VECTOR ==================//
//--- This method reduces a sparse vector by an operation. The result is a float based on the op:
//--- u (mean), S (sum), s (sigma), M (max), m (min)

float spr_vector_reduction (int valSpr[], int indexSpr[], int vecNZ, int vecMax, char op){
  int i = 0;
  float sum = 0, meanVal, maxVal, minVal;
  
  int vecNZ_protected = min( VEC_MAX ,max(vecNZ, 0));

  //++++++ ALL is functional
  switch(op){
  case 'u':
    for (i = 0; i < vecNZ_protected; i++)      
      sum += valSpr[i];
    sum = sum / vecMax;
    return sum;
  case 'S':
    sum = 0;
    for (i = 0; i < vecNZ_protected; i++)      
      sum += valSpr[i];
    return sum;
  case 'm':
    minVal = valSpr[0];
    for (i = 0; i < vecNZ_protected; i++)      
      minVal = min(minVal, valSpr[i]);
    return minVal;
  case 'M':
    maxVal = valSpr[0];
    for (i = 0; i < vecNZ_protected; i++)      
      maxVal = max(maxVal, valSpr[i]);
    return maxVal;
  case 's':
    //--- Calculate the mean
    for (i = 0; i < vecNZ_protected; i++)      
      sum += valSpr[i];
    meanVal = (sum / (float)vecMax);
    
    //--- Calculate the sigma
    sum = 0;
    for (i = 0; i < vecNZ_protected; i++)      
      sum += pow((valSpr[i] - meanVal), 2);
    sum += (vecMax - vecNZ_protected) * pow(meanVal, 2);//---- for zero values
    return (sqrt(sum / (float)vecMax));
  }
  return 0;
}

float spr_vector_reduction_DENSE (int a[], int a_size, char op){
  int i = 0, temp, N_address = 1;
  float sum = 0, meanVal, maxVal = 0, minVal = 0;
  int * start_address_int;

  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(a[0]);  end_spr_addr[ Nspr ] = &(a[ a_size - 1 ]);
  Nspr++;

  //---- vector reduction  
  for (i = 0; i < a_size; i++){
    //temp = a[ i ];
    //++++ address generator
    start_address_int = address_generator_int(0, a_size, i, a);
    //++++ memory access
    mem_read_int( &temp, start_address_int, N_address);

    //++++ functional
    switch(op){
    case 'u':
    case 's':
    case 'S':
      sum += temp; break;
    case 'm':
      minVal = min(minVal, temp); break;
    case 'M':
      maxVal = max(maxVal, temp); break;
    }
  }

  if (op == 'u')
    sum = sum / a_size;
  else if ( op == 's'){
    //--- Calculate the sigma
      meanVal = sum / a_size;
      sum = 0;
    for (i = 0; i < a_size; i++){      
      //temp = a[i];
      //++++ address generator
      start_address_int = address_generator_int(0, a_size, i, a);
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      
      //++++ functional
      sum += pow((temp - meanVal), 2);
    }
    sum =  (sqrt(sum / (float)a_size));
  }
  //printf("%f\n", sum);
  Nspr--;
  return sum;
}

//================== APPEND a ROW ====================//
void Append_a_row_DENSE_protected_0(int c[ ROWS ][ COLS ], int row_vec[], int * rowMax){
  int N_address, temp[ COLS ];
  int* start_address_int;

  //---- update the matrix  
  //memcpy(&(c[*rowMax][ 0 ]), vec_row, COLS * sizeof (int));
  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(row_vec[0]);  end_spr_addr[ Nspr ] = &(row_vec[ COLS - 1 ]);
  Nspr++;

  //++++ address generator
  start_address_int = address_generator_int(0, COLS, 0, &(row_vec[0]));
  N_address = COLS;
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  //++++ address generator
  start_address_int = address_generator_int(*rowMax, COLS, 0, &(c[0][0]));
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);

  *rowMax = *rowMax + 1;
  Nspr--;
}

void Append_a_row_CRS(int val[], int col[], int rowPtr[], int valSpr[], int indexSpr[], int size_vec, int * rowMax){
  
  //---- find the size of the matrix and the vectoer
  int size_matrix = rowPtr[( *rowMax)]; 
  
  //---- update the matrix  
  memcpy(&(val[size_matrix]), valSpr, (size_vec) * sizeof (float));
  memcpy(&(col[size_matrix]), indexSpr, (size_vec) * sizeof (int));
  rowPtr[( *rowMax) + 1] = size_matrix + size_vec;

  *rowMax = *rowMax + 1;
}

void Append_a_row_CRS_protected_0(int val[], int col[], int rowPtr[], int valSpr[], int indexSpr[], int size_vec, int * rowMax){
  int size_matrix, N_address, size_vec_pr;
  int * start_address_int; 
  
  //---- find the size of the matrix and the vectoer
  //++++ address generator
  start_address_int = address_generator_int(0, 0, (*rowMax), rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &size_matrix, start_address_int, N_address);
  
  //---- update the matrix 
  size_vec_pr = max( 0, min(size_vec, VEC_MAX));//--- protection 0
 
  //++++ address generator
  start_address_int = address_generator_int(0, 0, size_matrix, val);
  N_address = size_vec_pr;
  //++++ memory access
  mem_write_int( start_address_int, valSpr, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, size_matrix, col);
  N_address = size_vec_pr;
  //++++ memory access
  mem_write_int( start_address_int, indexSpr, N_address);

  //++++ functional
  size_matrix += size_vec;

  //++++ address generator
  start_address_int = address_generator_int(0, 0, (*rowMax + 1), rowPtr);
  N_address = 1;
  //++++ memory access
  mem_write_int(start_address_int, &size_matrix, N_address);

  //++++ functional
  *rowMax = *rowMax + 1;
}


void Append_a_row_CRS_protected_1(int val[], int col[], int rowPtr[], int valSpr[], int indexSpr[], int size_vec, int * rowMax){
  int size_matrix, N_address, size_vec_pr;
  int * start_address_int; 
  
  //---- find the size of the matrix and the vectoer
  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, 0, (*rowMax), 0, ROWS, rowPtr);
  N_address = 1;
  //++++ memory access
    mem_read_int_p1( &size_matrix, start_address_int, N_address, 0, NZmax);
  //  mem_read_int( &size_matrix, start_address_int, N_address);
  
  //---- update the matrix 
  size_vec_pr = max( 0, min(size_vec, VEC_MAX));//--- protection 0
 
  //++++ address generator
    start_address_int = address_generator_int_p1(0, 0, 0, VEC_MAX, size_matrix, 0, VEC_MAX - 1, val);
  //start_address_int = address_generator_int(0, VEC_MAX, size_matrix, val);
  N_address = size_vec_pr;
  //++++ memory access
  mem_write_int( start_address_int, valSpr, N_address);

  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, VEC_MAX, size_matrix, 0, VEC_MAX - 1, col);
  N_address = size_vec_pr;
  //++++ memory access
  mem_write_int_p1( start_address_int, indexSpr, N_address, 0, COLS - 1);

  //++++ functional
  size_matrix += size_vec;

  //++++ address generator
  start_address_int = address_generator_int_p1(0, 0, 0, 0, (*rowMax + 1), 0, ROWS, rowPtr);
  N_address = 1;
  //++++ memory access
  mem_write_int_p1(start_address_int, &size_matrix, N_address, 0, NZmax);

  //++++ functional
  *rowMax = *rowMax + 1;
}

void Append_a_row_SLL_protected_0(int val[], int col[], int row[], int valSpr[], int indexSpr[], int size_vec, int *rowMax){
  int size_matrix = 0, row_size_matrix, size_vec_pr, N_address;
  int * start_address_int;
  //---- find the size of the matrix and the vectoer
  //++++ address generator
    start_address_int = address_generator_int(0, 0, size_matrix, row);
    N_address = 1;
    //++++ memory access
    mem_read_int(&row_size_matrix, start_address_int, N_address);

  while (row_size_matrix < *rowMax){
    //++++ functional
    size_matrix++;
    if (size_matrix >= NZmax)//--- protection 0 for loop size
      break;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, size_matrix, row);
    N_address = 1;
    //++++ memory access
    mem_read_int(&row_size_matrix, start_address_int, N_address);
  }

  //---- update the matrix  
  size_vec_pr = max( 0, min( size_vec, VEC_MAX));

  //++++ address generator
  start_address_int = address_generator_int(0, 0, size_matrix, val);
  N_address = size_vec_pr;
  //++++ memory access
  mem_write_int( start_address_int, valSpr, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, size_matrix, col);
  N_address = size_vec_pr;
  //++++ memory access
  mem_write_int( start_address_int, indexSpr, N_address);

  int k;
  for (k = 0; k < size_vec_pr; k++){
    //++++ address generator
    start_address_int = address_generator_int(0, 0, size_matrix + k, row);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, rowMax, N_address);
  }

  //---- set the end of the matrix and the size
  //++++ functional
  *rowMax = *rowMax + 1;

  //++++ address generator
  start_address_int = address_generator_int(0, 0, size_matrix + size_vec, row);
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, rowMax, N_address);
}

void Append_a_row_SLL(int val[], int col[], int row[], int valSpr[], int indexSpr[], int size_vec, int *rowMax){

  //---- find the size of the matrix and the vectoer
  int size_matrix = 0;
  while (row[size_matrix] != *rowMax)
    size_matrix++;
 
  //---- update the matrix  
  memcpy(&(val[size_matrix]), valSpr, (size_vec) * sizeof (int));
  memcpy(&(col[size_matrix]), indexSpr, (size_vec) * sizeof (int));
  int k;
  for (k = 0; k < size_vec; k++)
    row[size_matrix + k] = *rowMax;
  //memset(&(row[size_matrix]), (*rowMax), (size_vec));

  //---- set the end of the matrix and the size
  row[size_matrix + size_vec] = (*rowMax) + 1;
  *rowMax = *rowMax + 1;
}

void Append_a_row_COO_protected_0(int val_col_row[ NZmax ][ 3 ], int valSpr[], int indexSpr[], int size_vec, int *rowMax){
  int size_matrix = 0, row_size_matrix, size_vec_pr, N_address;
  int * start_address_int;
  //---- find the size of the matrix and the vectoer
  //++++ address generator
  start_address_int = address_generator_int(size_matrix, 3, 2, &(val_col_row[ 0 ][ 0 ]));
  N_address = 1;
  //++++ memory access
  mem_read_int(&row_size_matrix, start_address_int, N_address);

  while (row_size_matrix < *rowMax){
    //++++ functional
    size_matrix++;
    if (size_matrix >= NZmax)//--- protection 0 for loop size
      break;

    //++++ address generator
    start_address_int = address_generator_int(size_matrix, 3, 2, &(val_col_row[ 0 ][ 0 ]));
    N_address = 1;
    //++++ memory access
    mem_read_int(&row_size_matrix, start_address_int, N_address);
  }

  //---- update the matrix   
  size_vec_pr = max( 0, min( size_vec, VEC_MAX));
 
  int k;
  for (k = 0; k < size_vec_pr; k++){
    //++++ address generator
    start_address_int = address_generator_int(size_matrix + k, 3, 0, &(val_col_row[ 0 ][ 0 ]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(valSpr[ k ]), N_address);

    //++++ address generator
    start_address_int = address_generator_int(size_matrix + k, 3, 1, &(val_col_row[ 0 ][ 0 ]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(indexSpr[ k ]), N_address);

  //++++ address generator
    start_address_int = address_generator_int(size_matrix + k, 3, 2, &(val_col_row[ 0 ][ 0 ]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, rowMax, N_address);
  }

  //---- set the end of the matrix and the size
  //++++ functional
  *rowMax = *rowMax + 1;

  //++++ address generator
  start_address_int = address_generator_int(size_matrix + k, 3, 2, &(val_col_row[ 0 ][ 0 ]));
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, rowMax, N_address);
}

/*
void Append_a_row_COO(float val_col_row[ ROWS ][ 3 ], int rowNum, float valSpr[], int indexSpr[], int * NZsize){

  //---- find the size of the matrix and the vectoer
  int size_matrix = rowNum;
  while (val_col_row[size_matrix][ 0 ] != 0)
    size_matrix++;
 
  int size_vec = 0;
  while (valSpr[ size_vec ] != 0)
    size_vec++;

  //---- update the matrix  
  int k = size_matrix;
  int i = 0;
  for (k = size_matrix; k < (size_matrix + size_vec); k++){
    val_col_row[ k ][ 0 ] = valSpr[ i ];
    val_col_row[ k ][ 1 ] = (float) indexSpr[ i ];
    val_col_row[ k ][ 2 ] = (float) rowNum;
    i++;
  }
}
*/
void Append_a_row_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int valSpr[], int indexSpr[], int size_vec, int colMax, int *rowMax){

  //---- update the matrix  
  memcpy(&(val_row[(*rowMax)][0]), valSpr, (size_vec) * sizeof (int));
  memcpy(&(index_row[(*rowMax)][0]), indexSpr, (size_vec) * sizeof (int));

  //---- set the end of the row and the rowMax
  index_row[(*rowMax)][ size_vec ] = colMax;
  *rowMax = *rowMax + 1;
}
void Append_a_row_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int valSpr[], int indexSpr[], int size_vec, int colMax, int *rowMax){

  int N_address;
  int *start_address_int; 
  int size_vec_protected = max(min(size_vec, NZrowMax), 0);
  
  //---- update the matrix  
  // memcpy(&(val_row[(*rowMax)][0]), valSpr, (size_vec) * sizeof (int));
  //  memcpy(&(index_row[(*rowMax)][0]), indexSpr, (size_vec) * sizeof (int));
  //++++ address generator
  start_address_int = address_generator_int( (*rowMax), NZrowMax, 0, &(val_row[ 0 ][ 0 ]));
  N_address = size_vec_protected;
  //++++ memory access
  mem_write_int( start_address_int, valSpr, N_address);
  
  size_vec_protected = max(min(size_vec, NZrowMax + 1), 0);
  //++++ address generator
  start_address_int = address_generator_int( (*rowMax), NZrowMax + 1, 0, &(index_row[ 0 ][ 0 ]));
  N_address = size_vec_protected;
  //++++ memory access
  mem_write_int( start_address_int, indexSpr, N_address);
  
  //---- set the end of the row and the rowMax
  //index_row[(*rowMax)][ size_vec ] = colMax;
  //++++ address generator
  start_address_int = address_generator_int( (*rowMax), NZrowMax + 1, size_vec_protected, &(index_row[ 0 ][ 0 ]));
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &colMax, N_address);
  
  //++++ functional
  *rowMax = *rowMax + 1;
}

void Append_a_row_LIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int valSpr[], int indexSpr[], int vec_size, int* rowMax){

  int k, val_i_j, j, i = *rowMax;
  val_col_node * this_node;

  row_list[ i ] = &LIL_nodes[ i ][ 0 ];
  this_node = row_list[ i ];
  
  for(k = 0; k < vec_size; k++){
    j = indexSpr[ k ];
    val_i_j = valSpr[ k ];

    //---- add the new node
    this_node->val = val_i_j;
    this_node->col = j;
    this_node->next = &(LIL_nodes[ i ][ k + 1 ]);

    //--- next node
    this_node = this_node->next;
  }
  (LIL_nodes[ i ][ vec_size - 1 ]).next = NULL;
  *rowMax = *rowMax + 1;
}


void Append_a_row_LIL_protected_0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int valSpr[], int indexSpr[], int vec_size, int* rowMax){

  int k, val_i_j, j, i = *rowMax, N_address;
  val_col_node * this_node;
  int *start_address_int;
  val_col_node ** start_address_ptrptr_vcn, *temp_vcn;

  
  //row_list[ i ] = &LIL_nodes[ i ][ 0 ];
  start_address_ptrptr_vcn = row_list + i;
  if (in_spr_bound( (int *) start_address_ptrptr_vcn ))
    *start_address_ptrptr_vcn = &LIL_nodes[ i ][ 0 ];//---- this is just initialization

  //this_node = row_list[ i ];
  //++++ address generator
  start_address_ptrptr_vcn = address_generator_ptr_vcn( i , row_list);
  N_address = 1;
  //++++ memory access
  mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);
      

  for(k = 0; k < vec_size; k++){
    //printf("k:%d ",k);

    j = indexSpr[ k ];
    val_i_j = valSpr[ k ];

    //---- add the new node
    
    //this_node->val = val_i_j;
    N_address = 1;
    //++++ address generator
    start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, val) / 4 , (int *)this_node);
    //++++ memory access
    mem_write_int( start_address_int, &val_i_j, N_address);

    //this_node->col = j;
    //++++ address generator
    start_address_int = address_generator_int( 0, 0, offsetof(val_col_node, col) / 4 , (int *)this_node);
    //++++ memory access
    mem_write_int( start_address_int, &j, N_address);

    //this_node->next = &(LIL_nodes[ i ][ k + 1 ]);
    temp_vcn = (val_col_node *)&(LIL_nodes[ i ][ k + 1 ]);
    start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4, (int *) this_node));
    //++++ memory access
    mem_write_ptrptr_vcn(start_address_ptrptr_vcn, &temp_vcn, N_address);


    //--- next node
    //this_node = this_node->next;
    start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4, (int *) this_node));
    //++++ memory access
    mem_read_ptrptr_vcn(&this_node, start_address_ptrptr_vcn, N_address);

  }

  //(LIL_nodes[ i ][ vec_size - 1 ]).next = NULL;
  start_address_ptrptr_vcn = (struct val_col_node **) (address_generator_int( 0, 0, offsetof(val_col_node, next) / 4, (int *) &LIL_nodes[ i ][ vec_size - 1]));
  //++++ memory access
  temp_vcn = NULL;
  mem_write_ptrptr_vcn(start_address_ptrptr_vcn, &temp_vcn, N_address);

  *rowMax = *rowMax + 1;
}


void Append_a_row_LIL_old(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int valSpr[], int indexSpr[], int vec_size, int* rowMax){

  int k = 0;

    for(k = 0; k < vec_size; k++)
      add_node(row_list, LIL_nodes, *(rowMax), indexSpr[ k ], valSpr[ k ]);

    *rowMax = *rowMax + 1;
}

void Append_a_row_LIL_protected_0_old(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int valSpr[], int indexSpr[], int vec_size, int* rowMax){

  int k = 0;

  for(k = 0; k < vec_size; k++)
    add_node_p0(row_list, LIL_nodes, *(rowMax), indexSpr[ k ], valSpr[ k ]);

  *rowMax = *rowMax + 1;
}
//====================== APPEND A COLUMN ========================//
void Append_a_col_DENSE_protected_0(int c[ ROWS ][ COLS ], int rowMax, int * colMax, int col_vec[]){
  int i, N_address = 1, temp;
  int* start_address_int;

  //--- Add the vector to the sparse formats
  start_spr_addr[ Nspr ] = &(col_vec[0]);  end_spr_addr[ Nspr ] = &(col_vec[ ROWS - 1 ]);
  Nspr++;
  
  //---- update the matrix  
  //memcpy(&(c[*rowMax][ 0 ]), vec_row, COLS * sizeof (int));
  for ( i = 0; i < rowMax; i++){
    //    col_vec[ i ] = a[ i ][ colNum ];  
    //++++ address generator
    start_address_int = address_generator_int(0, COLS, i, col_vec);
    //++++ memory access
    mem_read_int( &temp, start_address_int, N_address);
    //++++ address generator
    start_address_int = address_generator_int(i, COLS, *colMax, &(c[0][0]));
    //++++ memory access
    mem_write_int( start_address_int, &temp, N_address);
    }
  *colMax = *colMax + 1;

  Nspr--;
  return;
}



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
    val_new[ j ] = valSpr[ k ];
    col_new[ j ] = *colMax;
    j++;
    rowPtr[ i ] += k;
    i++;
    /*    printf("k: %d\n",k);
    printVecI(val_new,4);
    printVecI(col_new,4);
    printVecI(rowPtr, 11);    */
  }

  //--- update the rest of the rowPtr and vec and col
  int remaining = matrix_size - vecSize - rowPtr[ i ];
  remaining = max(remaining , 0);
  memcpy(&( val_new[ j ] ), &( val[ i_start + nz ]), remaining * sizeof(int) );
  memcpy(&( col_new[ j ] ), &( col[ i_start + nz ]), remaining * sizeof(int) );

  while (i <= rowMax){
    rowPtr[ i ] += vecSize;
    i++;
  }

  //--- update the return values
  memcpy(val, val_new, matrix_size * sizeof (int));
  memcpy(col, col_new, matrix_size * sizeof (int));

  //printf("j %d, matrix_size %d\n",j, matrix_size);

  * colMax = *colMax + 1;

  return;
}

void Append_a_col_CRS_protected_0(int val[], int col[], int rowPtr[], int rowMax, int * colMax, int valSpr[], int indexSpr[], int vecSize){
  int i = 0, i_start, i_end = 0, nz = 0, k, j = 0;
  int matrix_size, rowPtr_i, N_address;
  int *start_address_int;

  if (vecSize <= 0){
    * colMax = *colMax + 1;
    return;
  }

  //----- 1. Find the size of the matrix:
  //++++ address generator
  start_address_int = address_generator_int(0, 0, rowMax, &(rowPtr[0]));
  N_address = 1;
  //++++ memory access
  mem_read_int( &matrix_size, start_address_int, N_address);

  //printf("matrix size: %d\n", matrix_size);
  
  //++++functional
  matrix_size += vecSize;

  //printf("matrix size: %d\n", matrix_size);


  //--- 2. Initialize new val and col
  int matrix_size_pr = min( VEC_MAX, max(matrix_size, 0));
  int col_new[ matrix_size_pr ];
  int val_new[ matrix_size_pr ];
  //printf("matrix size_pr: %d\n", matrix_size_pr);

  //--- 2. Add them to the sparse domain
  start_spr_addr[ Nspr ] = &(val_new[0]);  end_spr_addr[ Nspr ] = &(val_new[ matrix_size_pr - 1 ]);
  start_spr_addr[ Nspr + 1 ] = &(col_new[0]);  end_spr_addr[ Nspr + 1 ] = &(col_new[ matrix_size_pr - 1 ]);
  Nspr +=2;

  for ( k = 0; k < vecSize; k++){
    if (k >= COLS)//---- protection 0
      break;
    //printf("k:%d\n",k);
    //---- 3 update rowPtr until a row with new nonzero value
    i_start = i_end;
    while (indexSpr[ k ] > i){
      if (i >= ROWS)//----- protection 0
	break;

      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, &(rowPtr[0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &rowPtr_i, start_address_int, N_address);

      //+++ functional
      rowPtr_i += k;

      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, &(rowPtr[0]));
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &rowPtr_i, N_address);

      //++++ functional
      i++;
    }
    //printf("i:%d\n",i);
    //--- 4. copy the current part of the row
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + 1, &(rowPtr[0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &i_end, start_address_int, N_address);

    //++++ functional
    nz = i_end - i_start;
    int protected_nz = min( max(0 , nz), VEC_MAX);//----reconsider

    int temp[protected_nz];
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i_start, val);
    N_address = protected_nz;
    //++++ memory access
    mem_read_int( temp, start_address_int, N_address);

    //++++ address generator
    start_address_int = address_generator_int(0, 0, j, &(val_new[0]));
    N_address = protected_nz;
    //++++ memory access
    mem_write_int( start_address_int, &(temp[0]), N_address);
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i_start, col);
    N_address = protected_nz;
    //++++ memory access
    mem_read_int( temp, start_address_int, N_address);

    //++++ address generator
    start_address_int = address_generator_int(0, 0, j, col_new);
    N_address = protected_nz;
    //++++ memory access
    mem_write_int( start_address_int, temp, N_address);
    
    //++++ functional
    j += nz;

    //--- 5. add the new value
    //    val_new[ j ] = valSpr[ k ];
    //col_new[ j ] = *colMax;
    //j++;
    //rowPtr[ i ] += k;
    //i++;
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, j, val_new);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(valSpr[ k ]), N_address);

    //++++ address generator
    start_address_int = address_generator_int(0, 0, j, col_new);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, colMax, N_address);
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, rowPtr);
    N_address = 1;
    //++++ memory access
    mem_read_int( &rowPtr_i, start_address_int, N_address);

    //++++ fuctional
    rowPtr_i += k;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, rowPtr);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &rowPtr_i, N_address);

    //++++ functional
    j++;
    i++;
/*    printf("k: %d\n",k);
    printVecI(val_new,4);
    printVecI(col_new,4);
    printVecI(rowPtr, 11);    */
  }

  //--- 6. update the rest of the rowPtr and vec and col
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i, rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &rowPtr_i, start_address_int, N_address);

  //++++ functional
  int remaining = matrix_size - vecSize - rowPtr_i;
  remaining = min(max(remaining , 0), VEC_MAX);//----- protection 0

  //  memcpy(&( val_new[ j ] ), &( val[ i_start + nz ]), remaining * sizeof(int) );
  //memcpy(&( col_new[ j ] ), &( col[ i_start + nz ]), remaining * sizeof(int) );
  int temp2[ remaining ];
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start + nz, val);
  N_address = remaining;
  //++++ memory access
  mem_read_int( temp2, start_address_int, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, j, val_new);
  N_address = remaining;
  //++++ memory access
  mem_write_int( start_address_int, temp2, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start + nz, col);
  N_address = remaining;
  //++++ memory access
  mem_read_int( temp2, start_address_int, N_address);

  //++++ address generator
  start_address_int = address_generator_int(0, 0, j, col_new);
  N_address = remaining;
  //++++ memory access
  mem_write_int( start_address_int, temp2, N_address);

  while (i <= rowMax){
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, rowPtr);
    N_address = 1;
    //++++ memory access
    mem_read_int( &rowPtr_i, start_address_int, N_address);

    //++++ functional
    rowPtr_i += vecSize;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, rowPtr);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &rowPtr_i, N_address);

    //++++ functional
    i++;
  }

  //--- update the return values
  //  memcpy(val, val_new, matrix_size * sizeof (int));
  //memcpy(col, col_new, matrix_size * sizeof (int));

  int temp3 [ matrix_size_pr ];
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val_new);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_read_int( temp3, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_write_int( start_address_int, temp3, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col_new);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_read_int( temp3, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_write_int( start_address_int, temp3, N_address);
  
  * colMax = *colMax + 1;

  Nspr -= 2;
  return;
}


void Append_a_col_SLL_protected_0(int val[], int col[], int row[], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize){
  int N_address;
  int * start_address_int;

  //++++ functional
  if (vecSize <= 0){
    * colMax = *colMax + 1;
    return;
  }

  //---- 1. Find the size of the matrix 
  int matrix_size = 0;
  int row_i, temp;

  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row);
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i, start_address_int, N_address);

  //  while (row[matrix_size] < rowMax && matrix_size < VEC_MAX){
  while (row_i < rowMax && matrix_size < VEC_MAX){//---- protection 0     
    //++++ address generator
    start_address_int = address_generator_int(0, 0, matrix_size, row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);

    //++++ functional
    matrix_size++;
  }

  //++++ functional
  matrix_size += vecSize;
  int matrix_size_pr = max( min(VEC_MAX, matrix_size), 0);//----- potection 0
 
  //--- 2. Initialize new val and col
  int val_new[ matrix_size_pr ];
  int col_new[ matrix_size_pr ];
  int row_new[ matrix_size_pr + 1];

  //---2.1. Add them to the sparse addresses list
  start_spr_addr[ Nspr ] = &( val_new[0] );  end_spr_addr[ Nspr ] = &( val_new[ matrix_size_pr - 1 ]);
  start_spr_addr[ Nspr + 1 ] = &( col_new[0] );  end_spr_addr[ Nspr + 1 ] = &( col_new[ matrix_size_pr - 1 ]);
  start_spr_addr[ Nspr + 2 ] = &( row_new[0] );  end_spr_addr[ Nspr + 2 ] = &( row_new[ matrix_size_pr ]);
  Nspr += 3;

  int i = 0, k;
  for ( k = 0; k < vecSize ; k++){
    //printf("k:%d\n",k);
    
    //--- 3. update row, val, col until the next nonzero value in the vector
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
    
    while (indexSpr[ k ] >= row_i){
      //while (indexSpr[ k ] >= row[ i ]){
      //row_new[ i + k ] = row[ i ];
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, row);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + k, row_new);
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &temp ,N_address);
      
      //      val_new[ i + k ] = val[ i ]; 
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, val);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + k, val_new);
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &temp ,N_address);
            
      //col_new[ i + k ] = col[ i ]; 
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, col);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + k, col_new);
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &temp ,N_address);
            
      //----- Read the next row_i
      //++++ functional
      i++;
      
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, row);
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_i, start_address_int, N_address);
      }

    //--- 4. Add the new element
    //row_new[ i + k ] = indexSpr[ k ]; 
    //val_new[ i + k ] = valSpr[ k ]; 
    //col_new[ i + k ] = *colMax; 
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + k, row_new);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(indexSpr[ k ]) ,N_address);
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + k, val_new);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(valSpr[ k ]) ,N_address);
      
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + k, col_new);
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, colMax ,N_address);
  }

  //--- 5. Add the remaining
  int remaining = matrix_size_pr - vecSize - i;
  remaining = min (max(0, remaining) , VEC_MAX);//----- protection 0
  //printf("remaining: %d, i:%d, vecSize: %d\n", remaining, i, vecSize);
   
  int temp2[ matrix_size_pr ];
  // memcpy(&(val_new[ i + vecSize ]), &(val[ i ]), remaining * sizeof (int));
  //memcpy(&(col_new[ i + vecSize ]), &(col[ i ]), remaining * sizeof (int));
  //memcpy(&(row_new[ i + vecSize ]), &(row[ i ]), remaining * sizeof (int));
    //----- 5.1 complete val_new
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, val);
    N_address = remaining;
    //++++ memory access
    mem_read_int( temp2, start_address_int, N_address);
  
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + vecSize, val_new);
    N_address = remaining;
    //++++ memory access
    mem_write_int( start_address_int, temp2, N_address);
    
    //----- 5.2 complete col_new
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, col);
    N_address = remaining;
    //++++ memory access
    mem_read_int( temp2, start_address_int, N_address);
  
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + vecSize, col_new);
    N_address = remaining;
    //++++ memory access
    mem_write_int( start_address_int, temp2, N_address);

    //----- 5.3 complete row_new
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, row);
    N_address = remaining;
    //++++ memory access
    mem_read_int( temp2, start_address_int, N_address);
  
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + vecSize, row_new);
    N_address = remaining;
    //++++ memory access
    mem_write_int( start_address_int, temp2, N_address);
    
  //--- 6. Update the return values
    //  memcpy(val, val_new, (matrix_size_pr) * sizeof (int));
    //memcpy(col, col_new, (matrix_size_pr) * sizeof (int));
    //memcpy(row, row_new, (matrix_size_pr) * sizeof (int));
    //row[matrix_size_pr] = ROWS;

  //----- 6.1 Update the val
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val_new);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_read_int( temp2, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, val);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_write_int( start_address_int, temp2, N_address);

  //----- 6.2 Update the col
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col_new);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_read_int( temp2, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_write_int( start_address_int, temp2, N_address);
    
  //----- 6.3 Update the val
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row_new);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_read_int( temp2, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_write_int( start_address_int, temp2, N_address);
  
  temp = ROWS;
  //++++ address generator
  start_address_int = address_generator_int(0, 0, matrix_size_pr, row);
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &temp, N_address);

  //++++ functional
  * colMax = *colMax + 1;
  Nspr -=3;
  return;
}

void Append_a_col_SLL(int val[], int col[], int row[], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize){

  if (vecSize <= 0){
    * colMax = *colMax + 1;
    return;
  }

  //---- find the size of the matrix 
  int matrix_size = 0;
   while (row[matrix_size] < rowMax && matrix_size < VEC_MAX)
     matrix_size++;

   // printf("matrix_size:%d\n", matrix_size);
 
  //--- Initialize new val and col
  int val_new[ matrix_size + vecSize];
  int col_new[ matrix_size + vecSize];
  int row_new[ matrix_size + vecSize + 1];

  int i = 0, k;
  for ( k = 0; k < vecSize ; k++){
    
    //--- update row, val, col until the next nonzero value in the vector
    while (indexSpr[ k ] >= row[ i ]){
      row_new[ i + k ] = row[ i ]; 
      val_new[ i + k ] = val[ i ]; 
      col_new[ i + k ] = col[ i ]; 
      i++;
    }

    //--- add the new element
    row_new[ i + k ] = indexSpr[ k ]; 
    val_new[ i + k ] = valSpr[ k ]; 
    col_new[ i + k ] = *colMax; 
  }
  //--- add the remaining
  int remaining = matrix_size - i;
  remaining = max(0, remaining);
  //printf("remaining: %d, i:%d, vecSize: %d\n", remaining, i, vecSize);
   if (i < matrix_size){
   memcpy(&(val_new[ i + vecSize ]), &(val[ i ]), remaining * sizeof (int));
   memcpy(&(col_new[ i + vecSize ]), &(col[ i ]), remaining * sizeof (int));
   memcpy(&(row_new[ i + vecSize ]), &(row[ i ]), remaining * sizeof (int));
  }
  
  //--- update the return values
  memcpy(val, val_new, (matrix_size + vecSize) * sizeof (int));
  memcpy(col, col_new, (matrix_size + vecSize) * sizeof (int));
  memcpy(row, row_new, (matrix_size + vecSize) * sizeof (int));
  row[matrix_size + vecSize] = ROWS;

  * colMax = *colMax + 1;

  //  printVecI(val, matrix_size + vecSize);
  // printVecI(col, matrix_size + vecSize);
  //printVecI(row, matrix_size + vecSize);

  return;
}


void Append_a_col_COO_protected_0(int val_col_row[ NZmax + 1][ 3 ], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize){
  int N_address;
  int * start_address_int;

  //++++ functional
  if (vecSize <= 0){
    * colMax = *colMax + 1;
    return;
  }

  //---- 1. Find the size of the matrix 
  int matrix_size = 0;
  int row_i, temp[ 3 ], temp1;

  //++++ address generator
  start_address_int = address_generator_int(0, 3, 2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i, start_address_int, N_address);

  //  while (row[matrix_size] < rowMax && matrix_size < VEC_MAX){
  while (row_i < rowMax && matrix_size < VEC_MAX){//---- protection 0     
    //++++ address generator
    start_address_int = address_generator_int(matrix_size, 3, 2, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);

    //++++ functional
    matrix_size++;
  }

  //++++ functional
  matrix_size += vecSize;
  int matrix_size_pr = max( min(NZmax, matrix_size), 0);//----- potection 0
 
  //--- 2. Initialize new val and col
  //int val_new[ matrix_size_pr ];
  //int col_new[ matrix_size_pr ];
  //int row_new[ matrix_size_pr + 1];

  int val_col_row_new[ matrix_size_pr + 1][3];
  //---2.1. Add it to the sparse addresses list
  start_spr_addr[ Nspr ] = &( val_col_row_new[0][0] );  end_spr_addr[ Nspr ] = &( val_col_row_new[ matrix_size_pr][2]);
  Nspr ++;

  int i = 0, k;
  for ( k = 0; k < vecSize ; k++){
    //printf("k:%d\n",k);
    
    //--- 3. update row, val, col until the next nonzero value in the vector
    //++++ address generator
    start_address_int = address_generator_int(i, 3, 2, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
    
    while (indexSpr[ k ] >= row_i){
      //while (indexSpr[ k ] >= row[ i ]){
      //row_new[ i + k ] = row[ i ];
      //++++ address generator
      start_address_int = address_generator_int(i, 3, 0, &(val_col_row[0][0]));
      N_address = 3;
      //++++ memory access
      mem_read_int( temp, start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int(i + k, 3, 0, &(val_col_row_new[0][0]));
      N_address = 3;
      //++++ memory access
      mem_write_int( start_address_int, temp ,N_address);
      /*
      //      val_new[ i + k ] = val[ i ]; 
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, val);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + k, val_new);
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &temp ,N_address);
            
      //col_new[ i + k ] = col[ i ]; 
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i, col);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);

      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + k, col_new);
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &temp ,N_address);
      */    
      //----- Read the next row_i
      //++++ functional
      i++;
      
      //++++ address generator
      start_address_int = address_generator_int(i, 3, 2, &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &row_i, start_address_int, N_address);
      }

    //--- 4. Add the new element
    //row_new[ i + k ] = indexSpr[ k ]; 
    //val_new[ i + k ] = valSpr[ k ]; 
    //col_new[ i + k ] = *colMax; 
    
    //++++ address generator
    start_address_int = address_generator_int(i + k, 3, 2, &(val_col_row_new[0][0]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(indexSpr[ k ]) ,N_address);
    
    //++++ address generator
    start_address_int = address_generator_int(i + k, 3, 0, &(val_col_row_new[0][0]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(valSpr[ k ]) ,N_address);
      
    //++++ address generator
    start_address_int = address_generator_int(i + k, 3, 1, &(val_col_row_new[0][0]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, colMax ,N_address);
  }

  //--- 5. Add the remaining
  int remaining = matrix_size_pr - vecSize - i;
  remaining = min (max(0, remaining) , VEC_MAX);//----- protection 0
  //printf("remaining: %d, i:%d, vecSize: %d\n", remaining, i, vecSize);
   
  int temp2[ matrix_size_pr ][ 3 ];
  // memcpy(&(val_new[ i + vecSize ]), &(val[ i ]), remaining * sizeof (int));
  //memcpy(&(col_new[ i + vecSize ]), &(col[ i ]), remaining * sizeof (int));
  //memcpy(&(row_new[ i + vecSize ]), &(row[ i ]), remaining * sizeof (int));
  //----- 5.1 complete val_new
  //++++ address generator
  start_address_int = address_generator_int(i, 3, 0, &(val_col_row[0][0]));
  N_address = remaining * 3;
  //++++ memory access
  mem_read_int( &(temp2[0][0]), start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(i + vecSize,3,0,&(val_col_row_new[0][0]));
  N_address = remaining * 3;
  //++++ memory access
  mem_write_int( start_address_int, &(temp2[0][0]), N_address);
  /*
    //----- 5.2 complete col_new
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, col);
    N_address = remaining;
    //++++ memory access
    mem_read_int( temp2, start_address_int, N_address);
  
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + vecSize, col_new);
    N_address = remaining;
    //++++ memory access
    mem_write_int( start_address_int, temp2, N_address);

    //----- 5.3 complete row_new
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, row);
    N_address = remaining;
    //++++ memory access
    mem_read_int( temp2, start_address_int, N_address);
  
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i + vecSize, row_new);
    N_address = remaining;
    //++++ memory access
    mem_write_int( start_address_int, temp2, N_address);
  */
  //--- 6. Update the return values
    //  memcpy(val, val_new, (matrix_size_pr) * sizeof (int));
    //memcpy(col, col_new, (matrix_size_pr) * sizeof (int));
    //memcpy(row, row_new, (matrix_size_pr) * sizeof (int));
    //row[matrix_size_pr] = ROWS;

  //----- 6.1 Update the val
  //++++ address generator
  start_address_int = address_generator_int(0, 3, 0, &(val_col_row_new[0][0]));
  N_address = matrix_size_pr * 3;
  //++++ memory access
  mem_read_int( &(temp2[0][0]), start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 3, 0, &(val_col_row[0][0]));
  N_address = matrix_size_pr * 3;
  //++++ memory access
  mem_write_int( start_address_int, &(temp2[0][0]), N_address);
  /*
  //----- 6.2 Update the col
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col_new);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_read_int( temp2, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, col);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_write_int( start_address_int, temp2, N_address);
    
  //----- 6.3 Update the val
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row_new);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_read_int( temp2, start_address_int, N_address);
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, 0, row);
  N_address = matrix_size_pr;
  //++++ memory access
  mem_write_int( start_address_int, temp2, N_address);
  */
  temp1 = ROWS;
  //++++ address generator
  start_address_int = address_generator_int(matrix_size_pr,3,2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &temp1, N_address);

  //++++ functional
  * colMax = *colMax + 1;
  Nspr--;
  return;
}

void Append_a_col_COO(int val_col_row[ ROWS ][ 3 ], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize){

  //---- find the size of the matrix 
  int matrix_size = 0;
  while (val_col_row[matrix_size][ 2 ] != rowMax)
    matrix_size++;
 
  //--- Initialize new val and col
  int val_col_row_new[ matrix_size + vecSize ][ 3 ];

  int i = 0, k;
  for ( k = 0; k < vecSize ; k++){
    
    //--- update row, val, col until the next nonzero value in the vector
    while (indexSpr[ k ] >= val_col_row[ i ][ 2 ]){
      val_col_row_new[ i + k ][ 0 ] = val_col_row[ i ][ 0 ]; 
      val_col_row_new[ i + k ][ 1 ] = val_col_row[ i ][ 1 ]; 
      val_col_row_new[ i + k ][ 2 ] = val_col_row[ i ][ 2 ];
      i++;
    }

    //--- add the new element
    val_col_row_new[ i + k ][ 2 ] = indexSpr[ k ]; 
    val_col_row_new[ i + k ][ 0 ] = valSpr[ k ]; 
    val_col_row_new[ i + k ][ 1 ] = *colMax; 
  }

  //--- update the return values
  memcpy(val_col_row, val_col_row_new, ((matrix_size + vecSize) * 3 + 1 ) * sizeof (int));
  * colMax = *colMax + 1;

  return;
}

void Append_a_col_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int * colMax, int valSpr[], int indexSpr[], int vecSize){
  int i, k;
  int index_row_i_l, N_address;
  int * start_address_int;

  //+++++ functional
  if (vecSize ==0){
    *colMax = *colMax + 1;
    return;
  }

  for(k = 0; k < vecSize; k++){
    i = indexSpr[ k ];

    //---- 1.Find the row length    
    int l = 0;
    start_address_int = address_generator_int(i, NZrowMax + 1, l, &(index_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &index_row_i_l, start_address_int, N_address);
  
    while ( index_row_i_l < COLS ){
      l++;

      //++++ address generator
      start_address_int = address_generator_int(i, NZrowMax + 1, l, &(index_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &index_row_i_l, start_address_int, N_address);
    }

    //printf("l:%d\n",l);
    
    //----- 2. Add the new item
    start_address_int = address_generator_int(i, NZrowMax + 1, l, &(index_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, colMax, N_address);
  
    int temp = COLS;
    start_address_int = address_generator_int(i, NZrowMax + 1, l + 1, &(index_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &temp, N_address);
  
    start_address_int = address_generator_int(i, NZrowMax, l, &(val_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_write_int( start_address_int, &(valSpr[ k ]), N_address);
  

    //index_row[ i ][ l ] = * colMax;
    //index_row[ i ][ l + 1 ] = COLS;
    //val_row[ i ][ l ] = valSpr[ k ];

    /*
    printf("i:%d\n",i);
    printVecI(val_row[i],l + 1);
    printVecI(index_row[i], l + 1);
    */
  }
  *colMax = *colMax + 1;
  return;
}

void Append_a_col_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int * colMax, int valSpr[], int indexSpr[], int vecSize){
  int i, k;

  if (vecSize ==0){
    *colMax = *colMax + 1;
    return;
  }

  for(k = 0; k < vecSize; k++){
    i = indexSpr[ k ];
    
    int l = 0;
    while ( index_row[ i ][ l ] < COLS )
      l++;

    printf("l:%d\n",l);
    
    index_row[ i ][ l ] = * colMax;
    index_row[ i ][ l + 1 ] = COLS;
    val_row[ i ][ l ] = valSpr[ k ];

    printf("i:%d\n",i);
    printVecI(val_row[i],l + 1);
    printVecI(index_row[i], l + 1);
  
  }
  *colMax = *colMax + 1;
  return;
}

void Append_a_col_LIL_protected_0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize){

  int k = 0, i;
  for( k = 0; k < vecSize; k++){
    if (k >= VEC_MAX)//---- protection 0
      break;

    i = indexSpr[ k ];
    add_node_p0(row_list, LIL_nodes, i, *colMax, valSpr[ k ]);
  }

  *colMax = *colMax + 1;
  
  return;
}


void Append_a_col_LIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize){

  int k = 0, i;
  for( k = 0; k < vecSize; k++){
    i = indexSpr[ k ];
    add_node(row_list, LIL_nodes, i, *colMax, valSpr[ k ]);
  }

  *colMax = *colMax + 1;
  
  return;
}

//================== REMOVE A ROW ====================//
void Remove_a_row_DENSE_protected_0(int a[ ROWS ][ COLS ], int rowNum, int * rowMax){
  int N_address = COLS, temp[ COLS ], i;
  int* start_address_int;

  for (i = rowNum; i < *rowMax - 1; i++){
    //++++ address generator
    start_address_int = address_generator_int(i + 1, COLS, 0,&(a[0][0]));
    //++++ memory access
    mem_read_int( temp, start_address_int, N_address);
    //++++ address generator
    start_address_int = address_generator_int(i, COLS, 0, &(a[0][0]));
    //++++ memory access
    mem_write_int( start_address_int, temp, N_address);
  }
  *rowMax = *rowMax - 1;
}

void Remove_a_row_CRS_protected_0(int val[], int col[], int rowPtr[], int rowNum, int * rowMax){
  int i, N_address;
  int *start_address_int;  
  int temp[ VEC_MAX ];

  //---- Find the size of the matrix
  int size_matrix ;//= rowPtr[ (*rowMax) ];
  //++++ address generator
  start_address_int = address_generator_int(0, 0, *rowMax, rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &size_matrix, start_address_int, N_address);
  
  //----- Find the size of the row
  int i_start;// = rowPtr[ rowNum ];
  //++++ address generator
  start_address_int = address_generator_int(0, 0, rowNum, rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &i_start, start_address_int, N_address);
   
  int i_end; //= rowPtr[ rowNum + 1];
  //++++ address generator
  start_address_int = address_generator_int(0, 0, rowNum + 1, rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &i_end, start_address_int, N_address);
  
  //++++ functional
  int row_size = i_end - i_start;

  //---- update val  
  //memcpy( &(val[ i_start ]), &(val[ i_end ]), (size_matrix - i_start) * sizeof (int));
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_end, val);
  N_address = max(min( size_matrix - i_start, VEC_MAX), 0);
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start, val);
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);
    
  //---- Update col
  //memcpy( &(col[ i_start ]), &(col[ i_end ]), (size_matrix - i_start) * sizeof (int));
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_end, col);
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start, col);
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);

  //----- Update rowPtr
  //++++ address generator
  start_address_int = address_generator_int(0, 0, rowNum + 1, rowPtr);
  N_address = min(max(*rowMax - rowNum, 0), VEC_MAX);
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  
  //++++ functional
  for (i = 0; i < N_address; i++)
    temp[i] = temp[i] - row_size;  

  //++++ address generator
  start_address_int = address_generator_int(0, 0, rowNum, rowPtr);
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);


  //for (i = rowNum; i < (*rowMax); i++)
  //rowPtr[i] = rowPtr[ i + 1] - row_size;
  //++++ functional
  *rowMax = *rowMax - 1;
}

void Remove_a_row_CRS(int val[], int col[], int rowPtr[], int rowNum, int * rowMax){
  int i;  

  //---- find the size of the matrix and the row
  int size_matrix = rowPtr[ (*rowMax) ];

  int i_start = rowPtr[ rowNum ]; 
  int i_end = rowPtr[ rowNum + 1];
  int row_size = i_end - i_start;

  //---- update the matrix  
  memcpy( &(val[ i_start ]), &(val[ i_end ]), (size_matrix - i_start) * sizeof (int));
  memcpy( &(col[ i_start ]), &(col[ i_end ]), (size_matrix - i_start) * sizeof (int));
  for (i = rowNum; i < (*rowMax); i++)
    rowPtr[i] = rowPtr[ i + 1] - row_size;
  
  *rowMax = *rowMax - 1;
}

/*
void Append_a_col_LIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int *colMax, int valSpr[], int indexSpr[], int vecSize){

  int k = 0, i;
  for( k = 0; k < vecSize; k++){
    i = indexSpr[ k ];
    add_node(row_list, LIL_nodes, i, *colMax, valSpr[ k ]);
  }

  *colMax = *colMax + 1;
  
  return;
}
*/

/*
//++++ address generator
  start_address_int = address_generator_int(0, 0, *rowMax, rowPtr);
  N_address = 1;
  //++++ memory access
  mem_read_int( &size_matrix, start_address_int, N_address);
*/
void Remove_a_row_SLL_protected_0(int val[], int col[], int row[], int rowNum, int *rowMax){

  int i = 0, i_start, i_end;
  int N_address, row_i;
  int * start_address_int;
  int temp[ VEC_MAX ];

  //---- find the row boundries
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i, row);
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i, start_address_int, N_address);

  while ( row_i != rowNum){
    i++;
    if (i > VEC_MAX)//---- protection 0
      break;
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }
  
  i_start = i;//--including

  while ( row_i == rowNum){
    i++;
    if (i > VEC_MAX)//---- protection 0
      break;
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }

  i_end = i;//--- excluding

  //---- find the size of the matrix
  int size_matrix;
  while ( row_i < *rowMax){
    i++;
    if (i > VEC_MAX)//---- protection 0
      break;
    
    //++++ address generator
    start_address_int = address_generator_int(0, 0, i, row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }

  size_matrix = i;

 
  //---- update the matrix  
  int row_size = i_end - i_start;
  
  //---- update val  
  //memcpy( &(val[ i_start ]), &(val[ i_end ]), (size_matrix - i_start) * sizeof (int));
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_end, val);
  N_address = max(min( size_matrix - i_start, VEC_MAX), 0);
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start, val);
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);
    
  //---- Update col
  //memcpy( &(col[ i_start ]), &(col[ i_end ]), (size_matrix - i_start) * sizeof (int));
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_end, col);
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start, col);
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);

  //----- Update rowPtr
  //  int k;
  //for (k = 0; k < size_matrix - i_start; k++)
  //row[ i_start + k ] = row[ i_end + k ] - 1 ;
  
  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_end, row);
  //++++ memory access
  mem_read_int( temp, start_address_int, N_address);
  
  //++++ functional
  for (i = 0; i < N_address; i++)
    temp[i] = temp[i] - 1;  

  //++++ address generator
  start_address_int = address_generator_int(0, 0, i_start, row);
  //++++ memory access
  mem_write_int( start_address_int, temp, N_address);

  *rowMax = *rowMax - 1;
}
  

void Remove_a_row_SLL(int val[], int col[], int row[], int rowNum, int *rowMax){

  int i_start, i_end;

  //---- find the row boundries
  int i = 0;
  while ( row[ i ] != rowNum)
    i++;
  
  i_start = i;//--including

 while ( row[ i ] == rowNum)
    i++;
 i_end = i;//--- excluding

  //---- find the size of the matrix
  int size_matrix;
  while ( row[ i ] != *rowMax)
    i++;
  size_matrix = i;

 
  //---- update the matrix  
  int row_size = i_end - i_start;
  memcpy( &(val[ i_start ]), &(val[ i_end ]), (size_matrix - i_start) * sizeof (int));
  memcpy( &(col[ i_start ]), &(col[ i_end ]), (size_matrix - i_start) * sizeof (int));
  
  int k;
  for (k = 0; k < size_matrix - i_start; k++)
    row[ i_start + k ] = row[ i_end + k ] - 1 ;
  
  *rowMax = *rowMax - 1;
}

void Remove_a_row_COO_protected_0(int val_col_row[ NZmax + 1][3], int rowNum, int *rowMax){

  int i = 0, i_start, i_end, k;
  int N_address, row_i;
  int * start_address_int;
  int temp[ 3 ];

  //---- find the row boundries
  //++++ address generator
  start_address_int = address_generator_int(i, 3, 2, &val_col_row[0][0]);
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i, start_address_int, N_address);

  while ( row_i != rowNum){
    i++;
    if (i > VEC_MAX)//---- protection 0
      break;
    
    //++++ address generator
    start_address_int = address_generator_int(i, 3, 2, &val_col_row[0][0]);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }
  
  i_start = i;//--including

  while ( row_i == rowNum){
    i++;
    if (i > VEC_MAX)//---- protection 0
      break;
    
    //++++ address generator
    start_address_int = address_generator_int(i, 3, 2, &val_col_row[0][0]);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }

  i_end = i;//--- excluding

  //---- find the size of the matrix
  int size_matrix;
  while ( row_i < *rowMax){
    i++;
    if (i > VEC_MAX)//---- protection 0
      break;
    
    //++++ address generator
    start_address_int = address_generator_int(i, 3, 2, &val_col_row[0][0]);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }

  size_matrix = i;

 
  //---- update the matrix  
  int row_size = i_end - i_start;
  int copy_size_pr = max(min( size_matrix - i_start, VEC_MAX), 0);
  
  //---- update val  
  N_address = 3;
  for (k = 0; k < copy_size_pr; k++){
  //memcpy( &(val[ i_start ]), &(val[ i_end ]), (size_matrix - i_start) * sizeof (int));
  //memcpy( &(col[ i_start ]), &(col[ i_end ]), (size_matrix - i_start) * sizeof (int));
  //for (k = 0; k < size_matrix - i_start; k++)
  //row[ i_start + k ] = row[ i_end + k ] - 1 ;
  //++++ address generator
    start_address_int = address_generator_int(i_end + k, 3, 0, &val_col_row[0][0]);
    //++++ memory access
    mem_read_int( temp, start_address_int, N_address);
    //++++ functional
    temp[2] -= 1;  
    //++++ address generator
    start_address_int = address_generator_int(i_start + k, 3, 0, &val_col_row[0][0]);
    //++++ memory access
    mem_write_int( start_address_int, temp, N_address);
  }
  
  *rowMax = *rowMax - 1;
}
void Remove_a_row_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1], int rowNum, int * rowMax, int colMax){
  int i, j;
  for(i = rowNum; i < *rowMax - 1; i++){
  //---- update each row > rowNum  
    j = 0;
    while (index_row[ i + 1 ][ j ] < colMax){
      val_row[ i ][ j ] = val_row[ i + 1 ][ j ];
      index_row[ i ][ j ] = index_row[ i + 1 ][ j ];
      j++;
    }
    index_row[ i ][ j ] = colMax;
  }
  //memcpy(&(val_row[rowNum]), &(val_row[rowNum + 1]), (( *rowMax ) - rowNum - 1) * NZrowMax* sizeof (int));
  //memcpy(&(index_row[rowNum]), &(index_row[rowNum + 1]), (( *rowMax ) - rowNum - 1) * (NZrowMax + 1)* sizeof (int));
  *rowMax = *rowMax - 1;
}

/*
    start_address_int = address_generator_int(i_end + k, 3, 0, &val_col_row[0][0]);
    //++++ memory access
    mem_read_int( temp, start_address_int, N_address);
    //++++ functional
    temp[2] -= 1;  
    //++++ address generator
    start_address_int = address_generator_int(i_start + k, 3, 0, &val_col_row[0][0]);
    //++++ memory access
    mem_write_int( start_address_int, temp, N_address);
*/

void Remove_a_row_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1], int rowNum, int * rowMax, int colMax){
  int i, j, temp, N_address = 1, index_row_ij;
  int * start_address_int;

  for(i = rowNum; i < *rowMax - 1; i++){
    j = 0;

    //---- update each row > rowNum  
    //++++ address generator
    start_address_int = address_generator_int(i + 1, NZrowMax + 1, j, &index_row[0][0]);
    //++++ memory access
    mem_read_int( &index_row_ij, start_address_int, N_address);

    while (index_row_ij < colMax){
      
      //val_row[ i ][ j ] = val_row[ i + 1 ][ j ];
      //++++ address generator
      start_address_int = address_generator_int(i + 1, NZrowMax, j, &val_row[0][0]);
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(i, NZrowMax, j, &val_row[0][0]);
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);


      //index_row[ i ][ j ] = index_row[ i + 1 ][ j ];
      //++++ address generator
      start_address_int = address_generator_int(i + 1, NZrowMax + 1, j, &index_row[0][0]);
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);
      
      //++++ functional
      j++;
      if (j >= VEC_MAX)//----- protection 0
	break;

      //++++ address generator
      start_address_int = address_generator_int(i + 1, NZrowMax + 1, j, &index_row[0][0]);
      //++++ memory access
      mem_read_int( &index_row_ij, start_address_int, N_address);
    }
    //index_row[ i ][ j ] = colMax;
    //++++ address generator
    start_address_int = address_generator_int(i, NZrowMax + 1, j, &index_row[0][0]);
    //++++ memory access
    mem_write_int( start_address_int, &colMax, N_address);     
  }
  *rowMax = *rowMax - 1;
}

void Remove_a_row_LIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax ], int rowNum, int * rowMax){

  memcpy(&(row_list[rowNum]), &(row_list[rowNum + 1]), ((*rowMax) - rowNum - 1) * sizeof (struct val_col_node *));
  *rowMax = *rowMax - 1; 
}

void Remove_a_row_LIL_protected_0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax ], int rowNum, int * rowMax){
  int N_address;
  val_col_node **start_address_ptrptr_vcn;
  val_col_node *temp[ ROWS ];

  //  memcpy(&(row_list[rowNum]), &(row_list[rowNum + 1]), ((*rowMax) - rowNum - 1) * sizeof (struct val_col_node *));
  //++++ address generator
  start_address_ptrptr_vcn = address_generator_ptr_vcn( rowNum + 1 , row_list);
  N_address = max(min((*rowMax) - rowNum - 1, ROWS), 0);
  //N_address = (*rowMax) - rowNum - 1;
  //++++ memory access
  mem_read_ptrptr_vcn( &temp[0], start_address_ptrptr_vcn, N_address);
  //++++ address generator
  start_address_ptrptr_vcn = address_generator_ptr_vcn( rowNum , row_list);
  //++++ memory access
  mem_write_ptrptr_vcn( start_address_ptrptr_vcn, &temp[0], N_address);
  
  *rowMax = *rowMax - 1; 
}


//====================== REMOVE A COLUMN ========================//
void Remove_a_col_DENSE_protected_0(int a[ ROWS ][ COLS ], int rowMax, int colNum, int * colMax){
  int N_address, temp[ ROWS ], i;
  int* start_address_int;

  for (i = 0; i < rowMax; i++){
    //++++ address generator
    start_address_int = address_generator_int(i, COLS, colNum + 1, &(a[0][0]));
    N_address = *colMax - colNum -1;
    //++++ memory access
    mem_read_int( temp, start_address_int, N_address);
    //++++ address generator
    start_address_int = address_generator_int(i, COLS, colNum, &(a[0][0]));
    //++++ memory access
    mem_write_int( start_address_int, temp, N_address);
  }

  *colMax = *colMax - 1;
}


/*
void Remove_a_col_DENSE_protected_0(int a[ ROWS ][ COLS ], int colNum, int * colMax){
  int N_address = 1, temp = 0, j;
  int* start_address_int;

  //---- update the matrix  
  for (j = 0; j < COLS; j++){
    //++++ address generator
    start_address_int = address_generator_int(rowNum, COLS, j, &(a[0][0]));
    //++++ memory access
    mem_write_int( start_address_int, &temp, N_address);
  }
  *rowMax = *rowMax - 1;
}
*/
void Remove_a_col_CRS_protected_0(int val[], int col[], int rowPtr[], int rowMax, int colNum, int * colMax){
  int i = 0, i_start, i_next = 0, k, j = 0, r = 0;
  int N_address, col_k, temp;
  int *start_address_int;

  for (i = 0; i < rowMax; i++){

      //---- find the row range
      i_start = i_next;
      //i_next = rowPtr[ i + 1 ];
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + 1, rowPtr);
      N_address = 1;
      //++++ memory access
      mem_read_int( &i_next, start_address_int, N_address);
   


      //--- find the element in col = colNum and row i
      int kp = 0;
      for (k = i_start; k < i_next; k++){
	
	kp++;
	if (kp > VEC_MAX)//---- protection 0
	  break;
	
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, col);
	N_address = 1;
	//++++ memory access
	mem_read_int( &col_k, start_address_int, N_address);
   
	if (col_k == colNum)
	  r++;	
	else if(col_k  > colNum){
	  //col[ j ] = col[ k ] - 1;
	  temp = col_k - 1;
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, j, col);
	  N_address = 1;
	  //++++ memory access
	  mem_write_int(start_address_int, &temp, N_address);
   
	  //val[ j ] = val[ k ];
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, val);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int(&temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, j, val);
	  //++++ memory access
	  mem_write_int(start_address_int, &temp, N_address);
   
	  j++;
	}
	else{
	  //col[ j ] = col[ k ];
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, j, col);
	  N_address = 1;
	  //++++ memory access
	  mem_write_int(start_address_int, &col_k, N_address);
   
	  //val[ j ] = val[ k ];
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, k, val);
	  N_address = 1;
	  //++++ memory access
	  mem_read_int(&temp, start_address_int, N_address);
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, j, val);
	  //++++ memory access
	  mem_write_int(start_address_int, &temp, N_address);
   	
	  j++;
	}
      
	//++++ address generator
	start_address_int = address_generator_int(0, 0, k, col);
	N_address = 1;
	//++++ memory access
	mem_read_int( &col_k, start_address_int, N_address);
      }

      //rowPtr[ i + 1 ] -= r; 
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + 1, rowPtr);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ functional
      temp -= r;
      //++++ address generator
      start_address_int = address_generator_int(0, 0, i + 1, rowPtr);
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);
  }

  //--- update the return values
  * colMax = *colMax - 1;
  return; 
}

void Remove_a_col_CRS(int val[], int col[], int rowPtr[], int rowMax, int colNum, int * colMax){
  int i = 0, i_start, i_next = 0, k, j = 0, r = 0;

  for (i = 0; i < rowMax; i++){

      //---- find the row range
      i_start = i_next;
      i_next = rowPtr[ i + 1 ];

      //--- find the element in col = colNum and row i
      for (k = i_start; k < i_next; k++){
	if (col[ k ] == colNum)
	  r++;	
	else if(col[ k ] > colNum){
	  col[ j ] = col[ k ] - 1;
	  val[ j ] = val[ k ];
	  j++;
	}
	else{
	  col[ j ] = col[ k ];
	  val[ j ] = val[ k ];
	  j++;
	}
      }

      rowPtr[ i + 1 ] -= r; 
  }

  //--- update the return values
  printf("r: %d\n",r);
  * colMax = *colMax - 1;
  return; 
}
  
void Remove_a_col_SLL(int val[], int col[], int row[], int rowMax, int colNum, int *colMax){

  //---- find the size of the matrix 
  int matrix_size = 0;
  while (row[matrix_size] != rowMax)
    matrix_size++;

  int i = 0, k, removed = 0;
  for ( k = 0; k < matrix_size ; k++){
    
    if( col[ k ] > colNum){
      //---- Shift next columns one to left
      col[ k - removed ] = col[ k ] - 1;
      val[ k - removed ] = val[ k ];
      row[ k - removed ] = row[ k ];
    }
    else if (col[ k ] < colNum){
      //---- Keep the lower columns unchanged 
      col[ k - removed ] = col[ k ];
      val[ k - removed ] = val[ k ];
      row[ k - removed ] = row[ k ];
    }
    else
      //----- remove the col = colNum
      removed ++;
  }

  row[ matrix_size - removed] = rowMax;
  * colMax = *colMax - 1;

  return;
}

void Remove_a_col_SLL_protected_0(int val[], int col[], int row[], int rowMax, int colNum, int *colMax){
  int N_address, row_i, col_k, temp;
  int *start_address_int;
  int matrix_size = 0;

  //---- find the size of the matrix 
  //++++ address generator
  start_address_int = address_generator_int(0, 0, matrix_size, row);
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i, start_address_int, N_address);
    
  while (row_i != rowMax){
    matrix_size++;
    if (matrix_size > VEC_MAX)//----- protection 0
      break;

    //++++ address generator
    start_address_int = address_generator_int(0, 0, matrix_size, row);
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }

  int i = 0, k, removed = 0;
  for ( k = 0; k < matrix_size ; k++){

    //++++ address generator
    start_address_int = address_generator_int(0, 0, k, col);
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);
      
    if( col_k > colNum){
      //---- Shift next columns one to left
      //      col[ k - removed ] = col[ k ] - 1;
      temp = col_k - 1;
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k - removed, col);
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);
      
      //      val[ k - removed ] = val[ k ];
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k, val);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k - removed, val);
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);

      // row[ k - removed ] = row[ k ];
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k, row);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k - removed, row);
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);

    }
    else if (col_k  < colNum){
      //---- Keep the lower columns unchanged 
      //col[ k - removed ] = col[ k ];
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k - removed, col);
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &col_k, N_address);
      
      //      val[ k - removed ] = val[ k ];
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k, val);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k - removed, val);
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);

      // row[ k - removed ] = row[ k ];
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k, row);
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(0, 0, k - removed, row);
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);
    }
    else
      //----- remove the col = colNum
      removed ++;

    //++++ address generator
    //start_address_int = address_generator_int(0, 0, k, col);
    //N_address = 1;
    //++++ memory access
    //mem_read_int( &col_k, start_address_int, N_address);
  }

  //row[ matrix_size - removed] = rowMax;
  //++++ address generator
  start_address_int = address_generator_int(0, 0, matrix_size - removed, row);
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &rowMax, N_address);

  //++++ fuctional
  * colMax = *colMax - 1;
  return;
}

void Remove_a_col_COO_protected_0(int val_col_row[ NZmax + 1 ][ 3 ], int rowMax, int colNum, int *colMax){
  int N_address, row_i, col_k, temp;
  int *start_address_int;
  int matrix_size = 0;
  int temp1[3];
  //---- find size of the matrix 
  //++++ address generator
  start_address_int = address_generator_int(matrix_size, 3, 2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_read_int( &row_i, start_address_int, N_address);
    
  while (row_i < rowMax){
    matrix_size++;

    if (matrix_size > VEC_MAX)//----- protection 0
      break;

    //++++ address generator
    start_address_int = address_generator_int(matrix_size, 3, 2, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &row_i, start_address_int, N_address);
  }

  int i = 0, k, removed = 0;
  for ( k = 0; k < matrix_size ; k++){

    //++++ address generator
    start_address_int = address_generator_int(k, 3, 1, &(val_col_row[0][0]));
    N_address = 1;
    //++++ memory access
    mem_read_int( &col_k, start_address_int, N_address);
      
    if( col_k > colNum){
      //---- Shift next columns one to left
      //      col[ k - removed ] = col[ k ] - 1;
      /*temp = col_k - 1;
      //++++ address generator
      start_address_int = address_generator_int(k - removed, 3, 1, &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);
      */
      //      val[ k - removed ] = val[ k ];
      // row[ k - removed ] = row[ k ];
      //++++ address generator
      start_address_int = address_generator_int( k, 3, 0, &(val_col_row[0][0]));
      N_address = 3;
      //++++ memory access
      mem_read_int( temp1, start_address_int, N_address);
      //++++ functional 
      temp1[1] -= 1;
      //++++ address generator
      start_address_int = address_generator_int(k - removed, 3, 0, &(val_col_row[0][0]));
      //++++ memory access
      mem_write_int( start_address_int, temp1, N_address);
      /*
      //++++ address generator
      start_address_int = address_generator_int(k, 3, 2, &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(k - removed, 3, 2, &(val_col_row[0][0]));
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);
      */
    }
    else if (col_k  < colNum){
      //---- Keep the lower columns unchanged 
      
//++++ address generator
      start_address_int = address_generator_int( k, 3, 0, &(val_col_row[0][0]));
      N_address = 3;
      //++++ memory access
      mem_read_int( temp1, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(k - removed, 3, 0, &(val_col_row[0][0]));
      //++++ memory access
      mem_write_int( start_address_int, temp1, N_address);
      
      /*
//col[ k - removed ] = col[ k ];
      //++++ address generator
      start_address_int = address_generator_int(k - removed, 3, 1, &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_write_int( start_address_int, &col_k, N_address);
      
      //      val[ k - removed ] = val[ k ];
      //++++ address generator
      start_address_int = address_generator_int(k, 3, 0, &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(k - removed,3, 0, &(val_col_row[0][0]));
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);

      // row[ k - removed ] = row[ k ];
      //++++ address generator
      start_address_int = address_generator_int(k, 3, 2, &(val_col_row[0][0]));
      N_address = 1;
      //++++ memory access
      mem_read_int( &temp, start_address_int, N_address);
      //++++ address generator
      start_address_int = address_generator_int(k - removed, 3, 2, &(val_col_row[0][0]));
      //++++ memory access
      mem_write_int( start_address_int, &temp, N_address);*/
    }
    else
      //----- remove the col = colNum
      removed ++;

    //++++ address generator
    //start_address_int = address_generator_int(k, 3, 1, &(val_col_row[0][0]));
    //N_address = 1;
    //++++ memory access
    //mem_read_int( &col_k, start_address_int, N_address);
  }

  //row[ matrix_size - removed] = rowMax;
  //++++ address generator
  start_address_int = address_generator_int(matrix_size - removed, 3, 2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &rowMax, N_address);

  //++++ fuctional
  * colMax = *colMax - 1;
  return;
}

void Remove_a_col_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colNum, int *colMax){
  int i, k, removed = 0;

  for(i = 0; i < rowMax; i++){
    k = 0;
    removed = 0;
    while (index_row[ i ][ k ] < *colMax){
      if (index_row[ i ][ k ] == colNum)//------ remove col = colNum
	removed++;
      else if (index_row[ i ][ k ] < colNum){// ------ keep first columns unchanged
	index_row[ i ][ k - removed ] = index_row[ i ][ k ];
	val_row[ i ][ k - removed ] = val_row[ i ][ k ];
      }
      else if (index_row[ i ][ k ] > colNum){//------- shift next columns one left
	index_row[ i ][ k - removed ] = index_row[ i ][ k ] - 1;
	val_row[ i ][ k - removed ] = val_row[ i ][ k ];
      }
      k++;
    }

    index_row[ i ][ k - removed ] = *colMax - 1;    
}

  *colMax = *colMax - 1;
  return;
}
/*
  //++++ address generator
  start_address_int = address_generator_int(matrix_size - removed, 3, 2, &(val_col_row[0][0]));
  N_address = 1;
  //++++ memory access
  mem_write_int( start_address_int, &rowMax, N_address);
*/

void Remove_a_col_ELLPACK_protected_0(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colNum, int *colMax){
  int i, k, removed = 0;
  int N_address = 1, index_row_ik, temp;
  int *start_address_int;

  for(i = 0; i < rowMax; i++){
    k = 0;
    removed = 0;

    //++++ address generator
    start_address_int = address_generator_int(i, NZrowMax + 1, k, &(index_row[0][0]));
    //++++ memory access
    mem_read_int( &index_row_ik, start_address_int, N_address);
    
    while (index_row_ik < *colMax){
      if (index_row_ik == colNum)//------ remove col = colNum
	removed++;
      else if (index_row_ik  < colNum){// ------ keep first columns unchanged
	//index_row[ i ][ k - removed ] = index_row[ i ][ k ];
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax + 1, k, &index_row[0][0]);
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax + 1, k - removed, &index_row[0][0]);
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);


	//val_row[ i ][ k - removed ] = val_row[ i ][ k ];
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax, k, &val_row[0][0]);
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax, k - removed, &val_row[0][0]);
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);
      }
      else if (index_row_ik  > colNum){//------- shift next columns one left

	//index_row[ i ][ k - removed ] = index_row[ i ][ k ] - 1;
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax + 1, k, &index_row[0][0]);
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);
	//++++ functional
	temp--;
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax + 1, k - removed, &index_row[0][0]);
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);


	//val_row[ i ][ k - removed ] = val_row[ i ][ k ];
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax, k, &val_row[0][0]);
	//++++ memory access
	mem_read_int( &temp, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(i, NZrowMax, k - removed, &val_row[0][0]);
	//++++ memory access
	mem_write_int( start_address_int, &temp, N_address);
      }

      k++;
      if (k >= VEC_MAX)//---- protection 0
	break;
      //++++ address generator
      start_address_int = address_generator_int(i, NZrowMax + 1, k, &(index_row[0][0]));
      //++++ memory access
      mem_read_int( &index_row_ik, start_address_int, N_address);
    }

    //index_row[ i ][ k - removed ] = *colMax - 1;
    temp = *colMax - 1;
    //++++ address generator
    start_address_int = address_generator_int(i, NZrowMax + 1, k - removed, &(index_row[0][0]));
    //++++ memory access
    mem_write_int( start_address_int, &temp ,N_address);
    
}

  *colMax = *colMax - 1;
  return;
}

void Remove_a_col_LIL_protected_0(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int rowMax, int colNum, int *colMax){

  int k = 0, i, N_address = 1;
  int this_node_val, this_node_col, temp;
  int *start_address_int;
  val_col_node * this_node, * removing_node, *this_node_next, *temp_vcn;
  val_col_node **start_address_ptrptr_vcn;

  for( i = 0; i < rowMax; i++){
    //---- remove node with j = colNum
    //    remove_node(row_list, LIL_nodes, i, colNum);
    removing_node = NULL;
    
    //this_node = row_list[i];
    //++++ address generator
    start_address_ptrptr_vcn = address_generator_ptr_vcn( i , row_list);
    //++++ memory access
    mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);

    if (this_node != NULL){
      //++++ address generator
      start_address_ptrptr_vcn =(struct val_col_node **) address_generator_int(0, 0, offsetof(val_col_node, next)/4 , (int *)this_node);
      //++++ memory access
      mem_read_ptrptr_vcn( &this_node_next, start_address_ptrptr_vcn, N_address);

      k = 0;
      while (this_node_next != NULL){
	k++;
	if(k > VEC_MAX)//---protection 0
	  break;
	
	//++++ address generator
	start_address_int = address_generator_int(0, 0, offsetof(val_col_node, col)/4 , (int *)this_node);
	//++++ memory access
	mem_read_int( &this_node_col, start_address_int, N_address);

	if (this_node_col > colNum){ //----- j = j - 1 for the columns after colNum
	  // this_node->col = this_node->col - 1;
	  temp = this_node_col - 1;
	  //++++ address generator
	  start_address_int = address_generator_int(0, 0, offsetof(val_col_node, col)/4 , (int *)this_node);
	  //++++ memory access
	  mem_write_int( start_address_int, &temp, N_address);
	}
	else if (this_node_col == colNum)//---- remove it
	  removing_node = this_node;

	// this_node = this_node->next;
	//++++ address generator
	start_address_ptrptr_vcn =(struct val_col_node **) address_generator_int(0, 0, offsetof(val_col_node, next)/4 , (int *)this_node);
	//++++ memory access
	mem_read_ptrptr_vcn( &this_node, start_address_ptrptr_vcn, N_address);

	//----- Update this_node_next
	//++++ address generator
	start_address_ptrptr_vcn =(struct val_col_node **) address_generator_int(0, 0, offsetof(val_col_node, next)/4 , (int *)this_node);
	//++++ memory access
	mem_read_ptrptr_vcn( &this_node_next, start_address_ptrptr_vcn, N_address);
      }
      
      //--- Replace it with the last item in the row
      if (removing_node != NULL){
	//removing_node->col = this_node->col;
	//++++ address generator
	start_address_int = address_generator_int(0, 0, offsetof(val_col_node, col)/4 , (int *)this_node);
	//++++ memory access
	mem_read_int( &this_node_col, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(0, 0, offsetof(val_col_node, col)/4 , (int *)removing_node);
	//++++ memory access
	mem_write_int( start_address_int, &this_node_col, N_address);

	//removing_node->val = this_node->val;
	//++++ address generator
	start_address_int = address_generator_int(0, 0, offsetof(val_col_node, val)/4 , (int *)this_node);
	//++++ memory access
	mem_read_int( &this_node_val, start_address_int, N_address);
	//++++ address generator
	start_address_int = address_generator_int(0, 0, offsetof(val_col_node, val)/4 , (int *)removing_node);
	//++++ memory access
	mem_write_int( start_address_int, &this_node_val, N_address);

	//--- remove the last item
	//this_node = NULL;
	temp_vcn = NULL;
	//++++ address generator
	start_address_ptrptr_vcn = &this_node;
	//++++ memory access
	mem_write_ptrptr_vcn( start_address_ptrptr_vcn, &temp_vcn, N_address);
      }
    }
  }
  *colMax = *colMax - 1;
  return;
}

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

/*
//++++ address generator
  start_address_ptrptr_vcn = address_generator_ptr_vcn( rowNum + 1 , row_list);
  //N_address = max(min((*rowMax) - rowNum - 1, ROWS), 0);
  N_address = (*rowMax) - rowNum - 1;
  //++++ memory access
  mem_read_ptrptr_vcn( &temp[0], start_address_ptrptr_vcn, N_address);
  //++++ address generator
  start_address_ptrptr_vcn = address_generator_ptr_vcn( rowNum , row_list);
  //++++ memory access
  mem_write_ptrptr_vcn( start_address_ptrptr_vcn, &temp[0], N_address);
  */
//====================== ADD AN ELEMENT TO MATRIX ==================//
void Add_element_CRS(float val[], int col[], int rowPtr[], int rowMax, float new_val, int new_i, int new_j){

  int i, k;
  int matrix_size = rowPtr[ rowMax ];
  
  //---- find the place of the element
  int i_start = rowPtr[ new_i ];
  int i_end = rowPtr[ new_i + 1 ];
 
  for (k = i_start; k < i_end; k++){
    if (col[ k ] == new_j){//--- just update the val[ k ]
      val[ k ] = new_val;
      return;
    }
    else if (col[ k ] > new_j){
      break;
    }
  }

  //---- shift the rest one element  
  memcpy(&(val[k + 1]), &(val[ k ]), (matrix_size - k + 1) * sizeof (float));
  memcpy(&(col[k + 1]), &(col[ k ]), (matrix_size - k + 1) * sizeof (int));
  
  //---- add the new element
  val[ k ] = new_val;
  col[ k ] = new_j;
  
  //---- update rowPtr
  for ( i = new_i + 1; i <= rowMax; i++)
    rowPtr[ i ] += 1;
 
  return;
}

void Add_element_SLL(int val[], int col[], int row[], int rowMax, int new_i, int new_j, int new_val){

  //---- find the size of the matrix 
  int matrix_size = 0;
  while (row[matrix_size] != rowMax)
    matrix_size++;

  //------ find the row = i
  int i = 0;
  while (row[ i ] < new_i)
    i++;

  //----- find col = j in row = i
  while (row[ i ] == new_i){
    if (col[ i ] = new_j){//---- just update the element 
      val[ i ] = new_val;
      return;
    }
    else if (col [ i ] > new_j)//---- add the element right before this
      break;      
    i++;
  }

  //---- add the element right before this
  //---- shift all one right  
  memcpy(&(val[ i + 1]), &(val[ i ]), (matrix_size - i) * sizeof (int));
  memcpy(&(col[ i + 1]), &(col[ i ]), (matrix_size - i) * sizeof (int));
  memcpy(&(row[ i + 1]), &(row[ i ]), (matrix_size - i) * sizeof (int));
  
  //---- add the new element
  val[ i ] = new_val;
  col[ i ] = new_j;

  return;
}    


void Add_element_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int new_i, int new_j, int new_val){

  //----- find col = new_j in row = new_i
  int j = 0;
  while ( index_row[ new_i ][ j ] != rowMax){
    if (j >= NZrowMax)//---- protection 0
      break;

    if (index_row[ new_i ][ j ] = new_j){//---- just update the element 
      val_row[ new_i ][ j ] = new_val;
      return;
    }
    else if (index_row [ new_i ][ j ] > new_j)//---- add the element right before this
      break;      
    j++;
  }

  //---- add the element right before this
  //---- shift all one right  
  memcpy(&(val_row[ new_i][ j + 1]), &(val_row[ new_i ][ j ]), (NZrowMax - j) * sizeof (int));
  memcpy(&(index_row[ new_i][ j + 1]), &(index_row[ new_i ][ j ]), (NZrowMax + 1 - j) * sizeof (int));
  
  //---- add the new element
  val_row[ new_i ][ j ] = new_val;
  index_row[ new_i ][ j ] = new_j;

  return;
}    

void Add_element_LIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int new_val, int new_i, int new_j){

  add_node(row_list, LIL_nodes, new_i, new_j, new_val);
  return;
}
//====================== SPRS TO DENSE ========================//
void Sprs_to_dense_CRS(int val[], int col[], int rowPtr[], int rowMax, int colMax, int **c){
  int i, i_start, i_next, j, k, val_i_j;
  int i_start_pr;
  int i_next_pr;
  //c should be set to zero
  for ( i = 0; i < rowMax ; i++){
    
    //------- 1. Find the index range for row i --------------//
    i_start = rowPtr[ i ];
    i_next = rowPtr[ i + 1 ];    
    
    i_start_pr = max( 0, i_start);//-- protection 0 for loop length
    i_next_pr = min( i_next, VEC_MAX);
    //------ 2. Read nonzero items in row i ----------//

    for (k = i_start_pr; k < i_next_pr; k++){
      
      if (0 <= k && k < VEC_MAX){
	j = col[ k ];
	val_i_j = val[ k ];

	if (0 <= j && j < colMax)
	  c[ i ][ j ] = val_i_j;
      }
    }
  }
  return;
}
void Sprs_to_dense_DENSE(int a[ ROWS ][ COLS ], int rowMax, int colMax, int **c){
  int i, j;
  for ( i = 0; i < rowMax ; i++){
    for (j = 0; j < colMax; j++){
      c[ i ][ j ] = a[ i ][ j ];
    }
  }
  return;
}

void Sprs_to_dense_SLL(int val[], int col[], int row[], int rowMax, int colMax, int **c){
  int k, i, j, val_i_j;

  for (k = 0; k < NZmax; k++){
    if (row[ k ] >= rowMax)//---- It shows the end of the matrix
      break;
    
    j = col [ k ];
    i = row [ k ];
    val_i_j = val [ k ];
    if ( 0 <= i && i < rowMax && 0 <= j && j < colMax)
      c[ i ][ j ] = val_i_j;

  }
  return;
}

void Sprs_to_dense_COO(int val_col_row[ NZmax + 1 ][ 3 ], int rowMax, int colMax, int **c){
  int k, i, j, val_i_j;

  for (k = 0; k < NZmax; k++){
    if (val_col_row[ k ][ 2 ] >= rowMax)//---- It shows the end of the matrix
      break;
    
    j = val_col_row [ k ][ 1 ];
    i = val_col_row [ k ][ 2 ];
    val_i_j = val_col_row [ k ][ 0 ];
    if ( 0 <= i && i < rowMax && 0 <= j && j < colMax)
      c[ i ][ j ] = val_i_j;

  }
  return;
}

void Sprs_to_dense_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int **c){
  int i, j, k, val_i_j;

  for ( i = 0; i < rowMax ; i++){
    for (k = 0; k < colMax; k++){
      
      if (index_row[ i ][ k ] >= colMax)//---- End of the row
	break;
      
      //--- find the items in row i
      j = index_row[ i ][ k ];
      val_i_j = val_row[ i ][ k ];
      if ( 0 <= i && i < rowMax && 0 <= j && j < colMax)
	c[ i ][ j ] = val_i_j;
	
    }
  }
}


void Sprs_to_dense_LIL(val_col_node *row_list [], int rowMax, int colMax, int **c){

  int i, j, val_i_j;
  int k = 0;
  val_col_node * this_node;

  for ( i = 0; i < rowMax ; i++){
    this_node = row_list[ i ];
    k = 0;
    
    while (this_node != NULL){
      k++;
      if (k > VEC_MAX)
	break;
    //--- find the items in row i
      //j = this_node->col;
      if (in_spr_bound((int*) (this_node + offsetof(val_col_node, col)/ 4)) )
	j =* ((int *)this_node + offsetof(val_col_node, col)/ 4);
      if (in_spr_bound((int*) (this_node + offsetof(val_col_node, val)/ 4)) )
	val_i_j =* ((int *)this_node + offsetof(val_col_node, val)/ 4);
      //val_i_j = this_node->val;

      if ( 0 <= i && i < rowMax && 0 <= j && j < colMax)
	c[ i ][ j ] = val_i_j;
           
      //this_node = this_node->next;
      if (in_spr_bound((int*) (this_node + offsetof(val_col_node, next)/ 4)) )
	this_node = *(val_col_node **)((int *)this_node + offsetof(val_col_node, next)/ 4);
      else
	this_node = NULL;
      }

  }
}

void Sprs_to_dense_JAD( int val[], int col[], int row_order[], int JADptr[], int rowMax, int colMax, int **c){
  int i, j, jc, i_true, val_i_j;
  int k_start, k_end, k;

  for (jc = 0; jc < NZcolMax; jc++){
    i = 0;
    //---- find JAD col jc boundry
    k_start = JADptr[ jc ];
    k_end = JADptr[ jc + 1];
    
    //---- read NZ values in JAD col jc
    for (k = k_start; k < k_end; k++){
      j = col[ k ];
      val_i_j = val[ k ];
      i_true = row_order[ i ];

      //----- right the c element
      c[ i_true ][ j ] = val_i_j;

      i++;
    }

  }
  return;
}

//================== JAD FUNCTIONS =================//
void permute_rows_JAD(int row_order[], int perm_order[]){
  int i, k, old_i, new_i;
  for(i = 0; i < ROWS; i++){
    old_i = row_order[ i ];
    for (k = 0; k < ROWS; k++){
      if (old_i == perm_order[ k ])
	break;
    }
    new_i = k;
    row_order[ i ] = k;
  }
  return;
}

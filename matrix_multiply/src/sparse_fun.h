void sort_vectors_by_other(int a[], int b[], int c[], int size_vec);
//================ METHODS =====================//
void add_node(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1], int row_id, int col_id, float in_data);
int * address_generator_int(int i, int row_length, int j, int * Addr);
val_col_node ** address_generator_ptr_vcn(int i,  val_col_node ** ptr_Addr);
//==================== READ INPUT ====================//
/*void processLine(int ** a, int rowMax, int colMax, FILE * stream);
void processLineCRS(int val[],int col[],int rowPtr[], int rowMax, int colMax, FILE * stream);
void processLineSLL(int val[],int col[],int row[], int rowMax, int colMax, FILE * stream);
void processLineCOO(int val_col_row[NZmax + 1][3], int rowMax, int colMax, FILE * stream);
void processLineELLPACK(int val_row[ ROWS ][ NZrowMax ],int index_row[ ROWS ][ NZrowMax + 1 ], int NZ_row[],int rowMax, int colMax, FILE * stream);
void processLineLIL(val_col_node *row_list [], val_col_node LIL_nodes[ ROWS ][ NZrowMax + 1 ], int NZ_row[], int rowMax, int colMax, FILE * stream);
//void processLineJAD( int JADptr[], int val[], int col[], int val_row[ ROWS ][ NZrowMax ], int index_row[ ROWS ][ NZrowMax + 1 ], JAD_node sorted_row[], int NZ_row[], int rowMax, int colMax, int row_order[], FILE * stream);
//====================== PICK A ROW ========================//
void Pick_a_row_CRS(int val[], int col[], int rowPtr[], int rowNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_row_SLL(int val[], int col[], int row[], int rowNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_row_SLL(int val[], int col[], int row[], int rowNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_row_COO(int val_col_row[ NZmax + 1 ][ 3 ], int rowNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_row_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int colMax, int rowNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_row_LIL(val_col_node *row_list [], int rowNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_row_JAD(int JADptr[], int val[], int col[], int rowNum, int valSpr[], int indexSpr[], int * NZsize);
//====================== PICK A COLUMN ========================//
void Pick_a_col_CRS(int val[], int col[], int rowPtr[], int rowMax, int colNum, int valSpr[], int indexSpr[], int *NZsize);
void Pick_a_col_SLL(int val[], int col[], int row[], int rowMax, int colNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_col_COO(int val_col_row[ NZmax + 1 ][ 3 ], int rowMax, int colNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_col_ELLPACK(int val_row[ ROWS ][ NZrowMax], int index_row[ ROWS ][ NZrowMax + 1 ], int rowMax, int colMax, int colNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_col_LIL(val_col_node *row_list [], int rowMax, int colMax, int colNum, int valSpr[], int indexSpr[], int * NZsize);
void Pick_a_col_JAD(int JADptr[], int val[], int col[], int rowMax, int colNum, int valSpr[], int indexSpr[], int * NZsize);
//===================== OP on SPARSE VECTORS ======================//
void spr_vectors_op (int a_valSpr[], int a_indexSpr[], int a_size, int b_valSpr[], int b_indexSpr[], int b_size, int c_valSpr[], int c_indexSpr[], int *c_size, char op);*/
//============== REDUCTION of SPARSE VECTOR ==================//
float spr_vector_reduction (int valSpr[], int indexSpr[], int vecNZ, int vecMax, char op);
float spr_vector_reduction_DENSE (int a[], int a_size, char op);
//================== APPEND A ROW ====================//
void Append_a_row_CRS(int val[], int col[], int rowPtr[], int valSpr[], int indexSpr[], int size_vec, int * rowMax);
//================= REMOVE A ROW ====================//
void Remove_a_row_CRS(float val[], int col[], int rowPtr[], int rowNum, int * rowMax);
//====================== APPEND A COLUMN ========================//
void Append_a_col_CRS(float val[], int col[], int rowPtr[], int rowMax, int * colMax, float valSpr[], int indexSpr[], int vecSize);
//====================== REMOVE A COLUMN ========================//
void Remove_a_col_CRS(float val[], int col[], int rowPtr[], int rowMax, int * colMax, int colNum);
//====================== UPDATE THE DIAGONAL ========================//
void Update_diag_CRS(float val[], int col[], int rowPtr[], int rowMax, float valSpr[], int indexSpr[], int vecSize);
//====================== ADD AN ELEMENT TO MATRIX ==================//
void Add_element_CRS(float val[], int col[], int rowPtr[], int rowMax, float new_val, int new_i, int new_j);

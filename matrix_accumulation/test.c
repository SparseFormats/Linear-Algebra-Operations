#include <stdlib.h>
#include <stdio.h>

void alaki(int ** a_ptr){
  int *a;
  a  = *a_ptr;
  a = a + 1;
  *a_ptr = a;
  printf("inside alaki:%d\n", *a);
}

int main(){

  int *c;
  int a[2];
  a[0] = 1;
  a[1] = 2;
  c = a;
  alaki(&c);
  printf("in main: %d\n", *c);

  return 0;
}

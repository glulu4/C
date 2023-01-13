#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void fillArr(int* arrPointer, int size);
void printArr(int* arr, int size );
int* bubbleSort(int* arr, int size );


int main(){

  int size = 30;
  int * array = (int*)(malloc(sizeof(int) * size));  // creates space in the heap for 10 ints
  printf("%s", "Unsorted Array: ");
  fillArr(array, size);
  printArr(array, size);


  printf("%s", "\nSorted   Array: ");
  array = bubbleSort( array, size);
  printArr(array, size);



  free(array);
  return 0;
}

int* bubbleSort(int* arr, int size ){
  int sorted = 0; // 0 is false, 1 is true

  while ( !sorted ){
    sorted = 1;
    for ( int i = 0; i < size-1; i++){
      if ( arr[i+1] < arr[i]){ // if next element is less than previous
        int temp = arr[i];
        arr[i] = arr[i+1];
        arr[i+1] = temp;
        sorted = 0;
      }
    }
  }
  return arr;
}

void fillArr(int* arrPointer, int size){

  srand ( time(NULL) ); // seeding rand(), rand() default seed is 1... 
  for ( int i = 0; i < size; i++ ){
    *(arrPointer + i ) = rand() % 100 ; // rand() % (max_number + 1 - minimum_number) + minimum_number
  }
}

void printArr(int* arr, int size ){
  for ( int i = 0; i < size; i++)
    printf("%d ", *(arr+i));
}

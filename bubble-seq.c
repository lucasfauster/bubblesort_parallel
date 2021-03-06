#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void bubbleSort(int array[], int size) {
  for (int step = 0; step < size - 1; ++step) {
    for (int i = 0; i < size - step - 1; ++i) {
      if (array[i] > array[i + 1]) {
        int temp = array[i];
        array[i] = array[i + 1];
        array[i + 1] = temp;
      }
    }
  }
}

int main() {
  int size;

  printf("size of the array: ");
	scanf("%i", &size);

	int * data = (int *)malloc(size*sizeof(int));

  for(int i = size; i > 0; i--)
			data[i] = i;

	double start = clock();

  bubbleSort(data, size);
 
  double stop = clock();

  for(int i=0;i<size;i++) 
      printf(" %d;",data[i]);

  printf("\n");
  printf("array size: %i; %f secs \n", size, (stop-start)/CLOCKS_PER_SEC);
}
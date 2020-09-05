#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include<time.h> 

/*
Simple program to simulate a fault with a 50% chance
*/

int main(int argc, char* argv[]){
	int i = 0;
	int *ptr1, *ptr2=NULL;
	for(i=0;i<10000000;i++);

	srand(time(0)); 
	ptr1 = &i;
	//printf("Pointer value before %p\n", ptr1);
	ptr2 = (int *) ((uintptr_t) ptr1 | (uintptr_t) 0xFFF000); //simulate a fault by changing a pointer
	//printf("Pointer value after %p\n", ptr2);
	if((rand()%2)==0)
		i = *ptr2;
	else
		i = *ptr1;

 	//printf("Intero %d\n", i);
}

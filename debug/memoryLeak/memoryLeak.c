#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int ARR_SIZE = 1000;

int main(void)
{
    // create an array of ARR_SIZE ints
    int *intArray = malloc(sizeof(int) * ARR_SIZE);

    // populate them
    for (int i=0; i<ARR_SIZE; i++) {
        intArray[i] = i;
    }

    // print one out
    // first, seed the random number generator
    srand(time(NULL));
    int randNum = rand() % ARR_SIZE;

    printf("intArray[%d]: %d\n", randNum, intArray[randNum]);

    // end without freeing!
    //free(intArray);
    return 0;
}

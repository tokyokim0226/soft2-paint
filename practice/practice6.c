#include <stdio.h>
#include <stdlib.h>

typedef struct point {
    int x;
    int y;
}Point;


int main()
{
    int *ptr = (int *)malloc(10 * sizeof(int));
    
    if (ptr == NULL) exit(1);
    
    printf("%lu\n", sizeof(ptr) / sizeof(ptr[0]));
    //when done like this then this will return the sizeof the address 
    //(the address taht ptr is pointing to instead of the array within the address)
    
    return 0;
}

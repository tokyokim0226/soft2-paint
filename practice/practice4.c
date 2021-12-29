// ヒープ領域に大規模なメモリを確保しようとした場合
#include <stdio.h>
#include <stdlib.h>
int main()
{
    char *a = malloc(sizeof(char)*100000000);
    if (a != NULL){
        printf("確保成功\n");
    }
    return 0;
}
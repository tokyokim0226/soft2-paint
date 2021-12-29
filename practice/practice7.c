// この例は一度ヒープを確保して値を代入、free後に、もう一度malloc しています
// どの領域が確保されるかは状況に依存しますが、
// 同じアドレスの場合は前の値が残っていることがわかります。
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // 一旦確保して散らかす
    int *ptr = (int*)malloc(10 * sizeof(int));
    if (ptr == NULL) exit(1);
    
    printf("%p\n", ptr);
    
    for(int i = 0 ; i < 10 ; i++){
        ptr[i] = i*i;
    }
    free(ptr);
    
    // ここからが本番
    ptr = (int*)malloc(10 * sizeof(int));
    if (ptr == NULL) exit(1);
    
    printf("%p\n", ptr);
    
    // 初期化せずに表示すると...
    for(int i = 0 ; i < 10 ; i++){
	printf("%d: %d\n", i, ptr[i]);
    }
}
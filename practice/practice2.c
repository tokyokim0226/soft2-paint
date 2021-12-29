// このコードは関数func()内でmallocを使い
// ヒープ領域に確保したメモリ領域のアドレスを返すため、
// 関数を抜けた後でもにアクセス可能: OK

#include <stdio.h>
#include <stdlib.h>

int *func(int n)
{
    int *a = (int*)malloc(n * sizeof(int));

    // 実際は a に正しく領域が確保されたか確認する必要があるが
    // ここでは一旦省略
    
    for (int i = 0 ; i < n ; i++){
	a[i] = i+1;
    }
    
    return a;

}

int main()
{
    int *b = func(3);
    
    printf("%d\n",b[0]);
    printf("%d\n",b[1]);
    printf("%d\n",b[2]);
    
    return 0;

}

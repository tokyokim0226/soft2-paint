// このコードは関数func()のスタック領域に確保したメモリ領域のアドレスを返すため、
// 関数を抜けた後にアクセスできない領域ということでNG

#include <stdio.h>
#include <stdlib.h>

int *func(int n)
{
    // 可変長配列: この関数内だけ有効
    int a[n];
    
    for (int i = 0 ; i < n ; i++){
	a[i] = i+1;
    }

    //function returns address of local variable
    
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
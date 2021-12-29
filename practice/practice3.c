#include <stdio.h>
#include <stdlib.h>

int main () {
// 確保したい型のサイズをsizeof演算子で取得し、個数をかけて確保
// 型のキャストは必須ではないがこのようにすることが多い
int *ptr = (int *)malloc(10 * sizeof(int));

// 確保に失敗していないかチェックする
if (ptr == NULL){
    exit(1); // return EXIT_FAILURE; の別の書き方: exit関数で返り値1 でプログラム終了
}

// 確保した領域は配列同様に使用できる
ptr[0] = 123;
ptr[1] = 551;
ptr[2] = 334;
ptr[3] = 114514;

for (int i = 0; i<4;i++) {
    printf("%d\n",ptr[i]);
}

// do something; ...
//（中略）

// 使い終わったら解放する
free(ptr);

return 0;
}
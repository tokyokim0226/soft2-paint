// 色々詰め込んだプログラム
/*
  - errno によるエラー情報の詳細
  - (-1) を size_t でキャストしたら....
  - malloc が失敗するとき
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> // エラー情報を取得するための標準ライブラリ
#include <string.h>  // strerror を使う
#include <stdint.h> // **SIZE_MAX のために追記**

int main()
{
    // errno.h で定義されるグローバル変数
    // エラーが起きた場合、エラーの種類に対応する値がセットされる
    errno = 0;
    // まずは -1 を size_t にキャストすると何がみえるでしょう
    printf("OUTPUT (-1): %zu\n",(size_t)-1);

    // SIZE_MAX という値も出力してみましょう
    printf("OUTPUT (SIZE_MAX): %zu\n",SIZE_MAX);
    
    // malloc は size_t を引数にとります。それ以外は型変換されます
    int *a = (int*)malloc(-1);

    if (a == NULL){

	// stdio.h に定義されたperrorを用いる場合
	perror("PERROR OUTPUT: ");

	// string.h に定義されたstrerrorを用いる場合

	fprintf(stderr,"ERROR at %s : %s\n", __func__, strerror(errno));

	exit (EXIT_FAILURE);
    }

    free(a);
    return 0;
}
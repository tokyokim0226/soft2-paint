#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
	char *a = "abc def ghi jkl";
	
	// 破壊されるのでコピーする
	char buf[20];
	strcpy(buf, a);
	
	// 最初にbuf がどうなっているのか整数でみてみる
	for (int i = 0 ; i < 20 ; i++){
		printf("%d\n", buf[i]);
	}
	
	// この時点での
	printf("%p\n", buf);
	
	// 初回呼び出しで区切りたい文字列を指定
	char *p;
	p = strtok(buf, " ");
	printf("%p: %s\n", p, p); // bufの先頭と一致する
	
	// 2回目以降はbufで最初に見つけた区切り文字のあとの有効文字列に
	// 関数内部でポインタが当たっており、NULL を指定することで
	// そこから読み出すことを関数に伝えている
	while ( (p = strtok(NULL, " ")) != NULL ){
		printf("%p: %s\n", p, p);
	}
	
	// 最後にbuf がどうなっているのか整数でみてみる
	for (int i = 0 ; i < 20 ; i++){
		printf("%d\n", buf[i]);
	}
	return 0;
}
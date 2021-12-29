#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h> // for error catch

// Structure for canvas
typedef struct
{
    int width;
    int height;
    char **canvas;
    char pen;
} Canvas;

// Command 構造体と History構造体
// [*]
typedef struct command Command;
struct command{
    char *str;
    size_t bufsize;
    Command *next;
};

typedef struct
{
    Command *begin;
    size_t bufsize; 
} History;

// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(Canvas *c);
void free_canvas(Canvas *c);

// display functions
void rewind_screen(unsigned int line); 
void clear_command(void);
void clear_screen(void);

// enum for interpret_command results
typedef enum res{ EXIT, LINE, RECT, CIRCLE, UNDO, SAVE, CHPEN, CHPENERR, UNKNOWN, LOADED, ERRNONINT, LOADERROR, ERRLACKARGS, NOCOMMAND} Result;

// Result 型に応じて出力するメッセージを返す
char *strresult(Result res);

int max(const int a, const int b);

void draw_dot(Canvas *c, const int x0, const int y0);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
void draw_rect(Canvas *c, const int x0, const int y0, const int width, const int height);
void draw_circle(Canvas *c, const int x0, const int y0, const int r);


Result interpret_command(const char *command, History *his, Canvas *c);


void save_history(const char *filename, History *his);
int load_history(const char *filename, History *his);

// [*] list.c のpush_backと同じ
Command *push_command(History *his, const char *str);

int main(int argc, char **argv)
{
    //for history recording
    
    const int bufsize = 1000;
    
    // [*]
    History his = (History){ .begin = NULL, .bufsize = bufsize};
    
    int width;
    int height;
    if (argc != 3){
        fprintf(stderr,"usage: %s <width> <height>\n",argv[0]);
        return EXIT_FAILURE;
    } 
    else{
        char *e;
        long w = strtol(argv[1],&e,10);
        if (*e != '\0'){
            fprintf(stderr, "%s: irregular character found %s\n", argv[1],e);
            return EXIT_FAILURE;
	    }
        long h = strtol(argv[2],&e,10);
        if (*e != '\0'){
            fprintf(stderr, "%s: irregular character found %s\n", argv[2],e);
            return EXIT_FAILURE;
        }
	width = (int)w;
	height = (int)h;    
    }

    char pen = '*';
    

    char buf[bufsize];

    Canvas *c = init_canvas(width,height, pen);
    
    printf("\n"); // required especially for windows env

    while(1){
	// [*]
	// hsize はひとまずなし
	// 作る場合はリスト長を調べる関数を作っておく
	print_canvas(c);
	printf("* > ");
	if(fgets(buf, bufsize, stdin) == NULL) break;
	
	const Result r = interpret_command(buf, &his, c);

	if (r == EXIT) break;

	// 返ってきた結果に応じてコマンド結果を表示
	clear_command();
	printf("%s\n",strresult(r));
	// LINEの場合はHistory構造体に入れる
	if (r == LINE || r == RECT || r == CIRCLE || r == CHPEN) {
	    // [*]
	    push_command(&his,buf);
	}
	
	rewind_screen(2); // command results
	clear_command(); // command itself
	rewind_screen(height+2); // rewind the screen to command input
	
    }
    
    clear_screen();
    free_canvas(c);
    
    return 0;
}

Canvas *init_canvas(int width,int height, char pen){
    Canvas *new = (Canvas *)malloc(sizeof(Canvas));
    new->width = width;
    new->height = height;
    new->canvas = (char **)malloc(width * sizeof(char *));
    
    char *tmp = (char *)malloc(width*height*sizeof(char));
    memset(tmp, ' ', width*height*sizeof(char));
    for (int i = 0 ; i < width ; i++){
	new->canvas[i] = tmp + i * height;
    }
    
    new->pen = pen;
    return new;
}

void reset_canvas(Canvas *c){
    c -> pen = '*'; // change pen back to default
    const int width = c->width;
    const int height = c->height;
    memset(c->canvas[0], ' ', width*height*sizeof(char));
}

void print_canvas(Canvas *c){
    const int height = c->height;
    const int width = c->width;
    char **canvas = c->canvas;
    
    // 上の壁
    printf("+");
    for (int x = 0 ; x < width ; x++)
	printf("-");
    printf("+\n");
    
    // 外壁と内側
    for (int y = 0 ; y < height ; y++) {
	printf("|");
	for (int x = 0 ; x < width; x++){
	    const char c = canvas[x][y];
	    putchar(c);
	}
	printf("|\n");
    }
    
    // 下の壁
    printf( "+");
    for (int x = 0 ; x < width ; x++)
	printf("-");
    printf("+\n");
    fflush(stdout);
}

void free_canvas(Canvas *c){
    free(c->canvas[0]); //  for 2-D array free
    free(c->canvas);
    free(c);
}

void rewind_screen(unsigned int line){
    printf("\e[%dA",line);
}

void clear_command(void){
    printf("\e[2K");
}

void clear_screen(void){
    printf( "\e[2J");
}


int max(const int a, const int b){
    return (a > b) ? a : b;
}

void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1){
    const int width = c->width;
    const int height = c->height;
    char pen = c->pen;
    
    const int n = max(abs(x1 - x0), abs(y1 - y0));
    if ( (x0 >= 0) && (x0 < width) && (y0 >= 0) && (y0 < height)){
	    c->canvas[x0][y0] = pen;
    }
    for (int i = 1; i <= n; i++) {
        const int x = x0 + i * (x1 - x0) / n;
        const int y = y0 + i * (y1 - y0) / n;
        if ( (x >= 0) && (x< width) && (y >= 0) && (y < height)){
            c->canvas[x][y] = pen;
        }
    }
}

void draw_dot(Canvas *c, const int x0, const int y0){
    if (x0 < 0 || x0 >= c->width || y0<0 || y0 >= c->height){
        return;
    }
    else {
        c -> canvas[x0][y0] = c->pen;
        return;
    }
}

void draw_rect(Canvas *c, const int x0, const int y0, const int width, const int height){
    
    //For the top and bottom sides of the rectangle
    for (int x = x0; x< x0+width; x++) {
        draw_dot(c, x, y0);
        draw_dot(c, x, y0+height-1);
    }

    //For the left and right sides of the rectangle
    for (int y = y0; y < y0+height; y++) {
        draw_dot(c, x0, y);
        draw_dot(c, x0+width-1, y);
    }
}

void draw_circle(Canvas *c, const int x0, const int y0, const int r){
    for (int x = x0 - r; x<=x0+r ;x ++){
        for (int y = y0 - r; y<=y0+r; y++) {
            double distance = sqrt(pow(x-x0, 2) + pow(y-y0,2));
            if (distance >= r && distance < r+1) {
                draw_dot(c,x,y);
            }
        }
    }
}

void save_history(const char *filename, History *his){
    const char *default_history_file = "history.txt";
    if (filename == NULL){
	    filename = default_history_file;
    }
    
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n", filename);
        return;
    }
    // [*] 線形リスト版
    for (Command *p = his->begin ; p != NULL ; p = p->next){
	    fprintf(fp, "%s", p->str);
    }
    
    fclose(fp);
}

int load_history(const char *filename, History *his) {
    const char *default_history_file = "history.txt";
    if (filename == NULL) {
        filename = default_history_file;
    }

    FILE *fp;
    if ((fp = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n", filename);
        return 1;
    }

    //delete all command history for so far
    for (Command *command = his -> begin; command != NULL; ) {
        Command *c = command -> next;
        free(command -> str);
        free(command);
        command = c;
    }

    his -> begin = NULL;
    
    Command *end = NULL;
    char *buf = (char*)malloc(his->bufsize * sizeof(char));
    while ((fgets(buf, his->bufsize, fp))!=NULL) {

        char *str = (char*)malloc(sizeof(char) * his->bufsize);
        strcpy(str, buf);

        Command *command = (Command*)malloc(sizeof(Command));
        command->str = str;
        command->bufsize = strlen(str) + 1;
        command->next = NULL;

        if (end == NULL) {
            his -> begin = command;
        }
        else {
            end -> next = command;
        }

        end = command;
    }

    return 0;
}

Result interpret_command(const char *command, History *his, Canvas *c){
    char buf[his->bufsize];
    strcpy(buf, command);
    buf[strlen(buf) - 1] = 0; // remove the newline character at the end
    
    const char *s = strtok(buf, " ");
    if (s == NULL){ // 改行だけ入力された場合
	    return UNKNOWN;
    }

    // The first token corresponds to command
    if (strcmp(s, "line") == 0) {
        int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: x1, p[3]: x1 
        char *b[4];
        for (int i = 0 ; i < 4; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 4 ; i++){
            char *e;
            //strtol works by converting the numbers first in a char* to long ints and then 
            //if there are any other characters that aren't numbers, store them in &e.
            //Last number is to denote the base of the numbers (10 --> base 10)
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }
        
        draw_line(c,p[0],p[1],p[2],p[3]);
        return LINE;
    }

    if (strcmp(s, "rect") == 0) {
        int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: width, p[3]: height
        char *b[4];
        for (int i = 0 ; i < 4; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 4 ; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }
        
        draw_rect(c,p[0],p[1],p[2],p[3]);
        return RECT;
    }

    if (strcmp(s, "circle") == 0) {
        int p[3] = {0}; // p[0]: x0, p[1]: y0, p[2]: r (radius)
        char *b[3];
        for (int i = 0 ; i < 3; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 3 ; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }
        
        draw_circle(c,p[0],p[1],p[2]);
        return CIRCLE;
    }

        if (strcmp(s, "chpen") == 0) {
        s = strtok(NULL, " ");
        if (s == NULL) {
            return CHPENERR;
        }

        c -> pen = s[0];
        return CHPEN;
    }
    
    if (strcmp(s, "save") == 0) {
        s = strtok(NULL, " ");
        save_history(s, his);
        return SAVE;
    }

    if (strcmp(s, "load") == 0) {
        s = strtok(NULL, " ");
        int loading = load_history(s, his);
        if (loading == 1) {
            return LOADERROR;
        }

        reset_canvas(c);
        for(Command *command = his -> begin; command !=NULL; command = command ->next) {
            interpret_command(command -> str, his, c);
        }

        clear_command();

        return LOADED;
    }
    
    if (strcmp(s, "undo") == 0) {
        reset_canvas(c);
        //[*] 線形リストの先頭からスキャンして逐次実行
        // pop_back のスキャン中にinterpret_command を絡めた感じ
        Command *p = his->begin;
        if (p == NULL){
            return NOCOMMAND;
        }
        else{
            Command *q = NULL; // 新たな終端を決める時に使う
            while (p->next != NULL){ // 終端でないコマンドは実行して良い
                interpret_command(p->str, his, c);
                q = p;
                p = p->next;
            }
            // 1つしかないコマンドのundoではリストの先頭を変更する
            if (q == NULL) {
                his->begin = NULL;
            }
            else{
                q->next = NULL;
            }
            free(p->str);
            free(p);	
            return UNDO;
        }  
    }
    
    if (strcmp(s, "quit") == 0) {
	    return EXIT;
    }

    return UNKNOWN;
}

// pushing the command to the end of the lienar list
Command *push_command(History *his, const char *str){
    Command *c = (Command*)malloc(sizeof(Command));
    char *s = (char*)malloc(his->bufsize);
    strcpy(s, str);
    
    *c = (Command){ .str = s, .bufsize = his->bufsize, .next = NULL};
    
    Command *p = his->begin;
    
    if ( p == NULL) {
	    his->begin = c;
    }
    else{
        while (p->next != NULL){
            p = p->next;
	}
	p->next = c;
    }
    return c;
}

char *strresult(Result res){
    switch(res) {
        case EXIT:
            break;
        case SAVE:
            return "history saved";
        case LINE:
            return "1 line drawn";
        case RECT:
            return "1 rectangle drawn";
        case CIRCLE:
            return "1 circle drawn";
        case UNDO:
            return "undo!";
        case CHPEN:
            return "Pen changed!";
        case CHPENERR:
            return "Error in changing pen";
        case UNKNOWN:
            return "error: unknown command";
        case LOADED:
            return "file successfully loaded";
        case LOADERROR:
            return "error: error loading";
        case ERRNONINT:
            return "Non-int value is included";
        case ERRLACKARGS:
            return "Too few arguments";
        case NOCOMMAND:
            return "No command in history";
    }
    return NULL;
}


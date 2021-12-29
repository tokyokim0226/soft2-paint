#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h> // for error catch
#include <assert.h>

typedef struct layer Layer;

 struct layer {
  int **board;
  int **color; 
  int **bgcolor; 
  int visible; 
  int clipped; 
  Layer *next;
  Layer *prev;
};

typedef struct {
    Layer *begin;
    size_t size;
}Layer_List;

typedef struct {
    int **board;
    int **color;
    int **bgcolor;
    int width;
    int height;
} Clipboard;

// Structure for canvas
typedef struct
{
    int width;
    int height;
    int width_default;
    int height_default;
    int aspect;
    char **canvas;
    char pen;
    char pen_size;
    char pen_default;
    int layer_index;
    int color; //--> color that is coded from between 0-255
    Layer_List *layer_list;

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
    size_t bufsize; //buffer length for command
    size_t size; // number of commands added
} History;

// functions for Layer type
Layer *get_layer(Canvas *c, int index);
Layer *get_cur_layer(Canvas *c);
Layer *get_last_layer(Canvas *c);
int reverse_layer(Canvas *c, int index, char *mode);
int hide_layer(Canvas *c, int index);
int show_layer(Canvas *c, int index);
int change_layer(Canvas *c, int index);

// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(Canvas *c);
void free_canvas(Canvas *c);

//2D array (matrix) functions to use for clipboard functions
int **make_2darray(int width, int height);
int **copy_2darray(int width, int height, int **array);
void free_2darray(int **array);

//Clipboard functions
int in_board(int x, int y, Canvas *c); // Checking if a certain point is within canvas boundaries
void copy_to_clipboard(Canvas *c, Clipboard *clip, int x0, int y0, int w, int h);
void paste_from_clipboard(Canvas *c, Clipboard *clip, int x0, int y0);
Clipboard *construct_clipboard();
void free_clipboard(Clipboard *clip);

// display functions
void rewind_screen(unsigned int line); 
void forward_screen(unsigned int line);
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
Command *get_last_command(History *his, const int actual);
void remove_commands(Command *command);
Command *construct_command(char *str);
void push_back_history(History *his, char *str);
Command *push_command(History *his, const char *str);

int main(int argc, char **argv)
{
    //for history recording
    
    const int bufsize = 200;
    
    // [*]
    History *his = (History*)malloc(sizeof(History));
    his -> bufsize = bufsize;
    
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
	printf("%zu > ", his -> size);
	if(fgets(buf, bufsize, stdin) == NULL) break;
	
	const Result r = interpret_command(buf, his, c);

	if (r == EXIT) break;

	// 返ってきた結果に応じてコマンド結果を表示
	clear_command();
	printf("%s\n",strresult(r));
	// LINEの場合はHistory構造体に入れる
	if (r == LINE || r == RECT || r == CIRCLE || r == CHPEN) {
	    // [*]
	    push_command(his,buf);
	}
	
	rewind_screen(2); // command results
	clear_command(); // command itself
	rewind_screen(height + 2); // rewind the screen to command input
	rewind_screen(18);
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
    
    printf("%s%75s", "Pen Color Reference", "Background Color Reference \n\n");
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int color = 16 * i + j; //print out each of the colors from 0 ~ 255
            printf("\e[38;5;%dm%3d\e[0m ", color, color);
        }
        for (int j=0; j<16; j++) {
            int color = i * 16 + j;
            printf("\e[48;5;%dm%3d\e[0m ", color, color);
        }
        printf("\n");
    }

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

void forward_screen(unsigned int line) {
  if (line <= 0) return;
  for (int i=0; i<line; i++) {
    printf("\n");
  }
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

    if (strcmp(s, "palette") == 0) {

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

Layer *get_layer(Canvas *c, int index) { //getting the layer of a particular index
    size_t size = c -> layer_list -> size;
    if (index >= size || index < 0) {
        return NULL;
    }

    Layer *layer = c -> layer_list -> begin;
    for (int i = 0; i<index; i++) {
        layer = layer -> next;
    }

    return layer;
}

Layer *get_cur_layer(Canvas *c) {
  return get_layer(c, c->layer_index);
}

Layer *get_last_layer(Canvas *c) {
  Layer *layer = c->layer_list->begin;

  assert(layer != NULL);

  while (layer->next != NULL) layer = layer->next;

  return layer;
}

int reverse_layer(Canvas *c, int index, char *mode) {

  size_t size = c->layer_list->size;
  if (index >= size) {
    printf("out of bounds!\n");
    return 1;
  }

  Layer *layer = get_layer(c, index);
  Clipboard *clip = construct_clipboard();
  int w = c->width;
  int h = c->height;
  copy_to_clipboard(c, clip, 0, 0, w, h);
  if (strcmp(mode, "vertical") == 0 || mode[0] == 'v') {
    for (int x=0; x<w; x++) {
      for (int y=0; y<h; y++) {
        layer->board[x][y] = clip->board[x][h-1-y];
        layer->color[x][y] = clip->color[x][h-1-y];
        layer->bgcolor[x][y] = clip->bgcolor[x][h-1-y];
      }
    }
  } else if (strcmp(mode, "horizontal") == 0 || mode[0] == 'h') {
    for (int x=0; x<w; x++) {
      for (int y=0; y<h; y++) {
        layer->board[x][y] = clip->board[w-1-x][y];
        layer->color[x][y] = clip->color[w-1-x][y];
        layer->bgcolor[x][y] = clip->bgcolor[w-1-x][y];
      }
    }
  } else if (strcmp(mode, "diagonal") == 0 || mode[0] == 'd') {
    if (w != h) {
      printf("Not square!\n");
      return 1;
    }

    for (int x=0; x<w; x++) {
      for (int y=0; y<h; y++) {
        int diff = x - y;
        int xsum = (w-1) + diff;
        int ysum = (h-1) - diff;
        layer->board[x][y] = clip->board[xsum-x][ysum-y];
        layer->color[x][y] = clip->color[xsum-x][ysum-y];
        layer->bgcolor[x][y] = clip->bgcolor[xsum-x][ysum-y];
      }
    }
  } else {
    printf("No such mode!\n");
    free_clipboard(clip);
    return 1;
  }

  free_clipboard(clip);
  return 0;
}

int hide_layer(Canvas *c, int index) {

  size_t size = c->layer_list->size;
  if (index >= size) {
    printf("out of bounds!\n");
    return 1;
  }

  get_layer(c, index)->visible = 0;
  return 0;
}

int show_layer(Canvas *c, int index) {
  size_t size = c->layer_list->size;
  if (index >= size || index < 0) {
    printf("out of bounds!\n");
    return 1;
  }

  get_layer(c, index)->visible = 1;
  return 0;
}

int change_layer(Canvas *c, int index) {
  size_t size = c->layer_list->size;
  if (index >= size || index < 0) {
    printf("out of bounds!\n");
    return 1;
  }

  c->layer_index = index;

  return 0;
}

int **make_2darray(int width, int height) {
    int *tmp = (int *)malloc(width * height * sizeof(int));
    memset(tmp, 0, width * height * sizeof(int));
    int **array_2D = malloc(width*sizeof(int*));
    for (int i = 0; i<width; i++) {
        array_2D[i] = tmp + i * height;
    }
    return array_2D;
}

int **copy_2Darray(int width, int height, int **array) {
    int **new_array = make_2darray(width, height);
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            new_array[x][y] = array[x][y];
        }
  }
  return new_array;
}

void free_2darray(int **array) {
  if (array == NULL) return;
  free(array[0]);
  free(array);
}

int in_board(int x, int y, Canvas *c) {

    if (x < 0 || x >= c->width || y < 0 || y >= c->height) return 0;

  return 1;
}

void copy_to_clipboard(Canvas *c, Clipboard *clip, int x0, int y0, int w, int h) {
    clip -> width = w;
    clip -> height = h;

    free_2darray(clip -> board);
    free_2darray(clip -> color);
    free_2darray(clip -> bgcolor);

    clip->board = make_2darray(w, h);
    clip->color = make_2darray(w, h);
    clip->bgcolor = make_2darray(w, h);

    Layer *layer = get_cur_layer(c);
    for (int i=0; i < w; i++) {
        for (int j=0; j < h; j++) { 
            if (!in_board(x0+i,y0+j,c)) continue;
            clip->board[i][j] = layer->board[x0 + i][y0 + j];
            clip->color[i][j] = layer->color[x0 + i][y0 + j];
            clip->bgcolor[i][j] = layer->bgcolor[x0 + i][y0 + j];
        }
    }
}

void paste_from_clipboard(Canvas *c, Clipboard *clip, int x0, int y0){
    Layer *layer = get_cur_layer(c);
    for (int i = 0; i < clip -> width; i++) {
        for (int j = 0; j < clip -> height; j++) {
            if (!in_board(x0 + i, y0 + j, c)) continue;
            if (clip->board[i][j] == 0) continue;
            layer->board[x0 + i][y0 + j] = clip->board[i][j];
            layer->color[x0 + i][y0 + j] = clip->color[i][j];
            if (clip->bgcolor[i][j] != 0)  {
                layer->bgcolor[x0 + i][y0 + j] = clip->bgcolor[i][j];
            }
        }
    }
}

Clipboard *construct_clipboard(){
    Clipboard *clipboard = (Clipboard*)malloc(sizeof(Clipboard));

    clipboard->width = 0;
    clipboard->height = 0;
    clipboard->board = NULL;
    clipboard->color = NULL;
    clipboard->bgcolor = NULL;

    return clipboard;
}

void free_clipboard(Clipboard *clip){
    if (clip == NULL) return;
    free_2darray(clip->board);
    free_2darray(clip->color);
    free_2darray(clip->bgcolor);
    free(clip);
}

Command *get_last_command(History *his, const int actual) {

    if (actual) {
        if (his->begin == NULL) {
            return NULL;
        } 
        else {
            Command *command = his->begin;
            while (command->next != NULL) {
            command = command->next;
            }
            return command;
        }

    } 
    
    else {
        if (his->size == 0) {
            return NULL;
        }
        else {
            Command *command = his->begin;
            for (int i=0; i < his->size-1; i++) {
                command = command->next;
            }
            return command;
        }

    }
}

void remove_commands(Command *command) {
    while (command != NULL) {
        Command *temp = command;
        command = command->next;

        free(temp->str);
        free(temp);
    }
}


Command *construct_command(char *str) {

    Command *command = (Command*)malloc(sizeof(Command));
    command->bufsize = strlen(str) + 1;
    command->next = NULL;
    command->str = (char*)malloc(command->bufsize);
    strcpy(command->str, str);

    return command;
}

void push_back_history(History *his, char *str) {
    Command *command = construct_command(str);

    Command *node = get_last_command(his, 0);
    if (node != NULL) {
        remove_commands(node->next); // undoで取り消されたコマンドをすべて削除
        node->next = command;
    } 
    else {
        remove_commands(his->begin);
        his->begin = command;
    }
}
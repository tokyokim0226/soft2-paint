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
  int **highlight; 
  int vis; 
  Layer *prev;
  Layer *next;
};

typedef struct {
    Layer *begin;
    size_t size;
}Layer_List;

typedef struct {
    int width;
    int height;
    int **board;
    int **color;
    int **highlight;
} Clipboard;

// Structure for canvas
typedef struct
{
    int width;
    int height;
    int width_default;
    int height_default;
    int ratio;
    char **canvas;
    char pen;
    char pen_size;
    char pen_default;
    int layer_idx;
    int color; //--> color that is coded from between 0-255
    Layer_List *layer_list;
    Clipboard *clipboard;
} Canvas;

// Command 構造体と History構造体
// [*]
typedef struct command Command;

struct command{
    char *str;
    size_t bufsize;
    Command *next;
};

typedef struct{
    Command *begin;
    size_t bufsize; //buffer length for command
    size_t size; // number of commands added
} History;


// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_character(char c, int color, int highlight, FILE *fp);
void print_canvas(FILE *fp, Canvas *c);
int resize_canvas(Canvas *c, int width, int height);
void free_canvas(Canvas *c);

//2D array (matrix) functions to use for clipboard functions
int **make_matrix(int width, int height);
int **copy_matrix(int width, int height, int **array);
void free_matrix(int **array);

//Clipboard functions
int check_if_in_board(int x, int y, Canvas *c); // Checking if a certain point is within canvas boundaries
void clip_to_clipboard(Canvas *c, Clipboard *clip, int x0, int y0, int w, int h);
void paste_from_clipboard(Canvas *c, Clipboard *clip, int x0, int y0);
Clipboard *make_clipboard();
void free_clipboard(Clipboard *clip);

// functions for Layer type
Layer *get_layer(Canvas *c, int idx);
Layer *current_layer(Canvas *c);
Layer *final_layer(Canvas *c);
void add_layer(Canvas *c);
int insert_layer(Canvas *c, int idx, Layer *layer);
int remove_layer(Canvas *c, int idx, int should_free);
int resize_layer(Canvas *c, int idx, int width, int height);
int mirror_layer(Canvas *c, int idx, char *mode);
int move_layer(Canvas *c, int a, int b);
void copy_board(int width, int height, int **board, int **color, int **highlight, Layer *layer);
void free_board(Layer *layer);
int switch_to_layer(Canvas *c, int idx);
int *idx_reader_for_layer(int *len);
int layer_idx(int default_value);
Layer *make_layer(int width, int height);
int copy_layer(Canvas *c, int idx);
int merge_layer(Canvas *c, int idx);
int show_layer(Canvas *c, int idx);
int hide_layer(Canvas *c, int idx);
void free_layer(Layer *layer);
void free_all_layers(Canvas *c);

// display functions
void rewind_screen(unsigned int line); 
void unwind_screen(unsigned int line);
void clear_single_line(void);
void clear_screen(void);

// enum for interpret_command results
typedef enum res{ EXIT, LINE, RECT, CIRCLE, UNDO, SAVE, CHPEN, CHPENERR, UNKNOWN, LOADED, ERRNONINT, LOADERROR, ERRLACKARGS,
CHPENSIZE,ERASER,MARKER,COLORCHANGED,PENSIZENOTINT,PENSIZETOOBIG,COLORNOTINT,COLORNOTINRANGE ,NOCOMMAND, FILL, 
ADDLAYER, CHANGELAYER, REMOVELAYER, INSERTLAYER, MOVELAYER, COPYLAYER, MERGELAYER, RESIZE, CHANGERATIO, 
COPIEDTOCLIPBOARD, PASTEDFROMCLIPBOARD ,MOVED, LAYEROUTOFBOUNDS, COPYLAYERERR, MERGELAYERERR, SHOWLAYER, HIDELAYER,
RATIOERR, CHOOSEREVERSEDIRECTION, REVERSINGERROR,NOTHING_TO_REDO_OR_UNDO, MIRROR, REDO, UNKNOWNLAYERERR} Result;

// Result 型に応じて出力するメッセージを返す
char *strresult(Result res);

int max(const int a, const int b);

void draw_point(Canvas *c, const int x0, const int y0);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
void draw_rect(Canvas *c, const int x0, const int y0, const int width, const int height, int fill);
void erase_rect(Canvas *c, const int x0, const int y0, const int width, const int height);
void draw_circle(Canvas *c, const int x0, const int y0, const int r, int fill);
void fill(Canvas *c, Layer *layer, int x0, int y0, int pen, int color, int highlight);


Result interpret_command(const char *command, History *his, Canvas *c);


void save(const char *filename, History *his);
int load(const char *filename, History *his);
Command *final_command(History *his, const int actual);
void remove_commands(Command *command);
Command *make_command(char *str);
void update_history(History *his, char *str); //new version of push_command from kadai 3

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
    
    
    FILE *fp;
    char buf[bufsize];
    fp = stdout;
    Canvas *c = init_canvas(width,height, pen);
    
    fprintf(fp, "\n"); 

    while(1){

	print_canvas(fp, c);
    printf("||Layer %d/%zu || ", c->layer_idx + 1, c->layer_list->size);
    if (c->pen == ' ') { // when it is marker
        printf("marker \e[48;5;%dm   \e[0m ||", c->color);
    } 
    else if (c->pen == 0) {
        printf("eraser ||");
    } 
    else {
      printf("pen: \e[38;5;%dm%c\e[0m", c->color, c->pen);
    }

    Command *last = final_command(his, 0);
    if (last != NULL) {
        printf(" %s", last->str);
    } 
    else {
        printf("\n");
    }

	printf("%zu > ", his -> size);
	if(fgets(buf, bufsize, stdin) == NULL) break;
	
	const Result r = interpret_command(buf, his, c);

	if (r == EXIT) break;

	// 返ってきた結果に応じてコマンド結果を表示
	clear_single_line();
	printf("%s\n",strresult(r));
	// LINEの場合はHistory構造体に入れる
	if (r == LINE || r == RECT || r == CIRCLE || r == CHPEN || r == CHPENSIZE || r == ERASER || r == MARKER || r == COLORCHANGED
    || r == SHOWLAYER || r == HIDELAYER  || r == CHPENSIZE || r == ERASER || r == MARKER || r == ADDLAYER || r == CHANGELAYER
    || r == REMOVELAYER || r == INSERTLAYER || r == MOVELAYER || r == COPYLAYER || r == MERGELAYER || r == RESIZE || r == CHANGERATIO || r == COPIEDTOCLIPBOARD
    || r == PASTEDFROMCLIPBOARD ||r == FILL|| r == MOVED || r == MIRROR) {
	    // [*]
	    update_history(his,buf);
	}

    if (r == UNDO || r == LOADED) {
        clear_screen();
        print_canvas(fp, c);
        clear_single_line();
        printf("||Layer %d/%zu || ", c->layer_idx + 1, c->layer_list->size);
        if (c->pen == ' ') { // when it is marker
            printf("marker \e[48;5;%dm   \e[0m ||", c->color);
        } 
        else if (c->pen == 0) {
            printf("eraser ||");
        } 
        else {
        printf("pen: \e[38;4;%dm%c\e[0m ||", c->color, c->pen);
        }

        Command *last = final_command(his, 0);
        if (last != NULL) {
            printf(" %s", last->str);
        } 
        else {
            printf("\n");
        }

        printf("%zu > ", his -> size);

        clear_single_line();
	    printf("\n%s\n",strresult(r));

    }
	
	rewind_screen(2); // command results
	clear_single_line(); // command itself
    rewind_screen(1);
    clear_single_line();
	rewind_screen(height + 2); // rewind the screen to command input
	rewind_screen(18);
    }
    
    clear_screen();
    free_canvas(c);
    fclose(fp);
    
    return 0;
}

Canvas *init_canvas(int width,int height, char pen){
    Canvas *new = (Canvas *)malloc(sizeof(Canvas));
    new->width = width;
    new->height = height;
    new->canvas = (char **)malloc(width * sizeof(char *));
    new->width_default = width;
    new->height_default = height;
    new->ratio = 1;
    new->layer_list = (Layer_List*)malloc(sizeof(Layer_List));
    new->layer_list->begin = make_layer(width, height);
    new->layer_list->size = 1;
    new->layer_idx = 0;
    new->pen = pen;
    new->pen_default = pen;
    new->pen_size = 1;
    new->color = 231;
    new->clipboard = make_clipboard();
    
    return new;
}

void reset_canvas(Canvas *c){
    c -> pen = '*'; // change pen back to default
    c -> width = c -> width_default;
    c -> height = c -> height_default;
    c->ratio = 1;
    c->pen = c->pen_default;
    c->pen_size = 1;
    c->color = 231;
    free_all_layers(c);
    c->layer_idx = 0;
    c->layer_list->begin = make_layer(c->width, c->height);
    c->layer_list->size = 1;
    c->clipboard = make_clipboard();

}

void print_character(char c, int color, int highlight, FILE *fp) {
    fprintf(fp, "\e[38;5;%dm", color);
    if (highlight != 0) {
        fprintf(fp, "\e[48;5;%dm", highlight);
    }
    fputc(c, fp);
    fprintf(fp, "\e[0m");

}

void print_canvas(FILE *fp, Canvas *c){

    const int height = c->height;
    const int width = c->width;
    int **board = current_layer(c)->board;
    
    fprintf(fp, "%s%75s", "Pen Color Reference", "Background Color Reference \n\n");
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int color = 16 * i + j; //print out each of the colors from 0 ~ 255
            fprintf(fp, "\e[38;5;%dm%3d\e[0m ", color, color);
        }
        for (int j=0; j<16; j++) {
            int color = i * 16 + j;
            fprintf(fp, "\e[48;5;%dm%3d\e[0m ", color, color);
        }
        fprintf(fp, "\n");
    }

    // 上の壁
    clear_single_line();
    fprintf(fp, "+");
    for (int x = 0 ; x < width*c->ratio ; x++) fprintf(fp, "-");
    fprintf(fp, "+\n");
    
    // 外壁と内側
    for (int y = 0 ; y < height ; y++) {
        clear_single_line();
        fprintf(fp, "|");
        for (int x = 0 ; x < width; x++){
            char ch = ' ';
            int color = 0;
            int highlight = 0;
            int is_current_layer = 0;
            for (int i=0; i<c->layer_list->size; i++) {
                Layer *layer = get_layer(c, i);
                if (!layer->vis || layer->board[x][y] == 0) {
                    continue;
                }

                ch = layer->board[x][y];
                color = layer->color[x][y];
                if (layer->highlight[x][y] != 0) {
                    highlight = layer->highlight[x][y];
                }
                is_current_layer = (i == c->layer_idx);
            }
            for (int i=0; i<c->ratio; i++) {
                print_character(ch, color, highlight, fp);
            }
        }
        fprintf(fp, "|\n");
    }
    
    // 下の壁
    fprintf(fp, "+");
    for (int x = 0 ; x < width*c->ratio ; x++)
	fprintf(fp, "-");
    fprintf(fp, "+\n");
    fflush(fp);
}

void free_canvas(Canvas *c){
    free(c->canvas[0]); //  for 2-D array free
    free(c->canvas);
    free(c);
}

void rewind_screen(unsigned int line){
    printf("\e[%dA",line);
}

void unwind_screen(unsigned int line) {
  if (line <= 0) return;
  for (int i=0; i<line; i++) {
    printf("\n");
  }
}

void clear_single_line(void){
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

    draw_point(c, x0, y0);

    for (int i = 1; i <= n; i++) {
        double x = (x0 + 0.5) + i * (x1 - x0) / (double)n;
        double y = (y0 + 0.5) + i * (y1 - y0) / (double)n;

        // use manhattan distance to paint areas where the length is below pen_size
        for (int j=0; j < c->pen_size; j++) {
            for (int k=0; k<=j; k++) {
                draw_point(c, (int)x + k, (int)y - j + k);
                draw_point(c, (int)x + k, (int)y + j - k);
                draw_point(c, (int)x - k, (int)y - j + k);
                draw_point(c, (int)x - k, (int)y + j - k);
            }
        }
        if (x == (int)x) {
        draw_point(c, x-1, y);
        }
        if (y == (int)y) {
        draw_point(c, x, y-1);
        }
    }
}

void draw_point(Canvas *c, const int x0, const int y0){
    if (x0 < 0 || x0 >= c->width || y0<0 || y0 >= c->height){
        return;
    }

    else {
        Layer *layer = current_layer(c);
        if (c -> pen == ' ') {//when it is a marker {
            layer -> board[x0][y0] = ' ';
            layer -> highlight[x0][y0] = c -> color;
        }
        else if (c -> pen == 0){ //Eraser mode
            layer -> board[x0][y0] = 0;
            layer -> color[x0][y0] = 0;
            layer -> highlight[x0][y0] = 0;
        }
        else { //normal pen mode - keep background color as it is set before swap (keep the highlighted areas highlighted)
            layer -> board[x0][y0] = c -> pen;
            layer -> color[x0][y0] = c -> color;
        }
        return;
    }
    return;
}

void draw_rect(Canvas *c, const int x0, const int y0, const int width, const int height, int fill){
    if (fill) {
        for (int x = x0; x< x0+width; x++) {
            for (int y = y0; y < y0+height; y++) {
                draw_point(c,x,y);
            }
        }
    }
    else {
        //For the top and bottom sides of the rectangle
        for (int x = x0; x< x0+width; x++) {
            for (int i=0; i < c->pen_size; i++) {
                draw_point(c, x, y0);
                draw_point(c, x, y0+height-1-i);
            }
        }

        //For the left and right sides of the rectangle
        for (int y = y0; y < y0+height; y++) {
            for (int i=0; i < c->pen_size; i++) {
                draw_point(c, x0, y);
                draw_point(c, x0+width-1-i, y);
            }
        }
    }
}

void erase_rect(Canvas *c, int x0, int y0, int width, int height) {
    int cur_pen = c->pen;
    c->pen = 0;
    draw_rect(c, x0, y0, width, height, 1);
    c->pen = cur_pen;
}

void draw_circle(Canvas *c, const int x0, const int y0, const int r, int fill){
    for (int x = x0 - r; x<=x0+r ;x ++){
        for (int y = y0 - r; y<=y0+r; y++) {
            double distance = sqrt(pow(x-x0, 2) + pow(y-y0,2));
            if (fill) {
                if (distance < r+1) {
                    draw_point(c,x,y);
                }
            }
            else {
                if (distance >= r - c -> pen_size && distance < r) {
                    draw_point(c,x,y);
                }
            }
        }
    }
}

void fill(Canvas *c, Layer *layer, int x0, int y0, int pen, int color, int highlight) {
    draw_point(c, x0, y0);
    for (int dx=-1; dx<=1; dx++) {
        for (int dy=-1; dy<=1; dy++) {

            int adj_x = x0 + dx;
            int adj_y = y0 + dy;
            if (!check_if_in_board(adj_x, adj_y, c)) continue;
            if ((dx != 0 && dy != 0)) continue;

            //use recursion to conduct the same for neighboring if not across a boundary
            if (layer->board[adj_x][adj_y] == pen && layer->color[adj_x][adj_y] == color && highlight == layer->highlight[adj_x][adj_y]) {
                fill(c, layer, adj_x, adj_y, pen, color, highlight);
            }

        }
    }

}

void save(const char *filename, History *his){
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
    Command *command = his->begin;
    for (int i=0; i < his->size; i++){
	    fprintf(fp, "%s", command ->str);
        command = command -> next;
    }
    
    fclose(fp);
}

int load(const char *filename, History *his) {
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
    remove_commands(his -> begin);

    his -> begin = NULL;
    his -> size = 0;
    
    char *buf = (char*)malloc(sizeof(char) * his->bufsize);

    while ((fgets(buf, his->bufsize, fp)) != NULL) {
        update_history(his, buf);
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
            //if there are aadj_y other characters that aren't numbers, store them in &e.
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
        int p[5] = {0}; // p[0]: x0, p[1]: y0, p[2]: width, p[3]: height
        char *b[5];
        for (int i = 0 ; i < 5; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 5 ; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }
        
        draw_rect(c,p[0],p[1],p[2],p[3],p[4]);
        return RECT;
    }

    if (strcmp(s, "circle") == 0) {
        int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: r (radius) p[3]: fill
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
        
        draw_circle(c,p[0],p[1],p[2],p[3]);
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

    if (strcmp(s, "pensize") == 0) {
        s = strtok(NULL, " ");
        if (s == NULL) {
            return ERRLACKARGS;
        }
        char *e;
        long v = strtol(s,&e, 10);
        if (*e != '\0'){
            return PENSIZENOTINT;
        }
        if (v < 1) {
            c -> pen_size = 1;
        }
        else if(v > 10) {
            return PENSIZETOOBIG;
        }
        else {
            c -> pen_size = v;
        }
        return CHPENSIZE;
    }

    if (strcmp(s, "eraser") == 0) {
        c -> pen = 0;
        return ERASER;
    }

    if (strcmp(s, "marker") == 0) {
        c -> pen = ' ';
        if (c -> color == 0) {
            c -> color = 232;
        }
        return MARKER;  
    }
    
    if (strcmp(s, "color") == 0) {
        s = strtok(NULL, " ");
        if (s == NULL) {
            return ERRLACKARGS;
        }
        char *e;
        long v = strtol(s,&e, 10);
        if (*e != '\0'){
            return COLORNOTINT;
        }
        if (v < 0 || v > 255) {
            return COLORNOTINRANGE;
        }
        else {
            c -> color = v;
        }
        return COLORCHANGED;
    }

    if (strcmp(s, "fill") == 0) {
        int p[2] = {0}; 
        char *b[2];
        for (int i = 0 ; i < 2; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 2 ; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }   

        Layer *layer = current_layer(c);
    
        fill(c, layer, p[0],p[1], layer -> board[p[0]][p[1]],layer -> color[p[0]][p[1]], layer -> highlight[p[0]][p[1]]);
        return FILL;   
    }

    if (strcmp(s, "save") == 0) {
        s = strtok(NULL, " ");
        save(s, his);
        return SAVE;
    }

    if (strcmp(s, "load") == 0) {
        s = strtok(NULL, " ");
        int loading = load(s, his);
        if (loading == 1) {
            return LOADERROR;
        }

        reset_canvas(c);
        for(Command *command = his -> begin; command !=NULL; command = command ->next) {
            interpret_command(command -> str, his, c);
            rewind_screen(1);
        }


        return LOADED;
    }
    if (strcmp(s, "redo") == 0) {
        Command *maybe_last = final_command(his,0);
        Command *real_last = final_command(his,1);

        if (maybe_last != real_last) {
            his -> size ++;
            if (maybe_last == NULL) {
                maybe_last = his -> begin;
            }
            else {
                maybe_last = maybe_last -> next;
            }
            interpret_command(maybe_last -> str, his, c);
        }
        else {
            return NOTHING_TO_REDO_OR_UNDO;
        }
        return REDO;
    }

    if (strcmp(s, "undo") == 0) {
        reset_canvas(c);

        if (his -> size == 0) {
            return NOTHING_TO_REDO_OR_UNDO;
        }

        else if (his -> size == 1) {
            his -> size --;
        }

        else{
            his -> size --;
            Command *com = his -> begin;

            // 最初から実行し直す
            for (int i=0; i< his->size; i++) {
                interpret_command(com->str, his, c);
                rewind_screen(1);

                com = com->next;
            }
            clear_single_line();
        } 
        return UNDO;
    }

    if (strcmp(s, "layer") == 0 || strcmp(s, "l") == 0) {
    
    s = strtok(NULL, " ");

        if (strcmp(s, "add") == 0) {
            add_layer(c);
            switch_to_layer(c, c->layer_list->size - 1);
            return ADDLAYER;
        }

        else if (strcmp (s, "change") == 0) {
            int idx = layer_idx(-1);
            if (idx == -1){
                return ERRLACKARGS;
            }
            int result = switch_to_layer(c,idx);
            if (result == 1) {
                return LAYEROUTOFBOUNDS;
            }
            return CHANGELAYER;
        }

        else if (strcmp(s, "remove") == 0) {
            int idx = layer_idx(c -> layer_idx);
            int result = remove_layer(c, idx, 1); //also free the layer
            if (result == 1) {
                return LAYEROUTOFBOUNDS;
            }
            return REMOVELAYER;
        }

        else if (strcmp(s, "insert") == 0) {
            int idx = layer_idx(-1);
            if (idx == -1) {
                return ERRLACKARGS;
            }

            int result = insert_layer(c, idx, make_layer(c -> width, c -> height));
            if (result == 1) {
                return LAYEROUTOFBOUNDS;
            }
            return INSERTLAYER;
        }

        else if (strcmp(s, "move") == 0) {
            int p[2] = {0}; // p[0]: idx of layer to move, p[1]: idx of location to move layer
            char *b[2];
            for (int i = 0 ; i < 2; i++){
                b[i] = strtok(NULL, " ");
                if (b[i] == NULL){
                    return ERRLACKARGS;
                }
            }
            for (int i = 0 ; i < 2 ; i++){
                char *e;
                long v = strtol(b[i],&e, 10);
                if (*e != '\0'){
                    return ERRNONINT;
                }
                p[i] = (int)v;
            }   
        
            move_layer(c, p[1],p[0]);
            return MOVELAYER;
        }

        else if (strcmp(s, "copy") == 0) {
            int idx = layer_idx(c -> layer_idx);
            int result = copy_layer(c, idx);
            if (result == 1) {
                return COPYLAYERERR;
            }
            switch_to_layer(c, c->layer_list->size-1);
            return COPYLAYER;
        }

        else if (strcmp(s, "merge") == 0) {
            int idx = layer_idx(c->layer_idx);
            int result = merge_layer(c,idx);
            if (result == 1) {
                return MERGELAYERERR;
            }
            return MERGELAYER;
        }

        else if (strcmp(s, "show") == 0) {
            int idx = layer_idx(c -> layer_idx);
            int result = 0;
            if (idx == -1) {
                for (int i=0; i<c->layer_list->size; i++) {
                    show_layer(c,i);
                }
            }
            else {
                result = show_layer(c, idx);
            }

            if (result == 1) {
                return LAYEROUTOFBOUNDS;
            }
            return SHOWLAYER;
        }

        else if (strcmp(s,"hide") == 0) {
            int idx = layer_idx(c -> layer_idx);
            int result = 0;
            if (idx == -1) {
                for (int i=0; i<c->layer_list->size; i++) {
                    hide_layer(c,i);
                }
            }
            else {
                result = hide_layer(c,idx);
            }
            if (result == 1) {
                return LAYEROUTOFBOUNDS;
            }
            return HIDELAYER;
        }

        else {
            return UNKNOWNLAYERERR;
        }
    }

    if (strcmp(s, "mirror") == 0) {
        char *direction = strtok(NULL, " ");

        if (direction == NULL) {
            return CHOOSEREVERSEDIRECTION;
        }

        int idx = layer_idx(c->layer_idx);
        int result = mirror_layer(c, idx, direction);
        if (result == 1) {
            return REVERSINGERROR;
        }

        return MIRROR;        

    }

    if (strcmp(s, "resize") == 0) {
        int p[2] = {0}; // resizing dimensions
        char *b[2];
        for (int i = 0 ; i < 2; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 2 ; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }   
    
        resize_canvas(c, p[0],p[1]);
        return RESIZE;
    }

    if (strcmp(s, "ratio") == 0) {
        int p[1] = {0}; // p[0]: ratio
        char *b[1];
        for (int i = 0 ; i < 1; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 1 ; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }   
        if (p[0] < 0) {
            return RATIOERR;
        }
        c -> ratio = p[0];
        return CHANGERATIO;
    }

    if (strcmp(s, "copy") == 0) {
        int p[4] = {0}; 
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
    
        clip_to_clipboard(c, c -> clipboard, p[0],p[1],p[2],p[3]);
        return COPIEDTOCLIPBOARD;
    }

    if (strcmp(s, "paste") == 0) {
        int p[2] = {0};
        char *b[2];
        for (int i = 0 ; i < 2; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 2; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }   
    
        paste_from_clipboard(c,c->clipboard, p[0],p[1]);
        return PASTEDFROMCLIPBOARD;
    }

    if (strcmp(s, "move") == 0) {
        int p[6] = {0}; // 
        char *b[6];
        for (int i = 0 ; i < 6; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL){
                return ERRLACKARGS;
            }
        }
        for (int i = 0 ; i < 6; i++){
            char *e;
            long v = strtol(b[i],&e, 10);
            if (*e != '\0'){
                return ERRNONINT;
            }
            p[i] = (int)v;
        }   

        Clipboard *temp = make_clipboard();
        clip_to_clipboard(c, temp, p[0], p[1], p[2], p[3]);
        erase_rect(c, p[0], p[1], p[2], p[3]);
        paste_from_clipboard(c, temp, p[0] + p[4], p[1] + p[5]);

        return MOVED;
    } 

    if (strcmp(s, "quit") == 0) {
	    return EXIT;
    }

    return UNKNOWN;
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
        case REDO:
            return "redo!";
        case NOTHING_TO_REDO_OR_UNDO:
            return "Nothing to undo/redo";
        case CHPEN:
            return "Pen changed!";
        case CHPENERR:
            return "Error in changing pen";
        case FILL:
            return "filled!";
        case UNKNOWN:
            return "error: unknown command";
        case SHOWLAYER:
            return "layer shown!";
        case HIDELAYER:
            return "layer hidden!";
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
        case CHPENSIZE:
            return "Pensize changed!";
        case ERASER:
            return "Changed to eraser!";
        case MARKER:
            return "Changed to marker!";
        case COLORCHANGED:
            return "Color changed!";
        case PENSIZENOTINT:
            return "Pensize is not in proper integer";
        case PENSIZETOOBIG:
            return "Pensize is too big - must be below 10";
        case COLORNOTINT:
            return "Color is not in proper integer format";
        case COLORNOTINRANGE:
            return "Color is not in range - must be between 0 ~ 255";
        case ADDLAYER:
            return "Layer added!";
        case CHANGELAYER:
            return "Layer changed!";
        case REMOVELAYER:
            return "Layer removed!";
        case INSERTLAYER:
            return "Layer inserted!";
        case MOVELAYER:
            return "Layer moved!";
        case COPYLAYER:
            return "Layer copied!";
        case MERGELAYER:
            return "Layer merged!";
        case RESIZE:
            return "Resized!";
        case CHANGERATIO:
            return "horizontal ratio changed!";
        case COPIEDTOCLIPBOARD:
            return "Copied to clipboard!";
        case PASTEDFROMCLIPBOARD:
            return "Pasted from clipboard!";
        case MOVED:
            return "Moved!";
        case MIRROR:
            return "mirrored!";
        case LAYEROUTOFBOUNDS:
            return "Layer is out of bounds";
        case COPYLAYERERR:
            return "Error in copying layer";
        case MERGELAYERERR:
            return "Error in merging layer";
        case RATIOERR:
            return "Error in ratio ratio - cannot be a negative number";
        case CHOOSEREVERSEDIRECTION:
            return "choose an appropriate reverse mode (x for x-axis, y for y-axis)";
        case REVERSINGERROR:
            return "Error in reversing";
        case UNKNOWNLAYERERR:
            return "Unknown error regarding handling layers";
    }
    return NULL;
}

Layer *get_layer(Canvas *c, int idx) { //getting the layer of a particular idx
    size_t size = c -> layer_list -> size;
    if (idx >= size || idx < 0) {
        return NULL;
    }

    Layer *layer = c -> layer_list -> begin;
    for (int i = 0; i<idx; i++) {
        layer = layer -> next;
    }

    return layer;
}

Layer *current_layer(Canvas *c) {
  return get_layer(c, c->layer_idx);
}

Layer *final_layer(Canvas *c) {
  Layer *layer = c->layer_list->begin;

  assert(layer != NULL);

  while (layer->next != NULL) layer = layer->next;

  return layer;
}

int mirror_layer(Canvas *c, int idx, char *mode) {

    size_t size = c->layer_list->size;
    if (idx >= size) {
    return 1;
    }

    Layer *layer = get_layer(c, idx);
    Clipboard *clip = make_clipboard();
    int w = c->width;
    int h = c->height;
    clip_to_clipboard(c, clip, 0, 0, w, h);
    if (strcmp(mode, "x") == 0) {
        for (int x=0; x<w; x++) {
            for (int y=0; y<h; y++) {
            layer->board[x][y] = clip->board[x][h-1-y];
            layer->color[x][y] = clip->color[x][h-1-y];
            layer->highlight[x][y] = clip->highlight[x][h-1-y];
            }
        }
    } 
    else if (strcmp(mode, "y") == 0) {
        for (int x=0; x<w; x++) {
            for (int y=0; y<h; y++) {
            layer->board[x][y] = clip->board[w-1-x][y];
            layer->color[x][y] = clip->color[w-1-x][y];
            layer->highlight[x][y] = clip->highlight[w-1-x][y];
            }
        }
    } 
    else {
        free_clipboard(clip);
        return 1;
    }

    free_clipboard(clip);
    return 0;
}


int switch_to_layer(Canvas *c, int idx) {
    size_t size = c->layer_list->size;
    if (idx >= size || idx < 0) {
    return 1;
    }
    c->layer_idx = idx;
    return 0;
}

int **make_matrix(int width, int height) {
    int *temp_array = (int *)malloc(width * height * sizeof(int));
    memset(temp_array, 0, width * height * sizeof(int));
    int **array_2D = malloc(width*sizeof(int*));
    for (int i = 0; i<width; i++) {
        array_2D[i] = temp_array + i * height;
    }
    return array_2D;
}

int **copy_matrix(int width, int height, int **array) {
    int **new_array = make_matrix(width, height);
    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            new_array[x][y] = array[x][y];
        }
    }
    return new_array;
}

void free_matrix(int **array) {
    if (array == NULL) return;
    free(array[0]);
    free(array);
}

int check_if_in_board(int x, int y, Canvas *c) {
    if (x < 0 || x >= c->width || y < 0 || y >= c->height) return 0;
    return 1;
}

void clip_to_clipboard(Canvas *c, Clipboard *clip, int x0, int y0, int w, int h) {
    clip -> width = w;
    clip -> height = h;

    free_matrix(clip -> board);
    free_matrix(clip -> color);
    free_matrix(clip -> highlight);

    clip->board = make_matrix(w, h);
    clip->color = make_matrix(w, h);
    clip->highlight = make_matrix(w, h);

    Layer *layer = current_layer(c);
    for (int i=0; i < w; i++) {
        for (int j=0; j < h; j++) { 
            if (!check_if_in_board(x0+i,y0+j,c)) continue;
            clip->board[i][j] = layer->board[x0 + i][y0 + j];
            clip->color[i][j] = layer->color[x0 + i][y0 + j];
            clip->highlight[i][j] = layer->highlight[x0 + i][y0 + j];
        }
    }
}

void paste_from_clipboard(Canvas *c, Clipboard *clip, int x0, int y0){
    Layer *layer = current_layer(c);
    for (int i = 0; i < clip -> width; i++) {
        for (int j = 0; j < clip -> height; j++) {
            if (!check_if_in_board(x0 + i, y0 + j, c)) continue;
            if (clip->board[i][j] == 0) continue;
            layer->board[x0 + i][y0 + j] = clip->board[i][j];
            layer->color[x0 + i][y0 + j] = clip->color[i][j];
            if (clip->highlight[i][j] != 0)  {
                layer->highlight[x0 + i][y0 + j] = clip->highlight[i][j];
            }
        }
    }
}

Clipboard *make_clipboard(){
    Clipboard *clipboard = (Clipboard*)malloc(sizeof(Clipboard));

    clipboard->width = 0;
    clipboard->height = 0;
    clipboard->board = NULL;
    clipboard->color = NULL;
    clipboard->highlight = NULL;

    return clipboard;
}

void free_clipboard(Clipboard *clip){
    if (clip == NULL) return;
    free_matrix(clip->board);
    free_matrix(clip->color);
    free_matrix(clip->highlight);
    free(clip);
}

//retrieving the last command that was used
Command *final_command(History *his, const int real) {

    if (real) {
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

//removing all commands
void remove_commands(Command *command) {
    while (command != NULL) {
        Command *temp = command;
        command = command->next;

        free(temp->str);
        free(temp);
    }
}

//making a command and preparing it to be part of a linear list
Command *make_command(char *str) {

    Command *command = (Command*)malloc(sizeof(Command));
    command->bufsize = strlen(str) + 1;
    command->next = NULL;
    command->str = (char*)malloc(command->bufsize);
    strcpy(command->str, str);

    return command;
}

//updating the history
void update_history(History *his, char *str) {
    Command *command = make_command(str);

    Command *step = final_command(his, 0);
    if (step != NULL) {
        remove_commands(step->next); // undoで取り消されたコマンドをすべて削除
        step->next = command;
    } 
    else {
        remove_commands(his->begin);
        his->begin = command;
    }
    his -> size ++;
}

//retrieving the layer index
int layer_idx(int default_value) {
    int len;
    int *index_list = idx_reader_for_layer(&len);
    int idx = default_value;
    if (len >= 1) {
        idx = index_list[0] - 1;
    }
    if (default_value == -1 && len == 0) {
    }
    return idx;
}

//reading the index (to be used in the interpret command function)
int *idx_reader_for_layer(int *len) {

  char **b = (char**)calloc(32, sizeof(char*));

  int count = 0;
  while(1) {
    b[count] = strtok(NULL, " ");
    if (b[count] == NULL) {
      break;
    }
    count++;
  }

  *len = count;
  int *p = (int*)calloc(count, sizeof(int));

  for (int i = 0 ; i < count ; i++){
    char *e;
    long v = strtol(b[i],&e, 10);
    if (*e != '\0'){
      return NULL;
    }
    p[i] = (int)v;
  }

  free(b);

  return p;
}

//creating a layer
Layer *make_layer(int width, int height) {
    Layer *layer = (Layer*)malloc(sizeof(Layer));
    layer->vis = 1;
    layer->board = make_matrix(width, height);
    layer->color = make_matrix(width, height);
    layer->highlight = make_matrix(width, height);
    layer->next = NULL;
    layer->prev = NULL;

    return layer;
}

//adding an additional layer
void add_layer(Canvas *c) {
    Layer *last = final_layer(c);
    assert(last != NULL); //assert so that when last is a NULL

    Layer *new = make_layer(c->width, c->height);

    last->next = new;
    new->prev = last;

    c->layer_list->size++;
}

//insert layer in designated spot
int insert_layer(Canvas *c, int idx, Layer *layer) {

    size_t size = c->layer_list->size;

    if (idx > size || idx < 0) {
        return 1;
    }

    if (idx == c->layer_list->size) { 
        Layer *final = final_layer(c);
        final -> next = layer;

        layer -> prev = final;
        layer -> next = NULL;
    } 
    else {
        Layer *next_layer = get_layer(c, idx);

        if (idx == 0) {
            c -> layer_list -> begin = layer;
            layer->prev = NULL;
        } 
        else {
            next_layer->prev->next = layer;
            layer->prev = next_layer->prev;
        }

        next_layer->prev = layer;
        layer->next = next_layer;
    }

    c->layer_list->size++;

    return 0;
}

//resizing the layer
int resize_layer(Canvas *c, int idx, int width, int height) {

    int original_w = c->width;
    int original_h = c->height;

    int **new_board = make_matrix(width, height);
    int **new_color = make_matrix(width, height);
    int **new_highlight = make_matrix(width, height);

    Layer *layer = get_layer(c, idx);

    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {

            if (x >= original_w || y >= original_h) {
                new_board[x][y] = 0;
                new_color[x][y] = 0;
                new_highlight[x][y] = 0;
            } 
            else {
                new_board[x][y] = layer->board[x][y];
                new_color[x][y] = layer->color[x][y];
                new_highlight[x][y] = layer->highlight[x][y];
            }

        }
    }

    free(layer->board);
    free(layer->color);
    free(layer->highlight);

    layer->board = new_board;
    layer->color = new_color;
    layer->highlight = new_highlight;

    return 0;
}

//using the resized layer to resize the canvas output
int resize_canvas(Canvas *c, int width, int height) {

    if (width <= 0 || height <= 0)
        return 1;

    int len = c->layer_list->size;
    for (int i=0; i<len; i++) {
        resize_layer(c, i, width, height);
    }

    int delta_h = height - c->height;

    c->width = width;
    c->height = height;
    clear_screen();
    if (delta_h < 0) {
        delta_h = -delta_h;
        for (int i=0; i<delta_h; i++) {
            rewind_screen(1);
            clear_single_line();
        }
    } 
    else {
        for (int i=0; i<delta_h; i++) {
            unwind_screen(1);
        }
    }

    return 0;
}

//copying a particular layer
int copy_layer(Canvas *c, int idx) {

    size_t size = c->layer_list->size;
    if (idx >= size || idx < 0) {
        return 1;
    }

    const int width = c->width;
    const int height = c->height;

    Layer *layer = get_layer(c, idx);
    Layer *new_layer = make_layer(width, height);

    for (int x=0; x<width; x++) {
        for (int y=0; y<height; y++) {
            new_layer->board[x][y] = layer->board[x][y];
            new_layer->color[x][y] = layer->color[x][y];
            new_layer->highlight[x][y] = layer->highlight[x][y];
        }
    }

    new_layer->vis = 1;


    insert_layer(c, c->layer_list->size, new_layer);

    return 0;
}

//hide a particular layer (hide own if x idx input)
int hide_layer(Canvas *c, int idx) {

    size_t size = c->layer_list->size;
    if (idx >= size || idx < 0) {
        return 1;
    }

    get_layer(c, idx)->vis = 0;
    return 0;
}

//show a hidden partiuclar layer (show own if x idx input)
int show_layer(Canvas *c, int idx) {
    size_t size = c->layer_list->size;
    if (idx >= size || idx < 0) {
        return 1;
    }

    get_layer(c, idx)->vis = 1;
    return 0;
}

//merge a layer w previous(default) or designated idx layer
int merge_layer(Canvas *c, int idx) {

    size_t size = c->layer_list->size;
    if (idx > size || idx < 0) {
        return 1;
    }

    Layer *layer = get_layer(c, idx);
    Layer *prev_layer = layer->prev;

    if (prev_layer == NULL) {
    printf("none!\n");
    return 1;
    }


    for (int x=0; x<c->width; x++) {
        for (int y=0; y<c->height; y++) {

            if (layer->board[x][y] == 0) continue;

            prev_layer->board[x][y] = layer->board[x][y];
            prev_layer->color[x][y] = layer->color[x][y];

            if (layer->highlight[x][y] != 0) {
                prev_layer->highlight[x][y] = layer->highlight[x][y];
            }

        }
    }

    remove_layer(c, idx, 1);

    return 0;
}

//rm particular layer
int remove_layer(Canvas *c, int idx, int should_free) {

    size_t size = c->layer_list->size;
    if (idx >= size || idx < 0) {
        printf("out of bounds!\n");
        return 1;
    }

    Layer *layer = get_layer(c, idx);

    if (idx == 0 && layer->next == NULL) {
        printf("Can't remove all layers.\n");
        return 1;
    }

    if (layer->prev != NULL) {
        layer->prev->next = layer->next;
    }

    if (layer->next != NULL) {
        layer->next->prev = layer->prev;
    }

    if (idx == 0) {
        c->layer_list->begin = layer->next;
    }

    if (should_free) { 
        free_layer(layer);
    }

    c->layer_list->size--;

    if (c->layer_idx >= c->layer_list->size) {
        switch_to_layer(c, c->layer_list->size - 1);
    }

    return 0;
}

int move_layer(Canvas *c, int a, int b) {

    size_t size = c->layer_list->size;
    if (a >= size || b >= size || a < 0 || b < 0) {
        return 1;
    }

    Layer *now_layer = current_layer(c);
    Layer *layer = get_layer(c, a);


    remove_layer(c, a, 0); 
    insert_layer(c, b, layer);


    for (int i=0; i<c->layer_list->size; i++) {
        if (get_layer(c, i) == now_layer) {
            c->layer_idx = i;
        }
    }

    return 0;
}

void free_layer(Layer *layer) {
    free_board(layer);
    free(layer);
}

void free_all_layers(Canvas *c) {
  Layer *layer = c->layer_list->begin;

  while(layer != NULL) {
    Layer *temp = layer;
    layer = layer->next;

    free_layer(temp);
  }
}

void free_board(Layer *layer) {
  free_matrix(layer->board);
  free_matrix(layer->color);
  free_matrix(layer->highlight);
}
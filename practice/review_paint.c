#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <ctype.h>
//#include <errno.h> // for error catch

//Structure for canvas
typedef struct canvas{
    int width; //width of canvas
    int height; //height of canvas
    char **canvas; //to store the 2D array of the canvas
    char pen; //to store what character will be used as the 'pen'
}Canvas;

typedef struct command Command; //prototyping so we can use Command within struct command

struct command {
    char *str;
    size_t bufsize;
    Command *next;
};

typedef struct {
    Command *begin;
    size_t bufsize;
}History;

//functions for Canvas type;

Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(Canvas *c);
void free_canvas(Canvas *c);

//Display functions

void rewind_screen(unsigned int line);
void clear_command(void);
void clear_screen(void);

// enum for interpret_command results
typedef enum res{EXIT, LINE, UNDO, SAVE, UNKNOWN, ERRNONINT, ERRLACKARGS, NOCOMMAND} Result;

char *strresult(Result res);

int max(const int a, const int b);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
Result interpret_command(const char *command, History *his, Canvas *c);
void save_history(const char *filename, History *his);

Command *push_command(History *his, const char *str);

int main(int argc,char **argv) {
    //for history recording

    const int bufsize = 1000;

    History his = (History){.begin = NULL, .bufsize = bufsize};

    int width;
    int height;
    if (argc != 3) {
        fprintf(stderr, "usage: %s <width> <height> \n", argv[0]);
        return EXIT_FAILURE;
    }
    else {
        char *e;
        long w = strtol(argv[1], &e, 10); //&e indicates the character where the conversion stopped
        if (*e != '\0') {
            fprintf(stderr, "%s: irregular character found %s\n", argv[1], e);
            return EXIT_FAILURE;
        }
        long h = strtol(argv[2], &e, 10);
        if (*e != '\0') {
            fprintf(stderr, "%s: irregular character found %s\n", argv[2], e);
            return EXIT_FAILURE;
        }

        width = (int)w;
        height = (int)h;
    }

    char pen = '*';

    char buf[bufsize];

    Canvas *c = init_canvas(width, height, pen);

    printf("\n");

    while(1) {
        print_canvas(c);
        printf("* > ");
        if (fgets(buf, bufsize, stdin) == NULL) {
            break;
        }

        const Result r = interpret_command(buf, &his, c);

        if (r == EXIT) {
            break;
        }

        clear_command();
        printf("%s\n", strresult(r));
        if (r == LINE) {
            push_command(&his, buf);
        }


        rewind_screen(2);
        clear_command();
        rewind_screen(height+2);
    }

    clear_screen();
    free_canvas(c);

    return 0;

}


Canvas *init_canvas(int width, int height, char pen) {
    Canvas *new = (Canvas *)malloc(sizeof(Canvas)); //malloc to make a new Canvas type to return at the end
    new -> width = width; //input the width and height to the new Canvas type
    new -> height = height;
    new -> canvas = (char **)malloc(width * sizeof(char *)); //allocate memory for a 2D array
    //that has width * sizeof a 1d array???

    char *tmp = (char * )malloc(width*height*sizeof(char));
    memset(tmp, ' ', width*height*sizeof(char));
    for (int i = 0; i < width; ++i) {
        new -> canvas[i] = tmp + i * height; //canvas[i] is a pointer to the beginning of a row
        //the first row will have the same address of tmp, since the address of tmp also points to
        //the beginning of temp (the beggining of the matrix)
        //!!!!!the matrix looks inversed here because the program wants to read in [x][y] format
    }

    //choose pen as selected pen
    new -> pen = pen;

    //return this new Canvas type
    return new;
}

void reset_canvas(Canvas *c) {
    //basically setting the given Canvas as an empty canvas
    const int width = c -> width;
    const int height = c -> height;
    memset(c -> canvas[0], ' ', width*height*sizeof(char));
}

void print_canvas(Canvas *c) {
    const int height = c -> height;
    const int width = c -> width;
    char **canvas = c-> canvas;

    //Upper wall
    printf("+");
    for (int x = 0; x < width; x++) {
        printf("-");
    }
    printf("+\n");

    //Outer walls and the insides
    for (int y = 0; y<height; y++) {
        printf("|");
        for (int x = 0; x<width; x++) {
            const char c = canvas[x][y];
            putchar(c);
        }
        printf("|\n");
    }

    //Bottom wall
    printf("+");
    for (int x = 0; x < width; x++) {
        printf("-");
    }
    printf("+\n");
    fflush(stdout);

}

void free_canvas(Canvas *c) {
    free(c->canvas[0]);
    free(c->canvas);
    free(c);
}


void rewind_screen(unsigned int line){
    printf("\e[%dA",line);
}

//escape sequence to clear command line
void clear_command(void){
    printf("\e[2K");
}

//escape sequence to clear screen
void clear_screen(void){
    printf( "\e[2J");
}

int max(const int a, const int b) {
    return (a > b) ? a:b;
}

void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1){
    const int width = c -> width;
    const int height = c -> height;
    char pen = c -> pen;

    const int n = max(abs(x1 - x0), (y1 - y0));
    if ((x0 >= 0) && (x0 < width) && (y0 >= 0) && (y0 < height)) {
        c -> canvas[x0][y0] = pen;
        for (int i = 1; i <= n; i++) {
            const int x = x0 + i*(x1-x0)/n;
            const int y = y0 + i*(y1-y0)/n;
            if ((x0 >= 0) && (x0 < width) && (y0 >= 0) && (y0 < height)) {
                c -> canvas[x][y] = pen;
            }
        }
    }
}

void save_history(const char *filename, History *his) {
    const char *default_history_file = "history.txt";
    if (filename == NULL){
        filename = default_history_file;
    }

    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "error: cannot open %s.\n",filename);
        return;
    }

    //printout all the commands in the file using fprintf
    //start with the first one (his -> begin), where the str inside his->begin has the first command stored
    //then keep going until the final command, for which there will be no next and p->next will be NULL
    for (Command *p = his -> begin; p!=NULL; p= p->next) {
        fprintf(fp, "%s", p->str);
    }

    fclose(fp);
}

Result interpret_command(const char *command, History *his, Canvas *c) {
    char buf[his->bufsize];
    strcpy(buf, command);
    buf[strlen(buf)-1] = 0; //removing the newline character at the end

    const char *s = strtok(buf, " ");
    if (s == NULL) {
        return UNKNOWN;
    }

    if (strcmp(s, "line") == 0){
        int p[4] = {0};
        char *b[4];
        for (int i = 0; i < 4; i++){
            b[i] = strtok(NULL, " ");
            if (b[i] == NULL) {
                return ERRLACKARGS;
            }
        }

        for (int i = 0; i<4; i++) {
            char *e;
            long v = strtol(b[i], &e, 10);
            if (*e != '\0') {
                return ERRNONINT;
            }
            p[i] = (int)v;
        }

        draw_line(c, p[0], p[1], p[2], p[3]);
        return LINE;
    }

    if (strcmp(s, "save") == 0) {
        s = strtok(NULL, " ");
        save_history(s, his);
        return SAVE;
    }

    if (strcmp(s, "undo") == 0) {
        reset_canvas(c);
        Command *p = his -> begin;
        if (p == NULL) {
            return NOCOMMAND;
        }
        else {
            Command *q = NULL;
            while (p -> next != NULL) {
                interpret_command(p->str, his, c);
                q = p;
                p = p -> next;
            }
            if (q == NULL) {
                his -> begin = NULL;
            }
            else {
                q -> next = NULL;
            }
            free(p -> str);
            free(p);
            return UNDO;
        }
    }

    if (strcmp(s, "quit") == 0) {
        return EXIT;
    }
    return UNKNOWN;
}

Command *push_command(History *his, const char *str) {
    Command *c = (Command *)malloc(sizeof(Command));
    char *s = (char*)malloc(his -> bufsize);
    strcpy(s, str);

    *c = (Command){.str = s, .bufsize = his -> bufsize, .next = NULL};

    Command *p = his -> begin;

    if (p == NULL) {
        his->begin = c;
    }
    else {
        while (p -> next != NULL) {
            p = p -> next;
        }
        p -> next = c;
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
    case UNDO:
	return "undo!";
    case UNKNOWN:
	return "error: unknown command";
    case ERRNONINT:
	return "Non-int value is included";
    case ERRLACKARGS:
	return "Too few arguments";
    case NOCOMMAND:
	return "No command in history";
    }
    return NULL;
}


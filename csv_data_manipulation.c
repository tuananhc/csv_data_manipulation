#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#define MAXCOLS 	20  	/* maximum number of columns to be handled */
#define MAXROWS 	999 	/* maximum number of rows to be handled */
#define LABLEN 		20   	/* maximum length of each column header */
#define LINELEN 	100 	/* maximum length of command lines */
#define ERROR 		(-1) 	/* error return value from some functions */
#define O_NOC 		'-' 	/* the "do nothing" command */
#define O_IND 		'i' 	/* the "index" command */
#define O_ANA 		'a' 	/* the "analyze" command */
#define O_DPY 		'd' 	/* the "display" command */
#define O_PLT 		'p' 	/* the "plot" command */
#define O_SRT 		's' 	/* the "sort" command */
#define CH_COMMA 	',' 	/* comma character */
#define CH_CR 		'\r'    /* pesky CR character in DOS-format files */
#define CH_NL 		'\n'    /* newline character */
#define MAXPLOTLEN	60	/* maximum length of a column in a plot */
#define EPSILON 	1e-6    /* deemed 2 double values are equal */

typedef char head_t[LABLEN + 1];
typedef double csv_t[MAXROWS][MAXCOLS];

/****************************************************************/

/* function prototype */ 

void get_csv_data(csv_t D, head_t H[], int *dr, int *dc, int argc,
                  char *argv[]);
void error_and_exit(char *msg);
void print_prompt(void);
int get_command(int dc, int *command, int ccols[], int *nccols);
void handle_command(int command, int ccols[], int nccols,
                    csv_t D, head_t H[], int dr, int dc);
void do_index(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
/* add further function prototypes below here */
void do_analyze(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
void do_display(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
void do_sort(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
void do_plot(csv_t D, head_t H[], int dr, int dc, int ccols[], int nccols);
void double_swap(double *p1, double *p2);
void row_swap(csv_t D, int nccols, int r1, int r2);
int is_different(double x1, double x2);
double get_col_max(csv_t D, int col_num, int dr);
double get_col_min(csv_t D, int col_num, int dr);
double get_col_avg(csv_t D, int col_num, int dr);
void print_column_headings(csv_t D, head_t H[], int ccols[], int nccols);
int check_sorted_column(csv_t D, int dr, int col_num);
int count_instances(csv_t D, int dr, int ccols[], int nccols, int row_num);
void plot(int scale, int ccols[], int nccols, int count_arr[],
          double min, double max, double step);
int get_plot_scale(int count_arr[], int n);
void print_data(csv_t D, int ccols[], int nccols, int count, int row_num);
void get_count(csv_t D, int dr, int ccols[], int nccols, double max,
                double min, double step, int count_arr[]);
void divide_range(csv_t D1, csv_t D2, int dr, int col_num);

/****************************************************************/

/* main program controls all the action */
int 
main(int argc, char *argv[]) {
    head_t H[MAXCOLS];  /* labels from the first row in csv file */
    csv_t D;            /* the csv data stored in a 2d matrix */
    int dr = 0, dc = 0; /* number of rows and columns in csv file */
    int ccols[MAXCOLS];
    int nccols;
    int command;

    /* reads csv data from a file named on the
       commandline and saves it to D, H, dr, and dc */
    get_csv_data(D, H, &dr, &dc, argc, argv);

    /* processing commands against it */

    print_prompt();
    while (get_command(dc, &command, ccols, &nccols) != EOF) {
        handle_command(command, ccols, nccols,
                       D, H, dr, dc);
        print_prompt();
    }

    return 0;
}

/****************************************************************/

/* prints the prompt indicating ready for input */
void 
print_prompt(void) {
    printf("> ");
}

/****************************************************************/

/* read a line of input into the array passed as argument
   returns false if there is no input available
   all whitespace characters are removed
   all arguments are checked for validity
   if no argumnets, the numbers 0..dc-1 are put into the array */

int 
get_command(int dc, int *command, int columns[], int *nccols) {
    int i = 0, c, col = 0;
    char line[LINELEN];
    /* command is in first character position */
    if ((*command = getchar()) == EOF) {
        return EOF;
    }
    /* and now collect the rest of the line, integer by integer,
	   sometimes in C you just have to do things the hard way */
    while (((c = getchar()) != EOF) && (c != '\n')) {
        if (isdigit(c)) {
            /* digit contributes to a number */
            line[i++] = c;
        } else if (i != 0) {
            /* reached end of a number */
            line[i] = '\0';
            columns[col ++] = atoi(line);
            /* reset, to collect next number */
            i = 0;
        } else {
            /* just discard it */
        }
    }
    if (i > 0) {
        /* reached end of the final number in input line */
        line[i] = '\0';
        columns[col ++] = atoi(line);
    }

    if (col == 0) {
        /* no column numbers were provided, so generate them */
        for (i = 0; i < dc; i ++) {
            columns[i] = i;
        }
        *nccols = dc;
        return !EOF;
    }

    /* otherwise, check the ones that were typed against dc,
	   the number of cols in the CSV data that was read */
    for (i = 0; i < col; i ++) {
        if (columns[i] < 0 || columns[i] >= dc) {
            printf("%d is not between 0 and %d\n",
                   columns[i], dc);
            /* and change to "do nothing" command */
            *command = O_NOC;
        }
    }
    /* all good */
    *nccols = col;
    return !EOF;
}

/****************************************************************/

/* this next is a bit of magic code that you can ignore for now
   and that will be covered later in the semester; it reads the
   input csv data from a file named on the commandline and saves
   it into an array of character strings (first line), and into a
   matrix of doubles (all other lines), using the types defined
   at the top of the program.  If you really do want to understand
   what is happening, you need to look at:
   -- The end of Chapter 7 for use of argc and argv
   -- Chapter 11 for file operations fopen(), and etc
*/

void 
get_csv_data(csv_t D, head_t H[], int *dr, int *dc, int argc,
                  char *argv[]) {
    FILE *fp;
    int rows = 0, cols = 0, c, len;
    double num;

    if (argc < 2) {
        /* no filename specified */
        error_and_exit("no CSV file named on commandline");
    }
    if (argc > 2) {
        /* confusion on command line */
        error_and_exit("too many arguments supplied");
    }
    if ((fp = fopen(argv[1], "r")) == NULL) {
        error_and_exit("cannot open CSV file");
    }

    /* ok, file exists and can be read, next up, first input
	   line will be all the headings, need to read them as
	   characters and build up the corresponding strings */
    len = 0;
    while ((c = fgetc(fp)) != EOF && (c != CH_CR) && (c != CH_NL)) {
        /* process one input character at a time */
        if (c == CH_COMMA) {
            /* previous heading is ended, close it off */
            H[cols][len] = '\0';
            /* and start a new heading */
            cols += 1;
            len = 0;
        } else {
            /* store the character */
            if (len == LABLEN) {
                error_and_exit("a csv heading is too long");
            }
            H[cols][len] = c;
            len++;
        }
    }
    /* and don't forget to close off the last string */
    H[cols][len] = '\0';
    *dc = cols + 1;

    /* now to read all of the numbers in, assumption is that the input
	   data is properly formatted and error-free, and that every row
	   of data has a numeric value provided for every column */
    rows = cols = 0;
    while (fscanf(fp, "%lf", &num) == 1) {
        /* read a number, put it into the matrix */
        if (cols == *dc) {
            /* but first need to start a new row */
            cols = 0;
            rows += 1;
        }
        /* now ok to do the actual assignment... */
        D[rows][cols] = num;
        cols++;
        /* and consume the comma (or newline) that comes straight
		   after the number that was just read */
        fgetc(fp);
    }
    /* should be at last column of a row */
    if (cols != *dc) {
        error_and_exit("missing values in input");
    }
    /* and that's it, just a bit of tidying up required now  */
    *dr = rows + 1;
    fclose(fp);
    printf("    csv data loaded from %s", argv[1]);
    printf(" (%d rows by %d cols)\n", *dr, *dc);
    return;
}

void 
error_and_exit(char *msg) {
    printf("Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

/* the 'i' index command */
void 
do_index(csv_t D, head_t H[], int dr, int dc,
            int ccols[], int nccols) {
    int i, c;
    printf("\n");
    for (i = 0; i < nccols; i ++) {
        c = ccols[i];
        printf("    column %2d: %s%c", c, H[c], CH_NL);
    }
}

/* the 'd' display command */
void 
do_display(csv_t D, head_t H[], int dr, int dc,
            int ccols[], int nccols) {
    /* a csv dataset of rounded data from the original csv */
    csv_t rounded_csv;
    int i, c; /* counter variables */
    int count;
    printf("\n");

    /* printing the label of each column */
    print_column_headings(D, H, ccols, nccols);

    for (i = 0; i < nccols; i ++) {
        c = ccols[i];
        divide_range(D, rounded_csv, dr, c);
    }

    /* count the similar instances and print out the values of the csv file */
    for (i = 0; i < dr; i ++) {
        count = count_instances(D, dr, ccols, nccols, i);
        print_data(rounded_csv, ccols, nccols, count, i);
        i += count - 1;
    }    
    return;
}

/* the 'a' analyze command */
void 
do_analyze(csv_t D, head_t H[], int dr, int dc,
            int ccols[], int nccols) {
    double max, min, median, avg;
    int i, c; /* counter variables */
    printf("\n");
    for (i = 0; i < nccols; i ++) {
        c = ccols[i];
        median = 0;
        /* find the required values of a column */
        max = get_col_max(D, c, dr);
        min = get_col_min(D, c, dr);
        avg = get_col_avg(D, c, dr);
        /* if the column is sorted, the median is calculated */
        if (check_sorted_column(D, dr, c)) {
            printf("         %8s (sorted)%c", H[c], CH_NL);
            if (dr % 2 == 0) {
                median = (D[dr / 2 - 1][c] + D[dr / 2][c]) / 2;
            } else {
                median = D[dr / 2][c];
            }
        } else {
            printf("         %8s%c", H[c], CH_NL);
        }
        printf("    max = %7.1f%c", max, CH_NL);
        printf("    min = %7.1f%c", min, CH_NL);
        printf("    avg = %7.1f%c", avg, CH_NL);
        if (median) {
            printf("    med = %7.1f%c", median, CH_NL);
        }
        if (i != nccols - 1) {
            printf("\n");
        }
    }
    return;
}

/* the 's' sort command */
void 
do_sort(csv_t D, head_t H[], int dr, int dc,
             int ccols[], int nccols) {
    int i, j, k, c; /* counter variables */
    printf("\n");
    printf("    sorted by:");
    for (i = 0; i < nccols; i ++) {
        c = ccols[i];
        if (i != nccols - 1) {
            printf(" %s%c", H[c], CH_COMMA);
        } else {
            printf(" %s%c", H[c], CH_NL);
        }
    }
    /* sort the rows using insertion sort */
    for (i = nccols - 1; i >= 0; i --) {
        c = ccols[i];
        if (check_sorted_column(D, dr, c)) {
            continue;
        } else {
            for (j = 1; j < dr; j ++) {
                for (k = j - 1; k >= 0 && D[k][c] > D[k + 1][c]; k--) {
                    row_swap(D, dc, k, k + 1);
                }
            }
        }
    }
    return;
}

/* the 'p' plot command' */
void 
do_plot(csv_t D, head_t H[], int dr, int dc,
             int ccols[], int nccols) {
    int i, c; 
    int scale; 
    int count_arr[nccols * 10]; /* an array to store the values of counts */

    /* maximum and minimum values across all selected columns */
    double max, min, step; 

    /* maximum and minimum values of a specific column */
    double col_max, col_min; 

    printf("\n");
    max = get_col_max(D, ccols[0], dr);
    min = get_col_min(D, ccols[0], dr);
    
    /* find the maximum and minimum values across all selected columns */
    for (i = 1; i < nccols; i ++) {
        c = ccols[i];
        col_max = get_col_max(D, c, dr);
        col_min = get_col_min(D, c, dr);
        if (col_max > max) {
            max = col_max;
        }
        if (col_min < min) {
            min = col_min;
        }
    }

    /* if all values are similar, i.e the max value equals the min value, 
    a plot is not necessary and the function is terminated */
    if (max == min) {
        printf("all selected elements are %.1f%c", D[0][c], CH_NL);
        return;
    }

    /* enlarge the range by EPSILON value */
    min -= EPSILON;
    max += EPSILON;
    step = (max - min) / 10;

    /* get the number of elements within each range */
    get_count(D, dr, ccols, nccols, max, min, step, count_arr);

    /*get the appropriate scale of the plot */
    scale = get_plot_scale(count_arr, nccols * 10);

    /* plot the data */
    plot(scale, ccols, nccols, count_arr, min, max, step);

    return;
}

/* this function examines each incoming command and decides what
to do with it, kind of traffic control, deciding what gets
called for each command, and which of the arguments it gets */
void 
handle_command(int command, int ccols[], int nccols,
                    csv_t D, head_t H[], int dr, int dc) {
    if (command == O_NOC) {
        /* the null command, just do nothing */
    } else if (command == O_IND) {
        /* the 'i' index command */
        do_index(D, H, dr, dc, ccols, nccols);
    } else if (command == O_DPY) {
        /* the 'd' display command */
        do_display(D, H, dr, dc, ccols, nccols);
    } else if (command == O_ANA) {
        /* the 'a' analyze command */
        do_analyze(D, H, dr, dc, ccols, nccols);
    } else if (command == O_SRT) {
        /* the 's' sort command */
        do_sort(D, H, dr, dc, ccols, nccols);
    } else if (command == O_PLT) {
        /* the 'p' plot command */
        do_plot(D, H, dr, dc, ccols, nccols);
    } else {
        printf("command '%c' is not recognized"
               " or not implemented yet\n",
               command);
    }
    return;
}

/* this function swaps the values of 2 numbers */
void 
double_swap(double *p1, double *p2) {
    double tmp;
    tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    return;
}

/* the function checks if 2 double values are EPSILON value different
from each other */
int 
is_different(double x1, double x2) {
    return (x1 - x2 > EPSILON || x1 - x2 < -EPSILON);
}

/* ----functions used for the 's' sort command */

/* this function swaps 2 rows of an csv dataset */
void 
row_swap(csv_t D, int dc, int r1, int r2) {
    int i;
    for (i = 0; i < dc; i ++) {
        double_swap(&D[r1][i], &D[r2][i]);
    }
    return;
}

/* ------------------------------- */

/* ---- functions used for the 'a' analyze command ---- */

/* this function returns the minimum value of a specific 
column in the given 2d array */
double
get_col_min(csv_t D, int col_num, int dr) {
    int i;
    double min = D[0][col_num];
    for (i = 1; i < dr; i ++) {
        if (D[i][col_num] < min) {
            min = D[i][col_num];
        }
    }
    return min;
}

/* this function returns the minimum value of a specific 
column in the given 2d array */
double
get_col_max(csv_t D, int col_num, int dr) {
    int i;
    double max = D[0][col_num];
    for (i = 1; i < dr; i ++) {
        if (D[i][col_num] > max) {
            max = D[i][col_num];
        }
    }
    return max;
}

/* this function returns the average value of a specific 
column in the given 2d array */
double
get_col_avg(csv_t D, int col_num, int dr) {
    int i;
    double sum = D[0][col_num];
    for (i = 1; i < dr; i ++) {
        sum += D[i][col_num];
    }
    return sum / dr;
}

/* the function checks if a column is sorted in the ascending order */
int 
check_sorted_column(csv_t D, int dr, int col_num) {
    int i;
    for (i = 0; i < dr - 1; i ++) {
        if (D[i][col_num] - D[i + 1][col_num] > EPSILON) {
            return 0;
        }
    }
    return 1;
}

/* ------------------------------- */

/* ---- functions used for the 'd' display command ---- */

/* the function prints out the headings of selected columns in a 
pyramid shape */
void
print_column_headings(csv_t D, head_t H[], int ccols[], int nccols) {
    int i, j, c;
    for (i = nccols - 1; i >= 0; i --) {
        c = ccols[i];
        for (j = 1; j <= i; j ++) {
            printf("        ");
        }
        printf("%8s%c", H[c], CH_NL);
    }
}

/* the function prints out the data of the selected row in a csv dataset 
and the number of consecutively similar instances of that row */
void
print_data(csv_t D, int ccols[], int nccols, int count, int row_num) {
    int i, c;
    for (i = 0; i < nccols; i ++) {
        c = ccols[i];
        printf(" %7.1f", D[row_num][c]);
    }
    if (count == 1) {
        printf("    (%2d instance)%c", count, CH_NL);
    } else {
        printf("    (%2d instances)%c", count, CH_NL);
    }
    return;
}

/* the function calculates the number of similar instances to 
the row starting at row_num in the csv data */
int 
count_instances(csv_t D, int dr, int ccols[], int nccols, int row_num) {
    int i, j, c;
    int count, stop;
    count = 1;
    stop = 0;
    for (i = row_num + 1; i < dr; i ++) {
        for (j = 0; j < nccols && !stop; j ++) {
            c = ccols[j];
            if (D[i][c] != D[row_num][c]) {
                stop = 1;
            }
        }
        if (!stop) {
            count += 1;
        }
    }
    return count;
}

/* the function divides a given dataset into ranges so that all values 
within that range are within EPSILON value of each other and assigns them 
into a new csv dataset */
void 
divide_range(csv_t D1, csv_t D2, int dr, int col_num) {
    int i, j, k;
    int count, stop;
    for (i = 0; i < dr; i ++) {
        D2[i][col_num] = D1[i][col_num];
        count = 1;
        stop = 0;
        for (j = i + 1; j < dr && !stop; j ++) {
            for (k = i; k < j && !stop; k ++) {
                if (is_different(D1[k][col_num], D1[j][col_num])) {
                    stop = 1;
                }
            }
            if (!stop) {
                count += 1;
            }
        }
        for (j = 1; j < count; j ++) {
            D2[i + j][col_num] = D1[i][col_num];
        }
        i += count - 1;
    }
    return;
}

/* ------------------------------- */

/* ---- functions used for the 'p' plot command ---- */

/* the function stores the number of elements in the selected columns
are within a specific range into an array */
void
get_count(csv_t D, int dr, int ccols[], int nccols, 
            double max, double min, double step, int count_arr[]) {
    int i, j, k, c;
    int count;
    for (i = 0; i < 10; i ++) {
        for (j = 0; j < nccols; j ++) {
            c = ccols[j];
            count = 0;
            for (k = 0; k < dr; k ++) {
                if (D[k][c] >= min + i * step && D[k][c] <=
                    min + (i + 1) * step) {
                    count += 1;
                }
            }
            count_arr[i * nccols + j] = count;
        }
    }
    return;
}

/* the function calculates the appropriate scale of a plot so 
that no bar is more than 60 elements wide */
int 
get_plot_scale(int count_arr[], int n) {
    int i;
    int max = count_arr[0];
    for (i = 1; i < n; i ++) {
        if (count_arr[i] > max) {
            max = count_arr[i];
        }
    }
    return max / 60 + 1;
}

/* the function plots a bar chart based on the given information */
void
plot(int scale, int ccols[], int nccols, int count_arr[], 
    double min, double max, double step) {
    int i, j, k, c;
    for (i = 0; i < 10; i ++) {
        printf("    %7.1f +%c", min, CH_NL);
        for (j = 0; j < nccols; j ++) {
            c = ccols[j];
            printf("    %7d |", c);
            /* print out the instances */
            for (k = 0; k < count_arr[i * nccols + j] / scale; k ++) {
                printf("]");
            }
            printf("\n");
        }
        min += step;
    }
    printf("    %7.1f +%c", max, CH_NL);
    printf("    scale = %d%c", scale, CH_NL);
    return;
}

/* ------------------------------- */

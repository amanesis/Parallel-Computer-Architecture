#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <sys/time.h>

#define PATH -1
#define NONE 0
#define UP 1
#define LEFT 2
#define DIAGONAL 3

long long int m = 0;
long long int n = 0;

const char* name_flag = "-name";
const char* input_flag = "-input";
const char* match_flag = "-match";
const char* mismatch_flag = "-mismatch";
const char* gap_flag = "-gap";
const int line_size = 256;

char* q_seq;
char* d_seq;
long int match;
long int mismatch;
long int gap;

double gettime(void)
{
	struct timeval ttime;
	gettimeofday(&ttime, NULL);
	return ttime.tv_sec+ttime.tv_usec * 0.000001;
}

char* concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}


int open_file(char* input, char* name, FILE* *inputfile, FILE* *outputfile)
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    char* ftp = concat(4, cwd, "/", input, ".txt"); //Path to input file
    *inputfile = fopen(ftp, "r");
    char* outfile = concat(4, cwd, "/Report_", name, ".txt");
    *outputfile = fopen(outfile, "w");
    free(outfile);
    return 0;
}

void printMatrix(int* matrix)
{
    long long int i, j;
    printf("-\t-\t");
   for (j = 0; j < m-1; j++)
	{
    	printf("%c\t", q_seq[j]);
    }
    printf("\n-\t");
    for (i = 0; i < n; i++)
	{
        for (j = 0; j < m; j++)
		{
        	if (j==0 && i>0) printf("%c\t", d_seq[i-1]);
            printf("%d\t", matrix[m * i + j]);
        }
        printf("\n");
    }
}

int parsingInfo(FILE* inputfile, int *pair_size, long int *q_size, long int *q_size_min, long int *d_size)
{
    char* line = malloc(line_size*sizeof(char));

    if (fgets(line,line_size,inputfile))
	{
        sscanf(line,"Pairs:\t%[^\n]",line); //reads formatted input from a string.
        *pair_size = atoi(line);
        printf("Pairs: %d\n" , *pair_size);
    }

    if (fgets(line, line_size, inputfile))
	{
        sscanf(line,"Q_Sz_Min:\t%[^\n]", line);
        *q_size_min = atoi(line);
        printf("Q size (min): %d\n", *q_size_min);
    }

    if (fgets(line, line_size, inputfile))
	{
        sscanf(line,"Q_Sz_Max:\t%[^\n]", line);
        *q_size = atoi(line);
        printf("Q size: %d\n", *q_size);
    }

    if (fgets(line, line_size, inputfile))
	{
        sscanf(line,"D_Sz_All:\t%[^\n]", line);
        *d_size = atoi(line);
        printf("D size: %d\n", *d_size);
    }
    else
	{
        printf("file error\n");
        free(line);
        return -1;
    }

    free(line);
    return 0;
}

int parsing(FILE* inputfile, FILE* outputfile, char *q_seq, char *d_seq)
{
    char* line = malloc(line_size*sizeof(char));
    int found_q = 0;

	while(fgets(line, line_size, inputfile) != NULL)
	{
		if (strncmp(line, "Q:", 2) == 0)
		{
    		sscanf(line, "Q:\t%[^\n]", line);
            fprintf(outputfile, "Q:\t%s\n", line);
			strcpy(q_seq, line);

			found_q = 1;
		}
		else if (strncmp(line, "D:", 2) == 0)
		{
			sscanf(line, "D:\t%[^\n]", line);
            fprintf(outputfile, "D:\t%s\n", line);
			strcpy(d_seq, line);
			found_q = 0;
		}
		else if (strncmp(line, "\t", 1) == 0 || strncmp(line, "  ", 2) == 0)
		{
			fprintf(outputfile, "%s", line);
			sscanf(line, "\t%[^\n]", line);

			if (found_q == 1) strcat(q_seq, line); else strcat(d_seq, line);
		}
	}
    free(line);
    return 0;
}

void Score(long int i, long int j, int *H,  int *P, int* maxPos)
{
	int up, left, diag;
    long long int index = m*i  + j;

    up = H[index - m] + gap;
    left = H[index - 1] + gap;
    diag = H[index - m - 1] + matchMissmatch(i, j);

    //Calculates the maximum
    int max = NONE;
    int pred = NONE;

    if (diag > max)
	{
        max = diag;
        pred = DIAGONAL;
	}
    if (up > max)
	{
        max = up;
        pred = UP;
    }
    if (left > max)
	{
        max = left;
        pred = LEFT;
    }

    H[index] = max;
    P[index] = pred;

    if (max > H[*maxPos]) *maxPos = index;
}

int matchMissmatch(long int i, long int j)
{
    if (q_seq[j-1] == d_seq[i-1])
    {
    	printf("OP Match: %d\n", match);
        return match; //match
    }
    else
    {
    	printf("OP MisMatch: %d\n", mismatch);
        return mismatch; //MisMatch
    }
}


int main(int argc, char* argv[])
{
    char* name ="nametest0";
    char* input = "D0";
    int pair_size = 0;
    long int q_size = 0;
    long int q_size_min = 0;
    long int d_size = 0;
    int i,j;

    match = 5;
    mismatch = -3;
    gap = -4;

    FILE* inputfile;
	FILE* outputfile;
    open_file(input,name,&inputfile,&outputfile);

    parsingInfo(inputfile, &pair_size, &q_size, &q_size_min, &d_size);

    q_seq = malloc(sizeof(char[(int)q_size]));
    d_seq = malloc(sizeof(char[(int)d_size]));

    parsing(inputfile, outputfile, q_seq, d_seq);

    printf("%s\n\n", q_seq);
    printf("%s\n\n", d_seq);

    m = strlen(q_seq);
    n = strlen(d_seq);

    //Allocate memory for similarity matrix H
    int *H;
    H = calloc((m-1) * (n-1) , sizeof(int));

    //Allocate memory for predecessor matrix P
    int *P;
    P = calloc(m * n,sizeof(int));

    //Calculate similarity matrix
    int maxPos = 0;

    double time0 = gettime();

    for(i = 1; i < n; i++)
   	{
   		for(j = 1; j < m; j++)
  		{
  			Score(i, j, H, P, &maxPos);
  		}
   	}

	printMatrix(H);

	fclose(inputfile);
    fclose(outputfile);
    return 0;
}

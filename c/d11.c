#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255

#define DEBUG

typedef bool *srow;
typedef srow space[];

#define MAXSTAR 4000
typedef struct star {
  int row;
  int col;
} star;

unsigned long long part_1(FILE *input, int mult);

int
main(int argc, char *argv[])
{
  int problem =0, opt;
  while((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
      case 'p':
        problem = atoi(optarg); /* returns 0 if parse fails */
        break;
      default:
      usage:
        printf("Usage: %s -p [1|2] [input file path]\n", argv[0]);
        return 0;
    }
  }
  if (problem == 0) goto usage;
  FILE *input;
  unsigned long long res;
  if ((input = fopen(argv[optind], "r")) == NULL) {
    printf("Failed to open file: %s\n", argv[optind]);
    return 1;
  };
  

  switch(problem) {
    case 1:
      res = part_1(input, 1);
      break;

    case 2:
      res = part_1(input, 1000000);
    break;

    default:
      goto usage;
  }
  printf("Success: %llu\n", res);
}


/* Read input into array of bool arrays, then calculate sum
 * array for empty rows and columns */

#define s_at(s, row, col) (s[row])[col]
int abs(int x) {return (x > 0 ? x : -x); }

unsigned long long part_1(FILE *input, int mult)
{
  char line[MAXLINE]={0};
  int nrows=0, ncols=0;
  fgets(line, MAXLINE, input);
  for(;*(line+ncols) != '\n'; ncols++) {}
  rewind(input);
  srow s[MAXLINE]={0};
  bool *row;


  while(1)
  {
    if(fgets(line, MAXLINE, input)== NULL) break;
    if(*line=='\n') break;

    row= calloc(ncols, sizeof(bool));

    for(int i=0;i<ncols;i++) 
      row[i] = (line[i] == '#');
    s[nrows]=row;
    nrows++;
  }

  int row_empty[nrows];
  for(int i=0; i<nrows;i++)
  {
    bool empty=true;
    for(int j=0; j<ncols;j++)
    {
      if(s_at(s, i, j)) {empty=false; break;}
    }
    if(empty) row_empty[i]=1;
    else row_empty[i]=0;
  }
  int col_empty[ncols];
  for(int j=0; j<ncols;j++) {
    bool empty=true;
    for(int i=0; i<nrows;i++) {
      if(s_at(s,i,j)) {empty=false; break;}
    }
    if(empty) col_empty[j]=1;
    else col_empty[j]=0;
  }
  /* get sum array */
  for(int i=1;i<nrows;i++) 
    row_empty[i] = row_empty[i-1]+row_empty[i];
  for(int j=1;j<ncols;j++) 
    col_empty[j] = col_empty[j-1]+col_empty[j];
  star* stars[MAXSTAR]={0};
  int nstars=0;
  for(int i=0;i<nrows;i++) {
    for(int j=0; j<ncols;j++) {
      if(s_at(s,i,j)) {
        star *s=malloc(sizeof(star));
        s->row=i; s->col=j;
        stars[nstars]=s;
        nstars++;
        // printf("Row %d, col %d star %d\n", i, j, nstars);
      }
    }
  }
  unsigned long long res=0;
  for(int i=0;i<nstars-1;i++) {
    for(int j=i+1; j<nstars;j++) {
      unsigned long long d = abs(stars[i]->row - stars[j]->row)
        + abs(stars[i]->col - stars[j]->col);
      d+=(long long)(mult-1) * abs(row_empty[stars[i]->row] - row_empty[stars[j]->row]);
      d+=(long long)(mult-1) * abs(col_empty[stars[i]->col] - col_empty[stars[j]->col]);
      // printf("partial res for star at %d %d: %llu\n", i, j, d);
      res = res + d;
      // printf("row%d col%d dist %d\n", i, j, d);
    }
  }

  return res;
}

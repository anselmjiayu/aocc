#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255

#define MAXCOL 50

// #define DEBUG

enum object { ASH, ROCK };

typedef enum object *row;

typedef struct pattern
{
  enum object **data;
  int nrows;
  int ncols;
} pattern;

#define p_at(p, i, j) (p->data[i])[j]
typedef unsigned long long ULL;

int solution(FILE *input, int part);

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
      res = solution(input, 1);
      break;

    case 2:
      res = solution(input, 2);
      break;

    default:
      goto usage;
  }
  printf("Success: %llu\n", res);
}

row *mat_initializer()
{
  row *mat = calloc(MAXCOL, sizeof(row));
  for(int i=0;i<MAXCOL;i++) mat[i]=NULL;
  return mat;
}

bool 
parse_pattern(FILE *input, pattern *pp)
{
  char line[MAXLINE]={0};
  if(fgets(line, MAXLINE, input)==NULL) return 0;
  int ncols=0, nrows=0;
  row *mat = mat_initializer();
  for(char *p = line;*p!='\n';p++,ncols++) { }
  do {
    if(*line == '\n') break;
    enum object *r = calloc(ncols, sizeof(enum object));
    for(int i=0;i<ncols;i++) {
      if (line[i] == '.') r[i] = ASH;
      else r[i] = ROCK;
    }
    mat[nrows]=r;
    nrows++;
  } while(fgets(line, MAXLINE, input));
  pattern p = {mat,nrows,ncols};
  *pp = p;
  return 1;
}

int 
search_reflection_center
(ULL *a, int len)
{
  for(int i=0; i<len-1;i++) {
    bool f = true;
    for(int p1=i,p2=i+1; p1 >=0 && p2 < len;p1--,p2++) {
      if(a[p1] != a[p2]) { f=false; break; }
    }
    if(f) return i+1;
  }
  return 0;
}

bool
has_single_bit(ULL x)
{
  return x && !(x & (x - 1));
}

int 
search_reflection_center_v2
(ULL *a, int len)
{
  for(int i=0; i<len-1;i++) {
    bool f = true;
    bool cleared = false;
    for(int p1=i,p2=i+1; p1 >=0 && p2 < len;p1--,p2++) {
      if(a[p1] != a[p2]) {
        if (cleared) { f = false; break; }
        bool smudge = has_single_bit(a[p1] ^ a[p2]);
        if(!smudge) { f=false; break; }
        cleared = true;
      }
    }
    if(f && cleared) return i+1;
  }
  return 0;
}

int
process_pattern
(pattern *p, int part)
{
  ULL *rows = calloc(p->nrows, sizeof(ULL));
  ULL *cols = calloc(p->ncols, sizeof(ULL));
  for(int i=0;i<p->nrows; i++) {
    ULL r = 0ull;
    for(int j=0;j<p->ncols;j++) {
      r<<=1;
      if(p_at(p, i, j)==ROCK) r++;
    }
    rows[i]=r;
  }
  for(int j=0;j<p->ncols;j++) {
    ULL c = 0ull;
    for(int i=0;i<p->nrows;i++) {
      c<<=1;
      if(p_at(p, i, j)==ROCK) c++;
    }
    cols[j]=c;
  }
  int r=0;
  if(part == 1) {
    if((r = search_reflection_center(rows, p->nrows)) > 0) return r*100;
    return search_reflection_center(cols, p->ncols);
  } else {
    if((r = search_reflection_center_v2(rows, p->nrows)) > 0) return r*100;
    return search_reflection_center_v2(cols, p->ncols);
  }
}

int
solution(FILE *input, int part)
{
  int res = 0;
  for(;;) {
    pattern p={0};
    bool has_next = parse_pattern(input, &p);
    if(!has_next) break;
    int lres = process_pattern(&p, part);
#ifdef DEBUG
    printf("Pattern result: %d\n", lres);
#endif
    res += lres;
  }
  return res;
}

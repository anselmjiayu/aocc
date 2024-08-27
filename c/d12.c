#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 600
#define NONE -1l

/* Be careful of turning this on */
// #define DEBUG

enum cond { OPERATIONAL, DAMAGED, UNKNOWN };

typedef struct cond_n {
  struct cond_n *next;
  enum cond cond;
  int index;
} cond_n;

typedef struct spec_n {
  struct spec_n *next;
  bool active;
  int remaining;
  int index;
} spec_n;

typedef struct mat {
  long long **data;
  int ncols;
} mat;

void cond_free(cond_n *n) {
  if(n->next) cond_free(n->next);
  free(n);
}

void spec_free(spec_n *n) {
  if(n->next) spec_free(n->next);
  free(n);
}

int part_1 (FILE *input);
long long part_2 (FILE *input);

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
      res = part_1(input);
      break;

    case 2:
      res = part_2(input);
    break;

    default:
      goto usage;
  }
  printf("Success: %llu\n", res);
}

int parse_cond(char *line, cond_n *head)
{
  cond_n *n=head; cond_n *next;
  int index=0;
  for(char *p=line; *p; p++,index++) {
    if(*p==' ') return index;

    next = malloc(sizeof(cond_n));
    next->next=NULL;
    next->index=index;
    switch(*p) {
      case '?':
      next->cond=UNKNOWN;
      break;
      case '.':
      next->cond=OPERATIONAL;
      break;
      case '#':
      next->cond=DAMAGED;
      break;
    }
    n->next=next;
    n=next;
#ifdef DEBUG
    printf("cond parsed: %d\n", next->cond);
#endif
  }
  return index;
}

char *buf;

void getnext() {
  while(*buf == ' ') buf++;
}

void match(char *str)
{
  int l = strlen(str);
  assert(memcmp((void*)buf, str, l) == 0);
  buf += l;
}

int
getnum()
{
  getnext();
  int res = 0;
  while(isdigit(*buf))
  {
    res = res * 10 + (*buf - '0');
    buf++;
  }
  return res;
}

/* assume that buf points to the first character of
 * condition spec input */
int parse_spec(spec_n *head)
{
  spec_n *n=head; spec_n *next;
  int index=0;
  for(;;index++) {
    if (*buf == '\n'
    || *buf == '\0') return index;

    next = malloc(sizeof(spec_n));
    next->next=NULL;
    next->active=false;
    next->remaining= getnum();
    next->index=index;
    if(*buf==',') match(",");
    n->next=next;
    n=next;
#ifdef DEBUG
    printf("spec parsed: %d\n", next->remaining);
#endif
  }
}

#define dp_at(d, i, j) (*(d.data))[d.ncols*i+j]

long long walk(cond_n *nc, spec_n *ns, mat dp);

long long memoized_walk(cond_n *nc, spec_n *ns, mat dp)
{
  if(nc == NULL || ns == NULL) return walk(nc, ns, dp);
  if (ns->index ==NONE) return walk(nc, ns, dp);
  long long res = dp_at(dp, nc->index, ns->index);
#ifdef DEBUG
  if(res != NONE) 
    printf("result %lld retrieved at ci %d, si %d\n",
           res, nc->index, ns->index);
#endif
  if(res != NONE) return res;
  res = walk(nc, ns, dp);
  dp_at(dp, nc->index, ns->index) = res;
  return res;
}

long long walk(cond_n *nc, spec_n *ns, mat dp)
{
  if(nc == NULL && ns == NULL) return 1;
  if(nc == NULL) {
    if(ns->remaining == 0 && ns->next == NULL) return 1;
    else return 0;
  };
  if(ns == NULL) {
    switch(nc->cond) {
      case DAMAGED: return 0;
      default: return memoized_walk(nc->next, ns, dp);
    }
  }
  switch(nc->cond) {
    case OPERATIONAL:
      if (ns->remaining > 0) {
        if (ns->active) return 0;
        return memoized_walk(nc->next, ns, dp);
      }
      if (ns->remaining == 0)
        return memoized_walk(nc->next, ns->next, dp);
      return memoized_walk(nc->next, ns, dp);

    case DAMAGED:
      if((ns->remaining) == 0) 
        return 0;
      spec_n *next = malloc(sizeof(spec_n));
      next->remaining = ns->remaining-1;
      next->active=true;
      next->index=NONE;
      next->next=ns->next;
      long long res = memoized_walk(nc->next, next, dp);
      free(next);
      return res;
      
    default:
      /* group done, treat as operational */
      if(ns->remaining == 0) return memoized_walk(nc->next, ns->next, dp);
      if(ns->active) {
        spec_n *next = malloc(sizeof(spec_n));
        next->remaining = ns->remaining-1;
        next->active=true;
        next->index=NONE;
        next->next=ns->next;
        long long res = memoized_walk(nc->next, next, dp);
        free(next);
        return res;
      }
      /* try operational */
      long long operational = memoized_walk(nc->next, ns, dp);
      /* try broken */
      spec_n *maybe = malloc(sizeof(spec_n));
      maybe->next = ns->next;
      maybe->active=true;
      maybe->index=NONE;
      maybe->remaining = ns->remaining -1;
      long long broken = memoized_walk(nc->next, maybe, dp);
      /* done with maybe */
      free(maybe);
      return operational + broken;
  }
}


int process_line_v1 (char *line)
{
  cond_n *hc = malloc(sizeof(cond_n));
  spec_n *hs = malloc(sizeof(spec_n));
  int nconds = parse_cond(line, hc);
  /* prepare buf for parse_spec */
  buf = line;
  do {
    buf++;
  } while (*buf != ' ');
  int nspecs = parse_spec(hs);

  /* prepare memoization */
  long long *mem = calloc(nconds*nspecs, sizeof(long long));
  for(int i=0;i<nconds*nspecs;i++) {
    mem[i]=NONE;
  }
  mat dp = {&mem, nspecs};

  int res = memoized_walk(hc->next, hs->next, dp);
  cond_free(hc);
  spec_free(hs);
  free(mem);
  return res;
}

/* This string processing procedure
 * is a huge mess. Probably it could be much
 * cleaner, but the problem itself is messy
 * to begin with... */
long long process_line_v2 (char *line)
{
  cond_n *hc = malloc(sizeof(cond_n));
  spec_n *hs = malloc(sizeof(spec_n));

  buf = line;
  int n_cin = 0;
  for(;*buf!=' '; buf++, n_cin++){ }

  char transform_line[MAXLINE]={0};
  char *p = transform_line;
  for(int i=0; i<5; i++)
  {
    memcpy(p,line, n_cin * sizeof(char));
    p += n_cin;
    *p = '?';
    p++;
  }
  *(p-1) = '\0';
  int nconds = parse_cond(transform_line, hc);
#ifdef DEBUG
  printf("n_cin: %d\n", n_cin);
  printf("conds line: %s\n", transform_line);
#endif
  /* prepare buf for parse_spec */
  buf = line;
  for(;*buf!=' '; buf++){ }
  buf++;
  p = buf;
  int n_sin = 0;
  for(; *p!='\n'; p++, n_sin++) { }
#ifdef DEBUG
  printf("n_sin: %d\n", n_sin);
#endif

  /* copy specs into transform_line
   * 5 times with comma separator */
  p = transform_line;
  for(int i=0; i<5; i++)
  {
    memcpy(p,buf, n_sin * sizeof(char));
    p += n_sin;
    *p = ',';
    p++;
  }
  *(p-1)='\n';
  *p = '\0';
  buf = transform_line;

  int nspecs = parse_spec(hs);
#ifdef DEBUG
  printf("specs line: %s\n", transform_line);
  printf("nconds: %d\n", nconds);
  printf("nspecs: %d\n", nspecs);
  for(cond_n *n=hc->next; n!=NULL; n=n->next)
  {
    printf("c%d:%d ", n->index, n->cond);
  }
  putchar('\n');
  for(spec_n *n=hs->next; n!=NULL; n=n->next)
  {
    printf("s%d:%d ", n->index, n->remaining);
  }
  putchar('\n');
#endif

  /* prepare memoization */
  long long *mem = calloc(nconds*nspecs, sizeof(long long));
  for(int i=0;i<nconds*nspecs;i++) {
    mem[i]=NONE;
  }
  mat dp = {&mem, nspecs};

  long long res = memoized_walk(hc->next, hs->next, dp);
  cond_free(hc);
  spec_free(hs);
  free(mem);
  return res;
}

int part_1 (FILE *input) {
  char line[MAXLINE];
  int res=0;
  while(fgets(line, MAXLINE, input))
  {
    if(*line=='\n') break;
    int lres = process_line_v1(line);
#ifdef DEBUG
    printf("line result: %d\n", lres);
#endif
    res += lres;
  }
  return res;
}

long long part_2 (FILE *input) {
  char line[MAXLINE];
  long long res=0l;
  while(fgets(line, MAXLINE, input))
  {
    if(*line=='\n') break;
    long long lres = process_line_v2(line);
#ifdef DEBUG
#endif
    printf("line result: %lld\n", lres);
    res += lres;
  }
  return res;
}

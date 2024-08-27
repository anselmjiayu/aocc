#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 300 /* This input has a long first line, let's increase line size */

#define DEBUG


typedef unsigned long long ULL;

typedef struct location {
  char *name;
  short l_index;
  short r_index;
  bool done;
} location;

typedef struct location_entry {
  char *name;
  short index;
  struct location_entry *next;
} location_entry;


#define LEN_LOC 3
#define TBL_SZ 50

// let's use the simplest hash method
static short loc_hash(char *name)
{
  short hash=0;
  for(int i=0;i<LEN_LOC;i++) {
    hash += (short)*name;
    name++;
  }
  return hash % TBL_SZ;
}

typedef location_entry *loc_tbl[TBL_SZ];
#define LOCS_SZ 1000
typedef location loc_arr[LOCS_SZ];
typedef struct insertion_result {
  short index;
  bool new;
} insertion_result;

enum direction {LEFT, RIGHT};

ULL part_1(FILE *input);
ULL part_2(FILE *input);

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
  ULL res;
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

volatile char *buf;

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

void
getstr(int len, char *dest)
{
  for(int i=0;i<len;i++) {
    *dest = *buf;
    buf++; dest++;
  }
}

bool check(char *s) { return memcmp((void*)buf, s, strlen(s)) == 0; }

// lookup entry index if exists, create node and return index if not
insertion_result loc_tbl_insert(char *name, loc_tbl lt, loc_arr la, int next_idx)
{
  short hash = loc_hash(name);
  for (location_entry *node = lt[hash]; node!=NULL; node=node->next)
  {
    if(strcmp(name, node->name) == 0) {
      insertion_result res={node->index, false}; return res;
    }
  }
  location_entry *new_node=malloc(sizeof(location_entry));
  new_node->index=next_idx;
  char *name_dup = calloc(LEN_LOC, sizeof(char));
  memcpy(name_dup, name, LEN_LOC*sizeof(char));
  new_node->name=name_dup;
  new_node->next=NULL;
  la[next_idx].name=name_dup;
  insertion_result res={next_idx, true};
  if (lt[hash] == NULL) {
    lt[hash] = new_node; return res;
  } else {
    location_entry *node = lt[hash];
    while (node->next !=NULL) node=node->next;
    node->next = new_node;
    return res;
  }
}

int parse_directions(char *line, enum direction dir_arr[])
{
  buf = line;
  int count = 0;
  for(;; buf++)
  {
    switch(*buf) {
      case 'L':
        dir_arr[count]=LEFT;
        count++;
        break;
      case 'R':
        dir_arr[count]=RIGHT;
        count++;
        break;
      default: goto dirs_done;
    }
  }
dirs_done: return count;
}

/* Takes index of next available spot in array, returns
 * updated available index */
int parse_location(char *line, loc_tbl lt, loc_arr la, int next)
{
  buf = line;
  int idx = next;
  char name[LEN_LOC]={0};
  getstr(LEN_LOC, name); /* read in node name */
  insertion_result inres = loc_tbl_insert(name, lt, la, idx);
  int loc_idx=inres.index;
  if(inres.new) idx++;
  getnext(); match("="); getnext(); match("(");

  getstr(LEN_LOC, name); /* read in left dest */
  inres = loc_tbl_insert(name, lt, la, idx);
  if(inres.new) idx++;
  la[loc_idx].l_index = inres.index;
  match(","); getnext();

  getstr(LEN_LOC, name); /* read in right dest */
  inres = loc_tbl_insert(name, lt, la, idx);
  if(inres.new) idx++;
  la[loc_idx].r_index = inres.index;

  return idx;
}

ULL part_1(FILE *input)
{
  char line[MAXLINE];
  fgets(line, MAXLINE, input);
  enum direction directions[LOCS_SZ]={0};
  int dir_count = parse_directions(line, directions);

  
  fgets(line, MAXLINE, input); /* discard separator line */
  int la_idx = 0;
  loc_tbl lt={0};
  loc_arr la={0};
  while(fgets(line, MAXLINE, input)) {
    if(*line == '\0' || *line == '\n') break; /* break on empty line*/
    la_idx = parse_location(line, lt, la, la_idx);
  }
  /* find start and end nodes */
  int start, end;
  for (int i=0; i<la_idx; i++) {
  if(strncmp(la[i].name,"AAA", LEN_LOC)==0) start=i;
  if(strncmp(la[i].name,"ZZZ", LEN_LOC)==0) end=i;
  }
#ifdef DEBUG
  printf("start: %d, end: %d\n", start, end);
  printf("start: %s %d %d\n", la[start].name, la[start].l_index, la[start].r_index);
  printf("end: %s %d %d\n", la[end].name, la[end].l_index, la[end].r_index);
#endif
  ULL steps=0;
  for(int pos=start; pos!=end;steps++)
  {
    switch(directions[steps%dir_count])
    {
      case LEFT:
      pos = la[pos].l_index;
      break;
      case RIGHT:
      pos = la[pos].r_index;
      break;
    }
  }
  return steps;
}

bool endswith(char *name, char c)
{ return *(name + LEN_LOC - 1) == c; }

ULL gcd(ULL x, ULL y)
{
  while(x != 0) {
    ULL q = y % x;
    y = x;
    x = q;
  }
  return y;
}

ULL arr_lcm(ULL ca[], int count)
{
  ULL acc = ca[0];
  for(int i=1; i<count;i++)
  {
    ULL next = ca[i];
    ULL denom = gcd(acc, next);
    acc = (acc/denom) * (next/denom) * denom;
  }
  return acc;
}


ULL part_2(FILE *input)
{
  char line[MAXLINE];
  fgets(line, MAXLINE, input);
  enum direction directions[LOCS_SZ]={0};
  int dir_count = parse_directions(line, directions);

  
  fgets(line, MAXLINE, input); /* discard separator line */
  int la_idx = 0;
  loc_tbl lt={0};
  loc_arr la={0};
  while(fgets(line, MAXLINE, input)) {
    if(*line == '\0' || *line == '\n') break; /* break on empty line*/
    la_idx = parse_location(line, lt, la, la_idx);
  }
  /* find start and end nodes */
  int pos_count=0; /* amnt of simul positions */
  for(int i=0; i<la_idx;i++)
  {if (endswith(la[i].name, 'A')) pos_count++;}

  int pos[pos_count];
  for(int i=0, j=0; i<la_idx; i++) {
    la[i].done = endswith(la[i].name, 'Z');
    if(endswith(la[i].name, 'A')) {
      pos[j] = i;
      j++;
    }
  }

  ULL cycles[pos_count];
  for(int i = 0; i< pos_count; i++)
  {
    printf("Calculating path %i starting at %s: \n",i, la[pos[i]].name);
    int steps=0;
    int pos_fst = pos[i];
    bool firstpass = true;

    for(int p=pos_fst;;steps++)
    {
      switch(directions[steps%dir_count])
      {
        case LEFT:
          p=la[p].l_index;
          break;
        case RIGHT:
          p=la[p].r_index;
          break;
      }
      if(p == pos_fst)
      {
        printf("Returned at %d steps\n", steps);
        break;
      }
      if(la[p].done)
      {
        printf("Marked as done at %d steps at %s\n", steps+1,
               la[p].name);
        /* There is an unspecified assumption in the problem 
         * input, that the cycles do not have remainders, i.e. the
         * second time a position reaches accept condition equals
         * the first time times 2. Let's exploit this observed
         * pattern and cut corners in our algorithm. */
        cycles[i] = steps+1;
        break;

        /* Unused */
        if(firstpass) firstpass=false;
        else {
          printf("Path %i loops at end\n", i);
          break;
        }
      }
    }

  }
  printf("Cycles: \n");
  for(int i=0; i<pos_count;i++)
    printf("%llu ", cycles[i]);
  putchar('\n');
  /* See above comment */
  return arr_lcm(cycles, pos_count);

}

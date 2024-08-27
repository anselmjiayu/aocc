#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255
typedef unsigned long long ULL;

typedef struct {
  ULL dest;
  ULL src;
  ULL len;
} conv_t;

typedef struct range_t {
  ULL start;
  ULL end;
  bool done;
} range_t;

#define the_empty_range NULL

typedef struct range_conv_t {
  range_t *first;
  range_t *second;
  range_t *third;
} range_conv_t;

#define MAXARR 99

#define MAX_RANGES 3000

// #define DEBUG

typedef ULL *results[MAXARR];

typedef conv_t *conversions[MAXARR];

typedef range_t *ranges[MAX_RANGES];

void process_conversion_grp
(results *prev, results *next, conversions *table);

// read in seed vals and return amnt of seeds
int read_initial_vals
(results *vals1, results *vals2, FILE* file);

int read_conversion_grp
(conversions *table, FILE* file);

int read_initial_pairs
(ranges *table,FILE* file);

// merge table of ranges with conversions to
// produce a new table
void merge_conversion_grp
(ranges *rtbl, ranges* merges, conversions *ctbl);

range_conv_t *merge
(range_t *input, conv_t *conv);

ULL get_res(results *vals);
ULL get_ranges_res(ranges *rtable);

void reset_ranges(ranges *rtable);


int
main(int argc, char *argv[])
{
  int problem =0, opt;
  while((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
      case 'p':
        problem = atoi(optarg);
        break;
      default:
      usage:
        printf("Usage: %s -p [1|2] [input file path]\n", argv[0]);
        return 0;
    }
  }
  if (problem == 0) goto usage;
  FILE *input;
  int res;
  if ((input = fopen(argv[optind], "r")) == NULL) {
    printf("Failed to open file: %s\n", argv[optind]);
    return 1;
  };
  
  results v1 = {0}, v2={0};
  conversions ctbl={0}; 
  results *prev_v = &v1, *next_v = &v2;
  ranges rtbl1 ={0}, rtbl2={0};
  ranges *prev_rtbl = &rtbl1, *next_rtbl = &rtbl2;
#define NUM_TBLS 7

  switch(problem) {
    case 1:
      read_initial_vals(prev_v, next_v, input);
      for(int i=0; i<NUM_TBLS; i++)
      {
        read_conversion_grp(&ctbl, input);
        process_conversion_grp(prev_v, next_v, &ctbl);
        results *t = prev_v;
        prev_v=next_v;
        next_v=t;
      }
      res = get_res(prev_v);
      break;

    case 2:
    read_initial_pairs(prev_rtbl, input);
      for(int i=0; i<NUM_TBLS; i++)
      {
        read_conversion_grp(&ctbl, input);
        merge_conversion_grp(prev_rtbl, next_rtbl, &ctbl);
        ranges *t = next_rtbl;
        next_rtbl=prev_rtbl;
        prev_rtbl=t;
      }
      res = get_ranges_res(prev_rtbl);
    break;

    default:
      printf("Usage: %s -p [1|2] [input file path]\n", argv[0]);
      return 0;
  }
  printf("Success: %i\n", res);
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

bool check(char *s) { return memcmp((void*)buf, s, strlen(s)) == 0; }

// discard whitespace then match a number
ULL
getnum()
{
  getnext();
  ULL res = 0;
  while(isdigit(*buf))
  {
    res = res * 10 + (*buf - '0');
    buf++;
  }
  return res;
}

bool check_if_possible(ULL *input,
                       ULL *output,
                       conv_t *range)
{
  if((*range).src + (*range).len <= *input) return false;
  if ((*range).src > *input) return false;
  *output = (*range).dest + *input - (*range).src;
  return true;
}

// for debugging use
void print_results(results *vals)
{
  int i=0;
  putchar('[');
  while(*((*vals)+i) !=NULL) {
    printf("%llu ", **((*vals)+i));
    i++;
  }
  putchar(']');
  return;
}

void print_conversions(conversions *table)
{
  int i=0;
  printf("[\n");
  while(*((*table)+i) != NULL)
  {
    printf("\td: %llu, s: %llu, l: %llu\n", (**((*table)+i)).dest, 
           (**((*table)+i)).src,(**((*table)+i)).len);
    i++;
  }
  printf("]\n");
}

// converts values in prev and stores them in next,
// using rules in table
void process_conversion_grp
(results *prev, results *next, conversions *table)
{
  int i=0, j;
  bool f;
  conv_t *conv;
#ifdef DEBUG
  printf("Prev result: ");
  print_results(prev);
  printf("\n");
  printf("Conversions: ");
  print_conversions(table);
#endif
  
loopconv:
  if (*((*prev)+i) == NULL) return;
  j=0;
  f=false;
conv_inner:
  conv = (*table)[j];
  if(conv == NULL) goto afterconv_inner;
  if(check_if_possible(*((*prev)+i), *((*next)+i), conv))
  {
    f = true;
    goto afterconv_inner;
  }
  j++;
  goto conv_inner;

afterconv_inner:
  if(!f) **((*next)+i)= **((*prev)+i);
  i++;
  goto loopconv;
}

int read_initial_vals
(results *vals1, results *vals2, FILE* file)
{
  char line[MAXLINE] = {0};
  int i=0;
  fgets(line, MAXLINE, file);
  buf=line;
  match("seeds:");
  while(*buf != '\n' && *buf != '\0') {
    ULL *t1 = malloc(sizeof(*t1));
    ULL *t2 = malloc(sizeof(*t2));
    *t1 = getnum();
    *t2 = *t1;
    (*vals1)[i] = t1;
    (*vals2)[i] = t2;
    i++;
  }
  // discard separator line
  fgets(line, MAXLINE, file);
  return i;
}

int read_conversion_grp
(conversions *table, FILE* file)
{
  char line[MAXLINE] = {0};
  int i=0;
  // discard header line
  fgets(line, MAXLINE, file);
  while(1)
  {
  fgets(line, MAXLINE, file);
    // end of conv group
    if(!isdigit(*line)) {
      (*table)[i]=NULL;
      break;
    };
    buf = line;
    conv_t *conv = malloc(sizeof *conv);
    (*conv).dest = getnum();
    (*conv).src = getnum();
    (*conv).len = getnum();
    (*table)[i] = conv;
    i++;
  }
  return i;
}

ULL get_res(results *vals)
{
  int i=0; ULL res = 10E15;
  while(*(*vals+i) != NULL)
  {
    res = MIN(res, **(*vals+i));
    i++;
  }
  return res;
}

// void reduce_rtbl
// (ranges *rtbl, void (*reducer)(range_t *range, void* acc),
//  void* init)
// {
//   int i=0; range_t *ptr; void* acc=init;
//   while(1)
//   {
//     ptr = *((*rtbl)+i);
//     if (ptr == NULL) break;
//     (*reducer)(ptr, acc);
//     i++;
//   }
// }

// void reduce
// (void* iterable, void* (*iterator)(void*),
//  void (*reducer)(void* element, void* acc),
//  void* init, bool (*done)(void*))
// {
//   void* ptr; void* acc=init;
//   while(1)
//   {
//     ptr = (*iterator)(iterable);
//     if ((*done)(iterable)) break;
//     (*reducer)(ptr, acc);
//   }
// }

// static range_t *rtbl_iterator(ranges *rtbl) { return *((*rtbl)+1); }
// static bool rtbl_done(ranges *rtbl) { return **rtbl == NULL; }

// // __requires empty sentinel at 0th index__
// void reduce_rtbl
// (ranges *rtbl, void (*reducer)(range_t *range, void* acc),
//  void* init)
// {
//   reduce(rtbl, (void* (*)(void*))rtbl_iterator, 
//          (void (*)(void*, void*))reducer, 
//          init, (bool (*)(void*))rtbl_done);
// };

// static void reset_reducer(range_t *r, void* _ignore) { (*r).done = false; }
// void reset_ranges(ranges *rtable) { reduce_rtbl(rtable, reset_reducer, NULL); }

// static void min_reducer(range_t *r, ULL* acc)
// { *acc = MIN(*acc, (*r).start); }

ULL get_ranges_res(ranges *rtable) {
  ULL init = 10E15; ULL *res = &init;
  for(int i=0; (*rtable)[i]!=NULL; i++)
  {
    *res=MIN(*res, (*rtable)[i]->start);
  }
  return *res;
}

range_t *new_range(ULL start, ULL end)
{
  ULL *sp = malloc(sizeof(*sp)), *ep = malloc(sizeof(*ep));
  range_t *rp = malloc(sizeof(*rp));
  (*rp).start=start;
  (*rp).end=end;
  (*rp).done=false;
  return rp;
}

range_conv_t *range_conv_init()
{
  range_conv_t *rpp = malloc(sizeof(*rpp));
  rpp->first = the_empty_range;
  rpp->second = the_empty_range;
  rpp->third = the_empty_range;
  return rpp;
}

int read_initial_pairs
(ranges *table,FILE* file)
{
  char line[MAXLINE] = {0};
  int i=0;
  fgets(line, MAXLINE, file);
  buf=line;
  match("seeds:");
  while(*buf != '\n' && *buf != '\0') {
    ULL s=getnum(); ULL r=getnum();
    range_t *rp = new_range(s, s+r);
    (*table)[i]=rp;
    i++;
  }
  (*table)[i]=NULL;
  // discard separator line
  fgets(line, MAXLINE, file);
  return i;
}


range_conv_t *merge
(range_t *input, conv_t *conv)
{
  ULL conv_start = conv->src, conv_end=(conv->src)+(conv->len),
  in_start = input->start, in_end = input->end;
  long long delta = conv->dest - conv->src;
  range_conv_t *ret = range_conv_init();
  if (in_start < conv_start)
  {
    ret->first = new_range(in_start, MIN(in_end, conv_start));
  } 
  if (in_end > conv_start && in_start < conv_end)
  {
    ret->second = new_range(MAX(in_start, conv_start), MIN(in_end, conv_end));
    ret->second->start += delta;
    ret->second->end += delta;
    ret->second->done = true;
  }
  if (in_end > conv_end)
  {
    ret->third = new_range(MAX(in_start, conv_end), in_end);
  }
  // free(input);
  return ret;
}

void print_ranges(ranges* rp)
{
  printf("[");
  for(int i=0; (*rp)[i] !=NULL; i++)
  {
    if(i>0) printf(",");
      range_t *r = (*rp)[i];
    printf("%llu %llu %b",r->start, r->end, r->done );
  }
  printf("] ");
}

void merge_conversion_grp
(ranges *rtbl, ranges *merges, conversions *ctbl)
{
  ranges buffer={0};
  range_t *r_pos; conv_t *c_pos;
  for(int i=0; (*ctbl)[i] !=NULL; i++)
  {
    c_pos=(*ctbl)[i];
    int k=0;
    
    // convert rtbl and store results in buffer
    for(int j=0; (*rtbl)[j]!=NULL; j++)
    {
      r_pos=(*rtbl)[j];
      if(r_pos->done)
      {
        buffer[k]=r_pos;
        k++;
      } else {
        range_conv_t *merge_res=merge(r_pos, c_pos);
        if(merge_res->first != the_empty_range) {
        buffer[k] = merge_res->first;
          k++;
        }
        if(merge_res->second != the_empty_range) {
          buffer[k] = merge_res->second;
          k++;
        }
        if(merge_res->third != the_empty_range) {
          buffer[k] = merge_res->third;
          k++;
        }
        free(merge_res);
      }
    }
    // write buffer into rtbl
    assert(k<MAX_RANGES);
    buffer[k]=NULL;
#ifdef DEBUG
    printf("ranges before: ");
    print_ranges(rtbl);
    printf("\nranges after: ");
    print_ranges(&buffer);
    printf("\n\n");
#endif
    for(int i=0; i<k; i++)
    {
      (*rtbl)[i]=buffer[i];
    }
    (*rtbl)[k]=NULL;

  }
#ifdef DEBUG
  printf("conversion group done \n");
#endif

  // write rtbl into res
  for(int i=0; (*rtbl)[i] !=NULL; i++){

    (*rtbl)[i]->done=false;
    (*merges)[i] = (*rtbl)[i];
  }
  

}


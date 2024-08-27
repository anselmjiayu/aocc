#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255

// #define DEBUG 1

typedef char line_t[MAXLINE];

typedef struct {
  line_t *prev;
  line_t *curr;
  line_t *next;
} input_st;

struct gear_t {
  int line;
  // amount of tokens connected
  short connect;
};


struct num_token_t {
  bool success;
  short start;
  short end;
  int res;
};

struct link_t {
  struct link_t *next;
  struct num_token_t *data;
};

void copy_line(line_t *dest, line_t *src)
{
  memcpy(dest, src, sizeof(line_t));
}

int process_line_v1(input_st *data);

// given a list head and a line, scan it for tokens and produce a list
void add_tokens_to_list(struct link_t *sentinel, line_t *line);
// given three linked lists and a line, produce sum of gear ratios in the line
int scan_line_for_gear(struct link_t *prev,struct link_t *curr,
                       struct link_t *next, line_t *line);

static bool first_line_p;
static bool last_line_p;

int
main(int argc, char *argv[])
{
  int problem=-1, opt;
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
  if (problem == -1) goto usage;
  FILE *input;
  if ((input = fopen(argv[optind], "r")) == NULL) {
    printf("Failed to open file: %s\n", argv[optind]);
    return 1;
  };

  static line_t line={0};
  static line_t prev = {0}, curr={0}, next={0};
  volatile int linum = -1;
  int res = 0;
  input_st _data = {
    &prev, &curr, &next
  };
  input_st *data=&_data;
  first_line_p = true;
  last_line_p = false;

  
  switch(problem) {
    case 1:
      while(fgets(line, MAXLINE, input) != NULL) {
        copy_line(data->prev, data->curr);
        copy_line(data->curr, data->next);

        // deal with potential empty last line
        if(*line == '\n') {
          break;
        } else {
          // read in line as next line in data structure
          copy_line(data->next, &line);
        }
        if (linum >= 0) {
          res += process_line_v1(data);
          first_line_p = false;
        }
        linum++;
      }
      last_line_p = true;
      // process last line
      res += process_line_v1(data);
      break;
    case 2:
      linum = 0;
        // initialize token table
      struct link_t *token_table[MAXLINE] ={0}; 
      while(fgets(line, MAXLINE, input) != NULL) {
        // initialize token list
        struct link_t *head=malloc(sizeof(struct link_t));
        head->data=NULL;
        head->next=NULL;

        // deal with potential empty last line
        if(*line == '\n') {
          break;
        } else {
          add_tokens_to_list(head, &line);
          token_table[linum] = head;
        }
        linum++;
      }
      // read file a second time to produce results
      if(fseek(input, 0L, SEEK_SET) == -1) {
        printf("Error: file rewind failed\n");
        exit(-1);
      }
      int total_lines = linum;
      linum = 0;
      while(fgets(line, MAXLINE, input) != NULL) {
        if(linum > 0 || linum < total_lines) {
          res += scan_line_for_gear(token_table[linum-1], token_table[linum], 
                             token_table[linum+1], &line);
        }
        linum++;
      }
      break;


    default:
      goto usage;
  }
  printf("Success: %i\n", res);
}


void get_next_number(int start, int line_len, line_t *line, struct num_token_t *token);

bool valid_sym_p(char c) {
  return c != '.';
}

int process_line_v1(input_st *data)
{
  // check if there is a prev and a next line
  bool has_prev = !first_line_p;
  bool has_next = !last_line_p;
  struct num_token_t token={0};
  const int line_len = strlen(*data->curr)-1;
  int line_sum = 0;
  int ptr = 0;
  bool valid;
  while(1) {
    valid = false;
    get_next_number(ptr, line_len, data->curr, &token);
    ptr = token.end;
    if(!token.success) break;
    if(token.start > 0 && valid_sym_p((*data->curr)[token.start-1])) valid = true;
    if(token.end < line_len-1 && valid_sym_p((*data->curr)[token.end])) valid = true;
    for (int i=MAX(0, token.start-1); i<=MIN(token.end, line_len-1); i++) {
      if (has_next && valid_sym_p((*data->next)[i])) {
        valid=true;
        continue;
      }
      if (has_prev && valid_sym_p((*data->prev)[i])) {
        valid=true;
        continue;
      }
    }
    if(valid) line_sum += token.res;
  }

  return line_sum;
}

void get_next_number(int start, int line_len, line_t *line, struct num_token_t *token)
{
  int p1=start, p2=start, res=0;
  while(!isdigit((*line)[p1])) {
    if (p1 >= line_len-1) {
      token->success = false;
      return;
    }
    p1 = p1+1;
  };
  p2=p1;
  while(isdigit((*line)[p2])) {
    res=res*10+((*line)[p2]-'0');
    p2++;
  }
  token->start=p1;
  token->end=p2;
  token->res=res;
  token->success = true;
}

void add_tokens_to_list(struct link_t *sentinel, line_t *line) 
{
  struct link_t* tail=sentinel;
  int p1=0, p2=0, res;
  char c;
  while(1) {
    res=0;
    p1=p2;
    while(!isdigit((*line)[p1])) {
      c = (*line)[p1];
      // end of line
      if (c == '\n' || c == '\0') break;
      p1++;
    }
    c = (*line)[p1];
    // end of line
    if (c == '\n' || c == '\0') break;
    p2=p1;
    while(isdigit((*line)[p2])) {
      res=res*10+((*line)[p2]-'0');
      p2++;
    }
    struct num_token_t* token = (struct num_token_t*) malloc(sizeof(struct num_token_t));
    token->start=p1;
    token->end=p2;
    token->res=res;
    token->success = true;

    struct link_t* next = (struct link_t*) malloc(sizeof(struct link_t));
    next->data=token;
    next->next=NULL;

    tail->next=next;
    tail=next;
  }
}

int scan_line_for_gear(struct link_t *prev,struct link_t *curr,
                       struct link_t *next, line_t *line)
{
  int p=0; char c;
  int res=0, count, acc; struct link_t* cl;
  while(1) {
    c=(*line)[p];
    if (c == '\n' || c == '\0') break;
    if(c == '*') {
      // count numbers
      count=0; acc=1;
      // first line
      cl = prev->next;
#ifdef DEBUG
    printf("links: %p %p %p\n", prev, curr, next);
#endif
      while (cl!=NULL) {
#ifdef DEBUG
    printf("cl: start %d,end %d\n", cl->data->start, cl->data->end);
#endif
        if (cl->data->end < p) {
          cl = cl->next;
          continue;
        }
        else if (cl->data->start <= p+1) {
          count++;
          acc *= cl->data->res;
          cl=cl->next;
        }
        else break;
      }
      // current line
      cl = curr->next;
      while (cl!=NULL) {
        if (cl->data->end < p) {
          cl=cl->next;
          continue;
        }
        else if (cl->data->start == p+1 || cl->data->end == p) {
          count++;
          acc *= cl->data->res;
          cl=cl->next;
        }
        else break;
      }
      // next line
      cl = next->next;
      while (cl!=NULL) {
        if (cl->data->end < p) {
          cl=cl->next;
          continue;
        }
        else if (cl->data->start <= p+1) {
          count++;
          acc *= cl->data->res;
          cl=cl->next;
        }
        else break;
      }
      if (count==2)
        res += acc;
    #ifdef DEBUG
    printf("acc: %d, count: %d\n", acc, count);
    #endif

    }
    p++;
  }
  return res;
}

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255

struct _link_t {
  int data;
  struct _link_t *next;
};

typedef struct _link_t link_t;

typedef char line_t[MAXLINE];

// construct list from scanning winners
// and check rest to calculate sum
int scan_line_v1(line_t *line);

// simply check number of matching cards
int scan_line_v2(line_t *line);

int
main(int argc, char *argv[])
{
  int problem, opt;
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
  char line[MAXLINE] = {0};
  int counts[MAXLINE] = {0};
  int res = 0;
  int line_total=0;
  int curr_line=0;
  int nnext;
  switch(problem) {
    case 1:
      while(fgets(line, MAXLINE, input) != NULL) {
        res += scan_line_v1(&line);
      }
      break;

    case 2:
    // first go through file to get number of lines
      while(fgets(line, MAXLINE, input) != NULL) {
        if(*line == '\n' || *line == '\0') break;
        line_total++;
      }
    rewind(input);

    // populate array with card count
    
      while(fgets(line, MAXLINE, input) != NULL) {
        if (curr_line >= line_total) break;
        counts[curr_line]+=1; //initial copy
        nnext=scan_line_v2(&line);
        for(int i=curr_line+1; i<MIN(line_total, curr_line+nnext+1); i++)
        {
          counts[i]+=counts[curr_line];
        }
        curr_line ++;
      }
    
    for(int i=0; i<line_total; i++) {
        // printf("line %d: %d\n", i, counts[i]);
        res += counts[i];
      }
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

// check if a number is in the list
bool check_link(link_t *head, int data);
int linesum(int count);

int scan_line_v1(line_t *line)
{
  return linesum(scan_line_v2(line));
}


int scan_line_v2(line_t *line)
{
  buf = *line;
  if (*buf == '\n' || *buf == '\0') return 0; //empty line
  match("Card");
  getnext();
  (void)getnum();
  match(":");
  link_t *head = malloc(sizeof(link_t));
  head->data=-1;
  head->next=NULL;

  link_t *cl=head;

  // construct table
  while(!check(" |"))
  {
    int n = getnum();
    link_t *next = malloc(sizeof(link_t));
    next->data=n;
    next->next=NULL;
    cl->next=next;
    cl=next;
  }

  match(" |");
  int count = 0;

  while(*buf != '\n' && *buf != '\0') {
    int n = getnum();
    if(check_link(head, n))
      count++;
  }
  return count;

}


int linesum(int count)
{
  if (count <=0) return 0;
  return 1UL << (count-1);
}

bool check_link(link_t *head, int data)
{
  link_t *cl = head->next;
  while(cl!=NULL)
  {
    if(cl->data == data) return true;
    cl=cl->next;
  }
  return false;
}

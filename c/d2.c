#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255


int possible_id_v1(char line[]);
int set_power_v2(char line[]);

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
        printf("Usage: %s -p [1|2] [input file path]\n", argv[0]);
        return 0;
    }
  }
  FILE *input;
  if ((input = fopen(argv[optind], "r")) == NULL) {
    printf("Failed to open file: %s\n", argv[optind]);
    return 1;
  };
  char line[MAXLINE] = {0};
  int res = 0;
  switch(problem) {
    case 1:
      while(fgets(line, MAXLINE, input) != NULL) {
        res += possible_id_v1(line);
      }
      break;
    case 2:
      while(fgets(line, MAXLINE, input) != NULL) {
        res += set_power_v2(line);
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

#define RED 0
#define GREEN 1
#define BLUE 2
typedef int counter_t[3];

bool check(char *s) { return memcmp((void*)buf, s, strlen(s)) == 0; }
counter_t counter_buf ={0};
counter_t *counter = &counter_buf;

// return false if has next value, true if no next value
bool updateCount(counter_t *counter)
{
  int c = getnum();
  getnext();
  if(check("red")) {
    *counter[RED] = MAX(*counter[RED], c);
    buf +=3;
  }
  else if(check("green"))
  {
    *counter[GREEN] = MAX(*counter[GREEN], c);
    buf +=5;
  }
  else if(check("blue"))
  {
    *counter[BLUE] = MAX(*counter[BLUE], c);
    buf +=4;
  }
  if (check(","))
  {
    match(",");
    return false;
  }
  else if (check(";")) {
    match(";");
  }
  return true;
}

bool check_counter(counter_t *counter)
{
  return *counter[RED] <= 12 && *counter[GREEN] <=13 && *counter[BLUE] <=14;
}

int counter_power(counter_t *counter)
{
  return (*counter[RED])*(*counter[GREEN])*(*counter[BLUE]);
}

void init_counter(counter_t *counter)
{
  for(int i=0;i<3;i++) *counter[i]=0;
}

int
possible_id_v1(char line[]) 
{
  buf = line;
  // ignore empty line
  getnext();
  if(*buf == '\n') return 0;
  match("Game");
  int id = getnum();
  match(":");
  init_counter(counter);
  while(*buf != '\n') {
    updateCount(counter);
  }
  bool valid_p = check_counter(counter);
  if (valid_p) return id;
  return 0;
}

int
set_power_v2(char line[])
{
  buf = line;
  getnext();
  if(*buf == '\n') return 0;
  match("Game");
  int id = getnum();
  match(":");
  init_counter(counter);
  while(*buf != '\n') {
    updateCount(counter);
  }
  int set_power = counter_power(counter);
  return set_power;
}

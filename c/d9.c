#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255

#define DEBUG


typedef long long LL;



LL part_1(FILE *input);
LL part_2(FILE *input);

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
  int res;
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

bool check(char *s) { return memcmp((void*)buf, s, strlen(s)) == 0; }

LL extrapolate(LL arr[], int len) {
  LL res=0;
  for(int i=0; i<len; i++)
  {
    res += arr[len-1-i];
    bool done=true;
    for(int j=0; j<len-i-1; j++)
    {
      if ((arr[j] = arr[j+1] - arr[j]) != 0) done = false;
    }
    if(done) break;
  }
  return res;
}


LL extrapolate_back(LL arr[], int len) {
  LL res=0;
  for(int i=0; i<len; i++)
  {
    if(i%2 == 0) res += arr[0];
    else res -= arr[0];
    bool done=true;
    for(int j=0; j<len-i-1; j++)
    {
      if ((arr[j] = arr[j+1] - arr[j]) != 0) done = false;
    }
    if(done) break;
  }
  return res;
}


LL part_1(FILE *input)
{
  LL res = 0;
  char line[MAXLINE]={0};
  LL arr[MAXLINE]={0};

  int linum=0;
  while(fgets(line, MAXLINE, input)) {
    if(*line == '\n' || *line == '\0') break;
    buf = line;
    int linlen=0;
    while(*buf !='\n')
    {
      getnext();
      bool negative= false;
      if(check("-")) {
        negative=true;
        match("-");
      }
      LL n = getnum();
      if (negative) arr[linlen] = -n;
      else arr[linlen]=n;
      linlen++;
    }
    LL linres = extrapolate(arr, linlen);
    printf("Line res for line %d: %lld\n",
           linum, linres);
    res += linres;
    linum++;
  }
  return res;
}

LL part_2(FILE *input)
{
  LL res = 0;
  char line[MAXLINE]={0};
  LL arr[MAXLINE]={0};

  int linum=0;
  while(fgets(line, MAXLINE, input)) {
    if(*line == '\n' || *line == '\0') break;
    buf = line;
    int linlen=0;
    while(*buf !='\n')
    {
      getnext();
      bool negative= false;
      if(check("-")) {
        negative=true;
        match("-");
      }
      LL n = getnum();
      if (negative) arr[linlen] = -n;
      else arr[linlen]=n;
      linlen++;
    }
    LL linres = extrapolate_back(arr, linlen);
    printf("Line res for line %d: %lld\n",
           linum, linres);
    res += linres;
    linum++;
  }
  return res;
}

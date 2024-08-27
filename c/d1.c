#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#define MAXLINE 255

int calibval_v1(char *line);
int calibval_v2(char *line);

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
        res += calibval_v1(line);
      }
      break;
    case 2:
      while(fgets(line, MAXLINE, input) != NULL) {
        res += calibval_v2(line);
      }
      break;
    default:
      printf("Usage: %s -p [1|2] [input file path]\n", argv[0]);
      return 0;
  }
  printf("Success: %i\n", res);
}

int
calibval_v1(char line[])
{
  int first, curr;
  unsigned char flag = 'f';
  do {
    if ((int)*line == '\n') break;
    if (isdigit((int)*line))
    {
      curr = (int)*line - '0';
      if (flag == 'f') {
        first = curr;
        flag = 't';
      }
    }
  } while (line++ != NULL);
  if (flag == 'f') return 0;
#ifdef DEBUG
  printf("first: %i, last:%i\n", first, curr);
#endif
  return first*10 + curr;
}


bool checkword_pred(const char* ptr, const char* word)
{
  int l = strlen(word);
  return memcmp(ptr, word, l) == 0;
}

int
calibval_v2(char line[])
{
  int first, curr=0;
  unsigned char flag = 'f';
  do {
    if ((int)*line == '\n') break;
    if (isdigit((int)*line))
    {
      curr = (int)*line - '0';
    }
    else if (isalpha((int)*line)) {
      switch(*line) {
        case 'o':
          if (checkword_pred(line, "one"))
            curr =1;
          break;
        case 't':
          if (checkword_pred(line, "two"))
            curr=2;
          else if (checkword_pred(line, "three")) curr=3;
          break;
        case 'f':
          if (checkword_pred(line, "four")) curr=4;
          else if (checkword_pred(line, "five")) curr=5;
          break;
        case 's':
          if (checkword_pred(line, "six")) curr=6;
          else if (checkword_pred(line, "seven")) curr=7;
          break;
        case 'e':
          if (checkword_pred(line, "eight")) curr=8;
          break;
        case 'n':
          if (checkword_pred(line, "nine")) curr=9;
          break;
        default: break;
      }
    }

    if (flag == 'f' && curr != 0) {
      first = curr;
      flag = 't';
    }
  } while (line++ != NULL);
  if (flag == 'f') return 0;
#ifdef DEBUG
  printf("first: %i, last:%i\n", first, curr);
#endif
  return first*10 + curr;
}

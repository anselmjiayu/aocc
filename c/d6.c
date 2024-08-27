#include <math.h>
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

int process_p1(FILE* input, int times[], int distances[]);
int process_p2(FILE* input);

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
  

  int times[5]={0}, distances[5]={0};
  switch(problem) {
    case 1:
      res = process_p1(input, times, distances);
      break;

    case 2:
      res=process_p2(input);
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
unsigned long long
getlongnum()
{
  unsigned long long res = 0;
  while(*buf != '\n' && *buf != '\0')
  {
    getnext();
    res = res * 10 + (*buf - '0');
    buf++;
  }
  return res;

}

bool check(char *s) { return memcmp((void*)buf, s, strlen(s)) == 0; }

double SQRT_E=1e-6;

double fabs(double x) {return x > 0 ? x : -x; }
double get_better_estimate(double guess, double x)
{
  return (guess + (x/guess))/2;
}

double sqrt(double x)
{
  double guess=(double)1;
  while(fabs(guess-(x/guess)) > SQRT_E)
  {
    guess=get_better_estimate(guess, x);
  }
  return guess;
}
struct q_sols {
  double x1;
  double x2;
};

int solve_viable(unsigned long long time, unsigned long long target, struct q_sols *solutions)
{
  // (x)(k-x) > t => -x^2 +kx-t>0 => -k +- sqrt(k^2-4t)/-2
  double cv = (double)time*(double)time-(double)4*(double)target;
  if (cv<0) return -1;
  double cvr = sqrt(cv);
  double s1 = (time+cvr)/2;
  double s2 = (time-cvr)/2;
  solutions->x1=s1;
  solutions->x2=s2;
  return 0;
}

unsigned long long amnt_line(unsigned long long time, unsigned long long target)
{
  struct q_sols s={0};
#ifdef DEBUG
  printf("time: %llu, dist: %llu\n", time, target);
#endif
  assert(solve_viable(time, target, &s) != -1);
  double x1=s.x1, x2=s.x2;
  unsigned long long min,max;
  if(x1 < x2) {
    min = ceil(x1);
    max = floor(x2);
  } else {
    min = ceil(x2);
    max = floor(x1);
  }
#ifdef DEBUG
  printf("min: %llu, max: %llu\n", min, max);
#endif
  if (min*(time-min)==target) min++;
  if (max*(time-max)==target) max--;
  return max-min+1;
}

int process_p1(FILE* input, int times[], int distances[])
{
  char line[MAXLINE];
  fgets(line, MAXLINE, input);
  buf = line;
  match("Time:");
  for(int i=0; *buf != '\n' && *buf != '\0'; i++)
  {
    times[i] = getnum();
  }
  fgets(line, MAXLINE, input);
  buf = line;
  match("Distance:");
  for(int i=0; *buf != '\n' && *buf != '\0'; i++)
  {
    distances[i] = getnum();
  }
  long res=1;
  for(int i=0; times[i]!=0; i++)
  {
     int r = amnt_line(times[i], distances[i]);
    // printf("%d: %d, ", i, r);
    res *=r;
  }
  return res;
}

int process_p2(FILE* input)
{
  SQRT_E=0.5;
  char line[MAXLINE];
  fgets(line, MAXLINE, input);
  buf = line;
  match("Time:");
  unsigned long long time = getlongnum();
  fgets(line, MAXLINE, input);
  buf = line;
  match("Distance:");
  unsigned long long dist = getlongnum();

  unsigned long long res=amnt_line(time
                                  , dist);
  return res;
}

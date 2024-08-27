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


typedef unsigned long long ULL;

enum hand_type {
  FIVE_OF_KIND = 0,
  FOUR_OF_KIND = 1,
  FULL_HOUSE = 2,
  THREE_OF_KIND = 3,
  TWO_PAIR = 4,
  ONE_PAIR = 5,
  HIGH_CARD = 6,
};

#define LEN_HAND 5
typedef char hand[LEN_HAND];

typedef struct hand_entry {
  hand literal;
  enum hand_type type;
  int bid;
} hand_entry;

// static const char card_order[]= {'A','K','Q','J','T','9','8','7','6','5','4','3','2'};
static const char* card_order = "AKQJT98765432";
static const char* card_order_v2 = "AKQT98765432J";


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

void bubblesort(short (*arr)[LEN_HAND])
{
  for(int i=0; i<LEN_HAND; i++)
  {
    for(int j=i+1; j<LEN_HAND; j++)
    {
      if((*arr)[i]<(*arr)[j])
      {
        short t=(*arr)[i];
        (*arr)[i]=(*arr)[j];
        (*arr)[j]=t;
      }
    }
  }
}

enum hand_type get_type_from_hand_v2(hand h)
{
  char index[LEN_HAND]={0};
  short count[LEN_HAND]={0};
  for(int i=0; i<LEN_HAND;i++)
  {
    for(int j=0; j<LEN_HAND;j++)
    {
      if(index[j]=='\0')
      {
        index[j]=h[i];
        count[j]=1;
        break;
      } else if (index[j]==h[i])
      {
        count[j]+=1;
        break;
      }
    }
  }
  for(int i=0; i<LEN_HAND; i++)
  {
    for(int j=i+1; j<LEN_HAND; j++)
    {
      if(count[i]<count[j])
      {
        char ti = index[i]; index[i]=index[j]; index[j]=ti;
        short tc=count[i]; count[i]=count[j]; count[j]=tc;
      }
    }
  }
  if(index[0]=='J') {count[1]+=count[0]; count[0]=0;} else {
    for(int i=1; i<LEN_HAND;i++) {
      if (index[i]=='J') {count[0] += count[i]; count[i]=0;}
    }
  }
  bubblesort(&count);
  switch(count[0]) {
    case 5: return FIVE_OF_KIND;
    case 4: return FOUR_OF_KIND;
    case 3: 
      if(count[1] == 2) return FULL_HOUSE; 
      else return THREE_OF_KIND;
    case 2:
      if(count[1] == 2) return TWO_PAIR;
      else return ONE_PAIR;
    default: return HIGH_CARD;
  }
}

enum hand_type get_type_from_hand(hand h)
{
  char index[LEN_HAND]={0};
  short count[LEN_HAND]={0};
  for(int i=0; i<LEN_HAND;i++)
  {
    for(int j=0; j<LEN_HAND;j++)
    {
      if(index[j]=='\0')
      {
        index[j]=h[i];
        count[j]=1;
        break;
      } else if (index[j]==h[i])
      {
        count[j]+=1;
        break;
      }
    }
  }
  bubblesort(&count);
  switch(count[0]) {
    case 5: return FIVE_OF_KIND;
    case 4: return FOUR_OF_KIND;
    case 3: 
      if(count[1] == 2) return FULL_HOUSE; 
      else return THREE_OF_KIND;
    case 2:
      if(count[1] == 2) return TWO_PAIR;
      else return ONE_PAIR;
    default: return HIGH_CARD;
  }
}
static inline char *card_index(char c) {return strchr(card_order, c);}
static inline char *card_index_v2(char c) {return strchr(card_order_v2, c);}

static int compare_hands(hand_entry *h1, hand_entry *h2)
{
  if (h1->type < h2->type) return 1; /* h1 > h2 */
  else if (h1->type > h2->type) return -1; /* h1 < h2 */
  else {
    for(int i=0; i<LEN_HAND;i++)
    {
      if (card_index(h1->literal[i]) < card_index(h2->literal[i])) return 1;
      else if (card_index(h1->literal[i]) > card_index(h2->literal[i])) return -1;
      else continue;
    }
    return 0;
  }
}

static int compare_hands_v2(hand_entry *h1, hand_entry *h2)
{
  if (h1->type < h2->type) return 1; /* h1 > h2 */
  else if (h1->type > h2->type) return -1; /* h1 < h2 */
  else {
    for(int i=0; i<LEN_HAND;i++)
    {
      if (card_index_v2(h1->literal[i]) < card_index_v2(h2->literal[i])) return 1;
      else if (card_index_v2(h1->literal[i]) > card_index_v2(h2->literal[i])) return -1;
      else continue;
    }
    return 0;
  }
}

static int entry_sz = sizeof(typeof(hand_entry*));
static inline hand_entry *get_mid(hand_entry *start, hand_entry *end)
{
  return start + (end-start)/entry_sz/2 * entry_sz;
}

void merge_cp(hand_entry src[], hand_entry target[], int start, int end)
{
  for (int i = start; i<end; i++)
  {
    target[i]=src[i];
  }
}

// using array B as the working area, sort array A in rev. order
void merge(hand_entry A[], int start, int end,
           int (*cf)(hand_entry*, hand_entry*), hand_entry B[])
{
  if(end-start<=1) return;
  int mid = (start+end)/2;
  // store sorted results in A
  merge(A, start, mid, cf, B);
  merge(A, mid, end, cf, B);
  int p1=start, p2=mid;
  // store sorted res in B
  for(int i=start; i<end; i++)
  {
    if (p1==mid) {B[i]=A[p2]; p2++;}
    else if (p2==end) {B[i]=A[p1]; p1++;}
    else {
      int cmp = (*cf)(&A[p1], &A[p2]);
      if (cmp < 0) /* A[p1] < A[p2] */
      {
        B[i]=A[p2]; p2++; /* increment p2 */
      } else {
        B[i]=A[p1]; p1++;
      }
    }
  }
  // copy B into A
  merge_cp(B,A, start, end);
}

hand_entry parse_line(char *line)
{
  buf=line;
  hand_entry e={0};
  for(int i=0;i<LEN_HAND;i++) {
    e.literal[i]=*buf; buf++;
  }
  e.type=get_type_from_hand(e.literal);
  e.bid = getnum();
  return e;
}

hand_entry parse_line_v2(char *line)
{
  buf=line;
  hand_entry e={0};
  for(int i=0;i<LEN_HAND;i++) {
    e.literal[i]=*buf; buf++;
  }
  e.type=get_type_from_hand_v2(e.literal);
  e.bid = getnum();
  #ifdef DEBUG
  printf("hand: %s %d %d \n", e.literal, e.type, e.bid);
  #endif /* ifdef DEBUG */
  return e;
}

#define MAX_HANDS 1500

ULL part_1(FILE *input)
{
  char line[MAXLINE];
  int hand_count=0;
  hand_entry entry_arr[MAX_HANDS]={0};
  hand_entry work_arr[MAX_HANDS]={0};
  while(fgets(line, MAXLINE, input)){
    if(*line == '\n' || *line == '\0') break;
    entry_arr[hand_count]=parse_line(line);
    hand_count++;
  }
  merge(entry_arr, 0, hand_count, compare_hands, work_arr);
  ULL res=0;
  for(int i=hand_count-1; i>=0;i--) {
    res += (hand_count-i)*(entry_arr[i].bid);
  }
  return res;
}

ULL part_2(FILE *input)
{
  char line[MAXLINE];
  int hand_count=0;
  hand_entry entry_arr[MAX_HANDS]={0};
  hand_entry work_arr[MAX_HANDS]={0};
  while(fgets(line, MAXLINE, input)){
    if(*line == '\n' || *line == '\0') break;
    entry_arr[hand_count]=parse_line_v2(line);
    hand_count++;
  }
  merge(entry_arr, 0, hand_count, compare_hands_v2, work_arr);
  ULL res=0;
#ifdef DEBUG
  for(int i=0; i<hand_count;i++)
  {
    hand_entry e=entry_arr[i];
    printf("hand: %s %d %d \n", e.literal, e.type, e.bid);
  }
#endif /* ifdef DEBUG */
  for(int i=hand_count-1; i>=0;i--) {
    res += (hand_count-i)*(entry_arr[i].bid);
  }
  return res;
}

// void mergesort(hand_entry *first, hand_entry *last, 
//                int (*cfp)(hand_entry *, hand_entry *))
// {
//   if(first + entry_sz >= last) return;
//   hand_entry *mid=get_mid(first, last);
//   mergesort(first, mid, cfp);
//   mergesort(mid, last, cfp);
// }

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#define MAXLINE 255
#define DEBUG

enum object { ROUND, CUBE, EMPTY };

typedef enum object *row;

typedef struct pattern {
  enum object **data;
  int nrows;
  int ncols;
} pattern;

#define p_at(p, i, j) (p->data[i])[j]
typedef long long LL;

LL solution(FILE *input, int part);

#define CYCLES 1000000000


/* bitset implementation */

typedef unsigned long bitchunk;

typedef struct bitset {
  bitchunk *bits;
  int size;
  int chunks;
} bitset;

#define BYTE_SZ 8
static int _chunk_sz = sizeof(bitchunk) * BYTE_SZ;

// assumes same size for s1 and s2
static inline bool cmp_bitset(bitset *s1, bitset *s2) {
  for (int i = 0; i < s1->chunks; i++) {
    if (s1->bits[i] != s2->bits[i])
      return false;
  }
  return true;
}

static inline void set_bit(bitset *s, int offset) {
  int chunk_idx = offset / _chunk_sz;
  int chunk_offset = offset % _chunk_sz;
  bitchunk mask = 1ul << (_chunk_sz - chunk_offset - 1);
  s->bits[chunk_idx] |= mask;
}

static inline void unset_bit(bitset *s, int offset) {
  int chunk_idx = offset / _chunk_sz;
  int chunk_offset = offset % _chunk_sz;
  bitchunk mask = ~(1ul << (_chunk_sz - chunk_offset - 1));
  s->bits[chunk_idx] &= mask;
}

// assumes bitset is large enough
void serialize_pattern(const pattern *p, bitset *dest) {
  int cols = p->ncols;
  for (int i = 0; i < p->nrows; i++) {
    for (int j = 0; j < cols; j++) {
      if (p_at(p, i, j) == ROUND)
        set_bit(dest, i * cols + j);
      else
        unset_bit(dest, i * cols + j);
    }
  }
}

bitset *init_bitset(int size) {
  int n_chunks = size / _chunk_sz + (size % _chunk_sz == 0 ? 0 : 1);
  bitchunk *data = calloc(n_chunks, sizeof(bitchunk));
  bitset *s = malloc(sizeof(bitset));
  for (int i = 0; i < n_chunks; i++)
    data[i] = 0ul;
  s->bits = data;
  s->chunks = n_chunks;
  s->size = size;
  return s;
}

/* hash table implementation */

#define HASH_MULT 37
#define HASH_TBL_SZ 9000

static inline unsigned long hash_bitset(bitset *s) {
  unsigned long res = 0ul;
  for (int i = 0; i < s->chunks; i++) {
    res *= HASH_MULT;
    res += s->bits[i];
    res %= HASH_TBL_SZ;
  }
  return res;
}

typedef struct result_entry {
  bitset *result;
  int iterations;
  struct result_entry *next;
} result_entry;

typedef result_entry *result_tbl[HASH_TBL_SZ];

int
insert_result(result_tbl t, bitset *result, int iterations)
{
  unsigned long hash = hash_bitset(result);
  result_entry *n = t[hash];
  result_entry *new_node = malloc(sizeof(result_entry));
  new_node->iterations = iterations;
  new_node->result = result;
  new_node->next=NULL;
  if(n == NULL) {
    t[hash] = new_node;
    return iterations;
  }
  for(;; n=n->next) {
    bool cmp = cmp_bitset(result, n->result);
    if(cmp) return n->iterations; /* loop located; pass back result */
    if(n->next == NULL) goto search_done;
  }
search_done:
  n->next = new_node;
  return iterations;
}

/***    main procedure      ***/

int main(int argc, char *argv[]) {
  int problem = 0, opt;
  while ((opt = getopt(argc, argv, "p:")) != -1) {
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
  if (problem == 0)
    goto usage;
  FILE *input;
  unsigned long long res;
  if ((input = fopen(argv[optind], "r")) == NULL) {
    printf("Failed to open file: %s\n", argv[optind]);
    return 1;
  };

  switch (problem) {
  case 1:
    res = solution(input, 1);
    break;

  case 2:
    res = solution(input, 2);
    break;

  default:
    goto usage;
  }
  printf("Success: %llu\n", res);
}

row *mat_initializer() {
  row *mat = calloc(MAXLINE, sizeof(row));
  for (int i = 0; i < MAXLINE; i++)
    mat[i] = NULL;
  return mat;
}

void parse_pattern(FILE *input, pattern *pp) {
  char line[MAXLINE] = {0};
  int nrow = 0, ncol = 0;
  row *data = mat_initializer();
  fgets(line, MAXLINE, input);
  for (char *p = line; *p != '\n'; p++, ncol++) {
  }
  do {
    if (*line == '\n')
      break;
    row r = calloc(ncol, sizeof(enum object));
    for (int i = 0; i < ncol; i++) {
      switch (line[i]) {
      case 'O':
        r[i] = ROUND;
        break;
      case '#':
        r[i] = CUBE;
        break;
      case '.':
        r[i] = EMPTY;
        break;
      }
    }
    data[nrow] = r;
    nrow++;
  } while (fgets(line, MAXLINE, input));
  pattern p = {data, nrow, ncol};
  *pp = p;
}

LL col_weight(pattern *p, int col_num) {
  LL res = 0;
  int last_empty = 0;
  int rows = p->nrows;
  for (int i = 0; i < rows; i++) {
    switch (p_at(p, i, col_num)) {
    case EMPTY:
      break;
    case CUBE:
      last_empty = i + 1;
      break;
    default:
      res += (rows - last_empty);
      last_empty++;
      break;
    }
  }
  return res;
}

LL get_weight(pattern *p) {
  int rows = p->nrows;
  LL weight = 0;
  for (int i = 0; i < p->nrows; i++) {
    for (int j = 0; j < p->ncols; j++) {
      if (p_at(p, i, j) == ROUND)
        weight += (rows - i);
    }
  }
  return weight;
}

void print_pattern(pattern *p) {
  putchar('\n');
  for (int i = 0; i < p->nrows; i++) {
    for (int j = 0; j < p->ncols; j++) {
      switch (p_at(p, i, j)) {
      case EMPTY:
        putchar('.');
        break;
      case CUBE:
        putchar('#');
        break;
      default:
        putchar('O');
        break;
      }
    }
    putchar('\n');
  }
  putchar('\n');
}

/* simulation that mutates the state */

void simulate(pattern *p) {
  int rows = p->nrows, cols = p->ncols;
  /* north */
  for (int j = 0; j < cols; j++) {
    for (int i = 0, last_empty = 0; i < rows; i++) {
      int id = p_at(p, i, j);
      switch (id) {
      case EMPTY:
        break;
      case CUBE:
        last_empty = i + 1;
        break;
      default:
        p_at(p, i, j) = EMPTY;
        p_at(p, last_empty, j) = id;
        last_empty++;
        break;
      }
    }
  }
  /* west */
  for (int i = 0; i < rows; i++) {
    for (int j = 0, last_empty = 0; j < cols; j++) {
      int id = p_at(p, i, j);
      switch (id) {
      case EMPTY:
        break;
      case CUBE:
        last_empty = j + 1;
        break;
      default:
        p_at(p, i, j) = EMPTY;
        p_at(p, i, last_empty) = id;
        last_empty++;
        break;
      }
    }
  }
  /*south*/
  for (int j = 0; j < cols; j++) {
    for (int i = rows - 1, last_empty = rows - 1; i >= 0; i--) {
      int id = p_at(p, i, j);
      switch (id) {
      case EMPTY:
        break;
      case CUBE:
        last_empty = i - 1;
        break;
      default:
        p_at(p, i, j) = EMPTY;
        p_at(p, last_empty, j) = id;
        last_empty--;
        break;
      }
    }
  }
  /* east */
  for (int i = 0; i < rows; i++) {
    for (int j = cols - 1, last_empty = cols - 1; j >= 0; j--) {
      int id = p_at(p, i, j);
      switch (id) {
      case EMPTY:
        break;
      case CUBE:
        last_empty = j - 1;
        break;
      default:
        p_at(p, i, j) = EMPTY;
        p_at(p, i, last_empty) = id;
        last_empty--;
        break;
      }
    }
  }
}

LL solution(FILE *input, int part) {
  LL res = 0;
  pattern p = {0};
  pattern *pp = &p;
  parse_pattern(input, pp);
  int pattern_sz =pp->nrows * pp->ncols;

  result_entry* table[HASH_TBL_SZ]={0};

  if (part == 1) {
    for (int i = 0; i < pp->ncols; i++) {
      LL lres = col_weight(pp, i);
#ifdef DEBUG
      printf("col %d: %lld\n", i, lres);
#endif
      res += lres;
    }
    return res;
  } else {
    int iters = 1;
    int cycle_len=-1, cycle_remaining=-1;
    for (;; iters++) {
      simulate(pp);
      bitset *next = init_bitset(pattern_sz);
      serialize_pattern(pp, next);
      int insert_res = insert_result(table, next, iters);
      if(insert_res != iters) {
        printf("Cycle found at %i last seen at %i\n",
               iters, insert_res);
        cycle_len = iters - insert_res;
        cycle_remaining = (CYCLES - iters) % cycle_len;
        printf("Cycles remaining: %i\n", cycle_remaining);
        break;
      }
      if(iters > 1000000) {
        printf("Nothing found after 1 million rounds...\n");
        break;
      }
    }
    for(int i=0;i<cycle_remaining;i++) {
      simulate(pp);
    }
    res = get_weight(pp);
  }
  return res;
}

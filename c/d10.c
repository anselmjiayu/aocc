#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/param.h>

#define MAXLINE 255

// #define DEBUG

typedef struct pipe {
  char lexeme;
  /* Waste memory to improve function call signatures
   * and get object-like semantics */
  int row;
  int col;
  struct pipe *north;
  struct pipe *east;
  struct pipe *south;
  struct pipe *west;
  /* marks the pipe as part of the loop */
  bool loop_p;
  // int dist1;
  // int dist2;
} pipe;

enum direction {NORTH, EAST, SOUTH, WEST};

typedef pipe pipe_row[];
typedef pipe_row *pipe_graph[];

int part_1(FILE *input);
int part_2(FILE *input);

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


int read_line(const char *line, int rownum, pipe_row **rp)
{
  /* get number of pipes */
  int row_len=0;
  for(char *p=(char*)line; *p != '\n'; p++) row_len++;

  pipe_row *row = calloc(row_len, sizeof(pipe));
  for(int i=0;i<row_len;i++)
  {
    (*row)[i].lexeme = *(line+i);
  }
  *rp = row;
  return row_len;
}

pipe *getstart(pipe_graph g, int nrows, int ncols,
               int *rowp, int* colp)
{
  for(int i=0;i<nrows;i++){
    for(int j=0;j<ncols;j++) {
      if((*g[i])[j].lexeme == 'S') {
        *rowp = i; *colp = j;
        return &(*g[i])[j];
      }
    }
  }
  return NULL;
}


#define graph_at(g, row, col) &(*g[row])[col]

void register_connections(pipe_graph g, int nrows, int ncols)
{
  for(int i=0;i<nrows;i++) {
    for (int j=0;j<ncols;j++) {
      pipe *p = graph_at(g, i, j);
      // initialize pointers.
      p->north=NULL; p->east=NULL;
      p->south=NULL; p->west=NULL;
      p->loop_p=false;

      switch(p->lexeme) {
        case 'J':
          if (j>0) p->west = graph_at(g, i, j-1);
          if(i>0) p->north = graph_at(g,i-1,j);
          break;
        case 'F':
          if(j<ncols-1) p->east = graph_at(g,i,j+1);
          if(i<nrows-1) p->south = graph_at(g,i+1,j);
          break;
        case '7':
          if (j>0) p->west = graph_at(g, i, j-1);
          if(i<nrows-1) p->south = graph_at(g,i+1,j);
          break;
        case 'L':
          if(j<ncols-1) p->east = graph_at(g,i,j+1);
          if(i>0) p->north = graph_at(g,i-1,j);
          break;
        case '|':
          if(i>0) p->north = graph_at(g,i-1,j);
          if(i<nrows-1) p->south = graph_at(g,i+1,j);
          break;
        case '-':
          if (j>0) p->west = graph_at(g, i, j-1);
          if(j<ncols-1) p->east = graph_at(g,i,j+1);
          break;
      }
    }
  }
}

int 
walk(pipe* start, enum direction dir, pipe *node, int dist)
{
  if(node==start) return dist;
#ifdef DEBUG
  printf("Node: %c, Dist: %d, Dir: %d\n", node->lexeme,
         dist, dir);
#endif
  node->loop_p = true;
  switch(node->lexeme) {
    case 'J': 
      if(dir == EAST) 
        return walk(start, NORTH, node->north, dist+1);
      else return walk(start, WEST, node->west, dist+1);
    case 'F': 
      if(dir == WEST)
        return walk(start, SOUTH, node->south, dist+1);
      else return walk(start, EAST, node->east, dist+1);
    case '7': 
      if(dir == EAST) 
        return walk(start, SOUTH, node->south, dist+1);
      else return walk(start, WEST, node->west, dist+1);
    case 'L': 
      if(dir == WEST)
        return walk(start, NORTH, node->north, dist+1);
      else return walk(start, EAST, node->east, dist+1);
    case '|': 
      if(dir == SOUTH) 
        return walk(start, SOUTH, node->south, dist+1);
      else return walk(start, NORTH, node->north, dist+1);
    case '-': 
      if(dir == WEST)
        return walk(start, WEST, node->west, dist+1);
      else return walk(start, EAST, node->east, dist+1);
    default: return -1; /* walk failed */
  }
}

int 
count_inside_nodes(pipe_graph g, int nrows, int ncols)
{
  int count=0;
  for(int i=0;i<nrows;i++) {
    bool up_align=false, down_align=false, done=true;
    for(int j=0, crossings=0; j<ncols;j++) {
      pipe *p  = graph_at(g, i, j);
#ifdef DEBUG
      printf("r%d c%d %c up%b down%b cross%d n%d in%b\n",
             i, j, p->lexeme, up_align, down_align, crossings,
             count, p->loop_p);
#endif
      if(p->loop_p) {
        switch(p->lexeme){

          /* Explanation:
           * only pipes that fully cross a node count as cutting
           * across the border. Count if the current pipes are
           * cutting across, and use the classic inside shape
           * algorithm. */
          case 'J': case'L':
            if(down_align) crossings++;
            done = !done;
            up_align = !done;
            down_align=false;
          break;
          case '7': case'F':
            if(up_align) crossings++;
            done=!done;
            down_align=!done;
            up_align=false;
          break;
          case '|':
            crossings++;
          break;
        }
      } else {
        if (crossings%2 == 1) count++;
      }
    }
  }
  return count;
}

int part_1(FILE *input)
{
  char line[MAXLINE];
  int nrows=0, ncols=0;
  pipe_row *g[MAXLINE]={0};
  for(; fgets(line, MAXLINE, input); nrows++)
  {
    if(*line=='\n') break;
    pipe_row *rp = NULL;
    // assume each line has same amnt of cols
    ncols = read_line(line, nrows, &rp);
    g[nrows] = rp;
  }
  int start_row=0, start_col=0;
  pipe *start_pipe = getstart(g, nrows, ncols, &start_row, &start_col);
  register_connections(g, nrows, ncols);
  int res;
  if(start_col>0 && 
    (graph_at(g, start_row, start_col-1))->east != NULL)
    res = walk(start_pipe, WEST,
               graph_at(g, start_row, start_col-1), 1);
  else if (start_row>0 && 
    (graph_at(g, start_row, start_col-1))->south != NULL)
    res = walk(start_pipe, NORTH,
               graph_at(g, start_row-1, start_col), 1);

  else
    /* assert that a loop exists at the start node */
    res = walk(start_pipe, EAST,
               graph_at(g, start_row, start_col+1), 1);


return res/2 + res%2;
}

int part_2(FILE *input)
{
  char line[MAXLINE];
  int nrows=0, ncols=0;
  pipe_row *g[MAXLINE]={0};
  for(; fgets(line, MAXLINE, input); nrows++)
  {
    if(*line=='\n') break;
    pipe_row *rp = NULL;
    // assume each line has same amnt of cols
    ncols = read_line(line, nrows, &rp);
    g[nrows] = rp;
  }
  int start_row=0, start_col=0;
  pipe *start_pipe = getstart(g, nrows, ncols, &start_row, &start_col);
  register_connections(g, nrows, ncols);
  // add start node to loop
  start_pipe->loop_p = true;
  int loop_len;
  /* determine start node type */
  bool s_west = start_col>0 && 
    (graph_at(g, start_row, start_col-1))->east != NULL;
  bool s_north = start_row>0 && 
    (graph_at(g, start_row-1, start_col))->south != NULL;
  bool s_east = start_col<ncols-1 &&
    (graph_at(g, start_row, start_col+1))->west != NULL;
  bool s_south = start_row<nrows-1 && 
    (graph_at(g, start_row+1, start_col))->north != NULL;

  if(s_north && s_south) start_pipe->lexeme='|';
  else if(s_east && s_west) start_pipe->lexeme='-';
  else if(s_west && s_north) start_pipe->lexeme='J';
  else if(s_west && s_south) start_pipe->lexeme='7';
  else if(s_east && s_south) start_pipe->lexeme='F';
  else if(s_east && s_north) start_pipe->lexeme='L';

  if(s_west)
    loop_len = walk(start_pipe, WEST,
               graph_at(g, start_row, start_col-1), 1);
  else if (s_north)
    loop_len = walk(start_pipe, NORTH,
               graph_at(g, start_row-1, start_col), 1);

  else
    /* assert that a loop exists at the start node */
    loop_len = walk(start_pipe, EAST,
               graph_at(g, start_row, start_col+1), 1);

return count_inside_nodes(g, nrows, ncols);
}

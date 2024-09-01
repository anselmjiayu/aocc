#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#define MAXLINE 255

// #define DEBUG

enum surface { EMPTY, VERT, HRI, SL, BSL };

typedef unsigned int mode;
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define REFRACT_INDEX 4
#define REFLECT_MASK 0b1111
#define LIGHT 0b1
#define ABSENT 0b0

#define reflect_mask(input, incident, reflected)                               \
	((input & (LIGHT << incident)) >> incident << reflected)
#define exchange(input, d1, d2)                                                \
	(reflect_mask(input, d1, d2) | reflect_mask(input, d2, d1) |               \
	 (input & ~(LIGHT << d1) & ~(LIGHT << d2)))

#define tilt_mirror(input) exchange(exchange(input, NORTH, EAST), SOUTH, WEST)
#define backtilt_mirror(input)                                                 \
	exchange(exchange(input, NORTH, WEST), SOUTH, EAST)

#define refract(input, absorb, d1, d2)                                         \
	((input & ~(LIGHT << absorb)) | (((input >> absorb) & LIGHT) << d1) |      \
	 (((input >> absorb) & LIGHT) << (d2 + REFRACT_INDEX)))

#define dash_mirror(input)                                                     \
	refract(refract(input, NORTH, EAST, WEST), SOUTH, EAST, WEST)
#define pipe_mirror(input)                                                     \
	refract(refract(input, EAST, NORTH, SOUTH), WEST, NORTH, SOUTH)
#define reflection_from(rays) (rays & REFLECT_MASK)
#define refraction_from(rays) (rays >> REFRACT_INDEX)
#define has_direction(mode, dir) (mode & (LIGHT << dir))
#define overlap(m1, m2) (m1 & m2)
#define add(m1, m2) (m1 | m2)

typedef struct ray {
	int row;
	int col;
	mode dir;
} ray;

typedef struct location {
	enum surface surface;
	mode state;
} location;

typedef struct contraption {
	location ***tiles;
	int nrow;
	int ncol;
} contraption;

#define c_at(c, i, j) (c->tiles)[i][j]

void
advance(ray *r)
{
	if (has_direction(r->dir, EAST))
		r->col++;
	else if (has_direction(r->dir, WEST))
		r->col--;
	else if (has_direction(r->dir, SOUTH))
		r->row++;
	else if (has_direction(r->dir, NORTH))
		r->row--;
}

ray *
duplicate(ray *r)
{
	ray *dup = malloc(sizeof(ray));
	dup->dir = r->dir;
	dup->row = r->row;
	dup->col = r->col;
	return dup;
}

void
sync(contraption *c, ray *r)
{
	if (r->col < 0 || r->row < 0 || r->col >= c->ncol || r->row >= c->nrow)
		return free(r);
	location *l = c_at(c, r->row, r->col);
	if (overlap(l->state, r->dir))
		return free(r);
	l->state = add(l->state, r->dir);
	mode refraction;
	ray *ref;
	switch (l->surface) {
	case EMPTY:
		advance(r);
		return sync(c, r);
	case SL:
		r->dir = tilt_mirror(r->dir);
		advance(r);
		return sync(c, r);
	case BSL:
		r->dir = backtilt_mirror(r->dir);
		advance(r);
		return sync(c, r);
	case VERT:
		refraction = refraction_from(pipe_mirror(r->dir));
		if (refraction) {
			ref = duplicate(r);
			ref->dir = refraction;
			advance(ref);
			sync(c, ref);
		}
		r->dir = reflection_from(pipe_mirror(r->dir));
		advance(r);
		return sync(c, r);
	case HRI:
		refraction = refraction_from(dash_mirror(r->dir));
		if (refraction) {
			ref = duplicate(r);
			ref->dir = refraction;
			advance(ref);
			sync(c, ref);
		}
		r->dir = reflection_from(dash_mirror(r->dir));
		advance(r);
		return sync(c, r);
	}
}

int
powered_tiles(contraption *c)
{
	int res = 0;
	for (int i = 0; i < c->nrow; i++) {
		for (int j = 0; j < c->ncol; j++) {
			if (c_at(c, i, j)->state)
				res++;
		}
	}
	return res;
}

void
print_surface(contraption *c)
{
	putchar('\n');
	for (int i = 0; i < c->nrow; i++) {
		for (int j = 0; j < c->ncol; j++) {
			switch (c_at(c, i, j)->surface) {
			case VERT:
				putchar('|');
				break;
			case EMPTY:
				putchar('.');
				break;
			case HRI:
				putchar('-');
				break;
			case SL:
				putchar('/');
				break;
			case BSL:
				putchar('\\');
				break;
			}
		}
		putchar('\n');
	}
	putchar('\n');
}

void
print_states(contraption *c)
{
	putchar('\n');
	for (int i = 0; i < c->nrow; i++) {
		for (int j = 0; j < c->ncol; j++) {
			printf("%2d", c_at(c, i, j)->state);
		}
		putchar('\n');
	}
	putchar('\n');
}

contraption *
construct_contraption(FILE *input)
{
	char line[MAXLINE];
	location ***tiles = calloc(MAXLINE, sizeof(location **));
	fgets(line, MAXLINE, input);
	int nrow = 0, ncol = 0;
	for (char *p = line; *p != '\n'; p++, ncol++) {
	}
	do {
		if (*line == '\n' || *line == '\0')
			break;
		location **row = calloc(ncol, sizeof(location *));
		for (int i = 0; i < ncol; i++) {
			location *t = malloc(sizeof(location));
			switch (line[i]) {
			case '|':
				t->surface = VERT;
				break;
			case '-':
				t->surface = HRI;
				break;
			case '/':
				t->surface = SL;
				break;
			case '\\':
				t->surface = BSL;
				break;
			default:
				t->surface = EMPTY;
				break;
			}
			t->state = ABSENT;
			row[i] = t;
		}
		tiles[nrow] = row;
		nrow++;
	} while (fgets(line, MAXLINE, input));
	contraption *c = malloc(sizeof(contraption));
	c->nrow = nrow;
	c->ncol = ncol;
	c->tiles = tiles;
	return c;
}

void
recalibrate_contraption(contraption *c)
{
	for (int i = 0; i < c->nrow; i++) {
		for (int j = 0; j < c->ncol; j++) {
			c_at(c, i, j)->state = ABSENT;
		}
	}
}
int
test_level(contraption *c, int side, int index)
{
	recalibrate_contraption(c);
	ray *init = malloc(sizeof(ray));
	switch (side) {
	case NORTH:
		init->row = 0;
		init->col = index;
		init->dir = LIGHT << SOUTH;
		break;
	case SOUTH:
		init->row = c->nrow - 1;
		init->col = index;
		init->dir = LIGHT << NORTH;
		break;
	case EAST:
		init->col = c->ncol - 1;
		init->row = index;
		init->dir = LIGHT << WEST;
		break;
	case WEST:
		init->col = 0;
		init->row = index;
		init->dir = LIGHT << EAST;
		break;
	}
	sync(c, init);
	return powered_tiles(c);
}

long solution(FILE *input, int part);

int
main(int argc, char *argv[])
{
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

long
solution(FILE *input, int part)
{
	long res = 0;
	contraption *c = construct_contraption(input);
	if (part == 1) {

		ray *init = malloc(sizeof(ray));
		init->row = 0;
		init->col = 0;
		init->dir = LIGHT << EAST;
#ifdef DEBUG
		print_surface(c);
		print_states(c);
#endif
		sync(c, init);
		res = powered_tiles(c);
#ifdef DEBUG
		print_surface(c);
		print_states(c);
#endif
	} else {
		for (int i = 0; i < c->ncol; i++) {
			res = MAX(res, test_level(c, NORTH, i));
			res = MAX(res, test_level(c, SOUTH, i));
		}
		for (int i = 0; i < c->nrow; i++) {
			res = MAX(res, test_level(c, EAST, i));
			res = MAX(res, test_level(c, WEST, i));
		}
	}
	return res;
}

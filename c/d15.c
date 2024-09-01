#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#define MAXLINE 30000

// #define DEBUG

#define LENS_TBL_SZ 256

typedef struct lens_box {
	char *label;
	int len;
	struct lens_box *next;
} lens_box;

typedef lens_box *lens_tbl[LENS_TBL_SZ];

enum token_type { LABEL, PUTS, POPS, DONE };

int
hash(char *label)
{
	int res = 0;
	for (char *p = label; *p != '\0'; p++) {
		res += (int)*p;
		res *= 17;
		res %= LENS_TBL_SZ;
	}
	return res;
}

void
tbl_insert(lens_tbl t, char *label, int len)
{
	int h = hash(label);
	lens_box *n = t[h];
	lens_box *new;
#ifdef DEBUG
	printf("inserting label %s with len %d at index %d\n", label, len, h);
#endif
	if (n == NULL)
		goto new_node;
	for (; n->next != NULL; n = n->next) {
		if (strcmp(label, n->label) == 0) {
			n->len = len;
			return;
		}
	}
	if (strcmp(label, n->label) == 0) {
		n->len = len;
		return;
	}

new_node:
	new = malloc(sizeof(lens_box));
	new->label = label;
	new->len = len;
	new->next = NULL;
	if (n)
		n->next = new;
	else
		t[h] = new;
}

void
tbl_pop(lens_tbl t, char *label)
{
	int h = hash(label);
	lens_box *prv = t[h];
#ifdef DEBUG
	printf("removing label %s at index %d\n", label, h);
#endif
	if (prv == NULL)
		return;
	if (strcmp(prv->label, label) == 0) {
		t[h] = prv->next;
		free(prv);
		return;
	}
	for (lens_box *n = prv->next; n != NULL; n = n->next, prv = prv->next) {
		if (strcmp(n->label, label) == 0) {
			prv->next = n->next;
			free(n);
			return;
		}
	}
}

long
tbl_power(lens_tbl t)
{
	long res = 0;
	for (int i = 0; i < LENS_TBL_SZ; i++) {
		int slot = 1;
		for (lens_box *n = t[i]; n != NULL; n = n->next, slot++) {
			res += (i + 1) * slot * n->len;
#ifdef DEBUG
			printf("box: %d, label:%s, slot: %d, len: %d\n", i + 1, n->label,
			       slot, n->len);
#endif
		}
	}
	return res;
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

	char line[MAXLINE];
	fgets(line, MAXLINE, input);
	int line_res = 0;
	if (part == 1) {
		for (int i = 0;; i++) {
			switch (line[i]) {
			case '\n':
			case '\0':
				res += line_res;
				goto parse_done;
				break;
			case ',':
				res += line_res;
				// printf("line res: %d\n", line_res);
				line_res = 0;
				break;
			default:
				line_res += (int)line[i];
				line_res *= 17;
				line_res %= 256;
				break;
			}
		}
	parse_done:
		return res;
	} else {
		lens_box *tbl[LENS_TBL_SZ] = {0};
		char *p = line, *lookahead = line;
		enum token_type t = LABEL;
		char *init_label = NULL;
		char **label = &init_label;
		for (;;) {
			if (*p == '\n' || *p == '\0')
				break;
			switch (t) {
			case LABEL:
				lookahead = p;
				int label_sz = 0;
				for (; isalpha(*lookahead); lookahead++, label_sz++) {
				}
				*label = strndup(p, label_sz);
				p = lookahead;
				if (*p == '=')
					t = PUTS;
				else
					t = POPS;
				break;
			case PUTS:
				p++;
				assert(isdigit(*p));
				tbl_insert(tbl, *label, (*p - '0'));
				t = DONE;
				p++; // consume digit
				break;
			case POPS:
				p++;
				tbl_pop(tbl, *label);
				t = DONE;
				break;
			case DONE:
				p++;
				t = LABEL;
				break;
			}
		}
		res = tbl_power(tbl);
	}
	return res;
}

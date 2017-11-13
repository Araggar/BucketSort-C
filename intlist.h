#ifndef INTLIST_H
#define INTLIST_H

typedef struct IntList {
	int* int_list;
	int index;
} List;

void intlist_init(List*, int);
void intlist_append(List*, int*);
#endif
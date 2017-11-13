#include "intlist.h"
#include <stdlib.h>

void intlist_init(List* list, int size, int* intlist) {
	list->int_list = intlist;
	list->index = 0;
}

void intlist_append(List* list, int* data) {
	list->int_list[list->index++] = *data;
}







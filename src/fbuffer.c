
/*  fbuffer.c
 *
 *  This file is part of zipPacker
 *
 *  Copyright © 1990−2019 by nýx.ch, CC-BY 4.0
 *
 *  Dieses Werk ist lizenziert unter der CC-BY
 *  Creative Commons Namensnennung 4.0 International Lizenz.
 *  https://creativecommons.org/licenses/by/4.0/deed.de
 *
 */

#include <string.h>

#ifdef __linux__
    #include <malloc.h>
#elif __APPLE__
	#include <stdlib.h>
	#include <malloc/malloc.h>
	#define malloc_usable_size malloc_size 
#endif

#include "fbuffer.h"

/* Return count entries in array */
fBuffer_len_t fbuffer_len(fBuffer_t *p) {
	fBuffer_len_t count = 0;
	fBuffer_t *ptr=p;
	while (ptr) {
		count++;
		ptr = ptr->_next;
	}
	return count;
}

/*  allocate memory for new entry ... */
fBuffer_t *fbuffer_new() {
	return (fBuffer_t*)calloc(1, sizeof(fBuffer_t));
}

/* add entry and return pointer to new entry */
fBuffer_t *fbuffer_append(fBuffer_t *p_last) {
	p_last->_next = fbuffer_new();
	return p_last->_next;
}

/* add entry at first position and return pointer to new start position */
fBuffer_t *fbuffer_insert(fBuffer_t *p) {
	fBuffer_t *pnew = fbuffer_new();
    pnew->_next = p;
    return pnew;
}

/* free allocated memory */
void fbuffer_free(fBuffer_t *p) {
    fBuffer_t *psave, *ptr=p;
    while (ptr) {
		//if (p[i].path != NULL) free(p[i].path);
		if (ptr->path) free(ptr->path);
        psave = ptr;
		ptr = ptr->_next;
		free(psave);
    }
}

/*
vim: ts=4:sw=4:
*/

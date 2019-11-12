
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

/* Return count (reserved) entries in array */
fBuffer_len_t fbuffer_len(fBuffer_t *p) {
    return malloc_usable_size(p) / sizeof(fBuffer_t);
	//return malloc_size(p) / sizeof(fBuffer_t);
} 

/*  allocate memory */
fBuffer_t *fbuffer_new(fBuffer_len_t count) {
    //return (fBuffer_t*)malloc(count * sizeof(fBuffer_t));
    return calloc(count, sizeof(fBuffer_t));
}

/* free allocated memory */
void fbuffer_free(fBuffer_t *p) {
    fBuffer_len_t len = fbuffer_len(p);
    for (fBuffer_len_t i=0; i < len-1; i++) {
        if (p[i].path != NULL) free(p[i].path);
    }
    free(p);
}

/* free all allocated memory for char *path...  */
void fbuffer_clear(fBuffer_t *p) {
    fBuffer_len_t len = fbuffer_len(p);
    for (fBuffer_len_t i=0; i < len-1; i++) {
        if (p[i].path != NULL) free(p[i].path);
    }
	memset(p, 0, len * sizeof(fBuffer_t));
}

/* allocate memory and return a copy of p */
/* only copy data with p[i].path != NULL   */
fBuffer_t *fbuffer_dup(fBuffer_t *p) {
	fBuffer_len_t len = fbuffer_len(p);

	fBuffer_t *pnew = fbuffer_new(len);

	for (fBuffer_len_t i=0; i < len-1; i++) {
        if (p[i].path != NULL) {
			pnew[i].path = strdup(p[i].path);
			pnew[i].st_mode = p[i].st_mode;
			pnew[i].st_size = p[i].st_size;
		}
    }
	return pnew;
}

/*
vim: ts=4:sw=4:
*/


/*  fbuffer.h
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


#ifndef FBUFFER_H
#define FBUFFER_H

#include <sys/stat.h>

#define fBuffer_len_t __uint32_t

typedef struct fBuffer_t {
	char 	*path;		/* File/Path name */
	mode_t	st_mode; 	/* File type and mode */
	off_t	st_size;	/* Total size, in bytes */
} fBuffer_t;


extern fBuffer_len_t fbuffer_len(fBuffer_t *p);
extern fBuffer_t *fbuffer_new(fBuffer_len_t count);
extern void fbuffer_free(fBuffer_t *p);
extern void fbuffer_clear(fBuffer_t *p);
extern fBuffer_t *fbuffer_dup(fBuffer_t *p);



#endif /* FBUFFER_H */

/*
vim: ts=4:sw=4:
*/

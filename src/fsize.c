
/*  fsize.c
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fcntl.h>
 

off_t fsize(char* fname) {
    struct stat stbuf;
    int fd;

    fd = open(fname, O_RDONLY);
    if (fd == -1) {
    	/* TODO:  Handle error */
        return -1;
    }

    if ((fstat(fd, &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
    	/* TODO: Handle error */
	close(fd);
        return -2;
    }

    close(fd);
    return stbuf.st_size;
}

/* 
vim: ts=4:sw=4:
*/

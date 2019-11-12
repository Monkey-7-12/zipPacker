
/*  dirwalk.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

/* PATH_MAX */
#ifdef __linux__
	#include <linux/limits.h>
#elif __APPLE__
	#include <sys/syslimits.h>
#endif


void dir_walker(char* name, void (*f_callback)(), void (*d_callback)()) {
	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;
	char path[PATH_MAX];

	if (!(dir = opendir(name))) {
		fprintf(stderr, "Can not open directory %s\n", name);
		exit(1);
	}

	while ((entry = readdir(dir)) != NULL) {
		snprintf(path, PATH_MAX, "%s/%s", name, entry->d_name);

		//printf(">>%s\n", entry->d_name);
		if (stat(path, &statbuf) != 0)
		{
			// FIXME
			continue;
		}
		
		if (S_ISDIR(statbuf.st_mode)) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			//snprintf(path, PATH_MAX, "%s/%s", name, entry->d_name);
			if (d_callback != NULL) (*d_callback)(path, statbuf.st_mode, statbuf.st_size);
			dir_walker(path, f_callback, d_callback);
		} else {
			//snprintf(path, PATH_MAX, "%s/%s", name, entry->d_name);
			if (f_callback != NULL) (*f_callback)(path, statbuf.st_mode, statbuf.st_size);
		}
	}
	closedir(dir);
}

/* 
vim: ts=4:sw=4:
*/

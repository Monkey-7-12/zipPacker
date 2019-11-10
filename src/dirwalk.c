
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

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int is_dir(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

void dir_walker(char* name, void (*f_callback)(), void (*d_callback)()) {
	DIR *dir;
	struct dirent *entry;
	char path[PATH_MAX];

	if (!(dir = opendir(name))) {
		fprintf(stderr, "Can not open directory %s\n", name);
		exit(1);
	}

	while ((entry = readdir(dir)) != NULL) {
		// DT_DIR work only with Btrfs, ext2, ext3 und ext4
		if (entry->d_type == DT_UNKNOWN) {
			if (is_dir(entry->d_name)) entry->d_type = DT_DIR;
		}

		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
			if (d_callback != NULL) (*d_callback)(path);
			dir_walker(path, f_callback, d_callback);
		} else {
			snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
			if (f_callback != NULL) (*f_callback)(path);
		}
	}
	closedir(dir);
}

/* 
vim: ts=4:sw=4:
*/

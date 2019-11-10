
/*  main.c
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
#include <unistd.h>
#include <string.h>
#include <zip.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <ctype.h> // tolower()

#include "strsize.h"
#include "fsize.h"
#include "dirwalk.h"

// PATH_MAX
#ifdef __linux__
	#include <linux/limits.h>
#elif __APPLE__
	#include <sys/syslimits.h>
#endif

#define MAX_FILES 1024				// Default max files per zip
#define MAX_SIZE  1073741824 		// Default max size per zip 

static int  max_files = MAX_FILES;	// Max files per zip file
static long max_size  = MAX_SIZE;	// Max size per zip file
static long depth_len = 0;		  // count character do delete from input filename

static char *zip_filename = NULL;	// output filename (format mask)

static unsigned fbuffer_size;		// size of fbuffer 
static char **fbuffer;  			// array of filenames
static char **pfnext;			   // ptr to next-free filename space

static  uint64_t total_size = 0;
static int zipfile_idx = 0;			// index for zipfile name _0001, _0002 ... ... ... 







void clear_buffer(char **buffer, unsigned size) {
	for (char **p = fbuffer; *p != NULL; p++) {
		free(*p);
	}
	memset(buffer, 0x00, size);
}



void zipper(char* zipfile, char** files) {
	int error;
	zip_int64_t idx;
	char *fname;

	//printf("zipper... %s\n", zipfile);

	zip_t *zip = zip_open(zipfile, ZIP_CREATE | ZIP_EXCL, &error);
	if (zip == NULL) {
		zip_error_t ziperror;
		zip_error_init_with_code(&ziperror, error);
		fprintf(stderr, "Failed to open output file %s: %s\n", zipfile, zip_error_strerror(&ziperror));
		exit(1);
	}
	
	for (char **p = files; *p != NULL; p++) {
		zip_source_t *source = zip_source_file(zip, *p, 0, 0);
		if (source == NULL) {
			fprintf(stderr, "-Failed to add file to zip: %s\n",  zip_strerror(zip));
			exit(3);
		}

		fname = &(*p)[depth_len];
		if (fname[0] == '/') fname++;
		idx = zip_file_add(zip, fname, source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8); 
		if (idx < 0) {
			zip_source_free(source);
			fprintf(stderr, "Failed to add file to zip: %s\n", zip_strerror(zip));
			exit(4);
		} 
		if (zip_set_file_compression(zip, idx, ZIP_CM_STORE, 0) < 0) {
			fprintf(stderr, "-Failed to set compressions flag: %s\n",  zip_strerror(zip));
			exit(5);
		}

		free(*p);  
	}
	if (zip_close(zip) < 0) {
		zip_error_t *ziperror = zip_get_error(zip);
		//int ze = zip_error_code_zip(error);
		//int se = zip_error_code_system(error);
		fprintf(stderr, "Failed to write output file %s: %s\n", zipfile, zip_error_strerror(ziperror));
		exit(1);
		
	}
	printf("   close\n");


	// TODO: test zip file exists
	
	//for (char **p = files; *p != NULL; p++) {
	//	printf("  > %s\n", *p);
	//	free(*p);
	//}
	free(files);




}

void do_zip() {
	// make a copy of array	
	int len;
	int idx;
	char **buf = malloc(fbuffer_size);
	memset(buf, 0x00, fbuffer_size);

	// FIXME: ???
	// for (int i=0; i<fbuffer_size && fbuffer[i] != NULL; i++) { ??????????
	for (idx=0; idx<max_files && fbuffer[idx] != NULL; idx++) {
		len = strlen(fbuffer[idx]);
		buf[idx] = malloc(len+1);
		strcpy(buf[idx], fbuffer[idx]);
	}
		
	// fork zipper()
	zipfile_idx++;
	char zip_name[PATH_MAX];
	sprintf(zip_name, zip_filename, zipfile_idx);
	printf("start zipper(%s)\n", zip_name);
	char *s = size2str(total_size,0);
	printf("  size : %s\n", s);
	printf("  files: %d\n", idx);
	zipper(zip_name, buf);
}

/* add_file_cb callback */
void add_file_cb(char* fname) {
	// add file to buffer-array
	int len = strlen(fname);
	off_t size;

	size = fsize(fname);
	if (size < 0) {
		// TODO / FIXME 
		#ifdef __linux__
			printf("error: size = %ld - %s\n", size, fname);
		#elif __APPLE__
    		printf("error: size = %lld - %s\n", size, fname);
		#endif
		exit(6);
	}

	if (total_size + size >= max_size || pfnext - fbuffer == max_files) {
		do_zip();
		clear_buffer(fbuffer, fbuffer_size);
		pfnext = fbuffer;
		total_size = 0;
	}

	*pfnext = (char *)malloc(len+1);
	strcpy(*pfnext, fname);
	pfnext++;
	if (size > 0) total_size += size;
}


void usage(char* prog, int exitcode) {
	printf(
		"usage: %s Options DIRECTORY [DIRECTORY ...]\n\n"
		"Options:\n"
		"  -c MAX_FILES   Set max files for zip archives\n"
		"                 Default: %d\n"
		"  -k             keep DIRECTORY name in zip file\n"
		"  -o FILENAME    Output zip-filename format string\n"
		"                 e.g.: /tmp/test_%%03d.zip\n"
		"                 -> /tmp/test_001.zip, /tmp/test_002.zip...\n"
		"  -s SIZE        Max SIZE for zip-archive\n"
		"                 e.g.: -s 52428800 -> = 50M\n"
		"                 -s 25G, -s 1024K...\n"
		"                 Default: %s\n"
		, prog, MAX_FILES, size2str(MAX_SIZE,0)
	);
	exit(exitcode);
}


int main(int argc, char* argv[]) {
	int opt; 
	int keep = 0;
 
	while((opt = getopt(argc, argv, ":c:kho:s:")) != -1)  
	{  
		switch(opt)  
		{  
			case 'c':   max_files = strtol(optarg, NULL, 0);
						break;
			case 'k':	keep = 1;
						break;
			case 'h':	usage(argv[0], 0);
						break;
			case 'o':   zip_filename = optarg;
						break;
			case 's':   max_size = str2size(optarg);
						break;
			case ':':  
				fprintf(stderr, "Option -%c requires a value\n", optopt);
				return 1;
			case '?':  
				fprintf(stderr, "unknown option: %c\n", optopt); 
				return 1;
		}  
	}  
	
	if (zip_filename == NULL) usage(argv[0], 1);

	if (optind == argc){
		fprintf(stderr, "DIRECTORY argument(s) required\n");
		return 1;
	}

	/* init fbuffer & pfnext pointer */
	fbuffer_size = (max_files+1) * sizeof(char*);
	fbuffer = malloc(fbuffer_size);
	memset(fbuffer, 0x00, fbuffer_size);
   	pfnext = fbuffer;

	/* ... */
	char resolved_path[PATH_MAX];
	char *ptr;
	for(; optind < argc; optind++) {
		realpath(argv[optind], resolved_path);
		if (keep == 0) {
			depth_len = strlen(resolved_path);
		}
		else {
			ptr = strrchr(resolved_path, '/');
			if (ptr == NULL) ptr = resolved_path;
			depth_len = ptr - resolved_path;
		}

		

		printf(">%s\n",resolved_path);
		printf(">%ld - %s\n", depth_len, &resolved_path[depth_len]);

		dir_walker(resolved_path, &add_file_cb, NULL);
		
	} 
	if ( *fbuffer != NULL ) {
		printf("xxx\n");
		do_zip();
		clear_buffer(fbuffer, fbuffer_size);
	}

	free(fbuffer); 
	return 0;
}

/*
vim: ts=4:sw=4:
*/

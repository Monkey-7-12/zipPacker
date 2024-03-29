
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
#include <unistd.h> 
#include <ctype.h> 

#include <pthread.h>
#include <signal.h>

#include "strsize.h"
#include "fsize.h"
#include "fbuffer.h"
#include "dirwalk.h"

// PATH_MAX
#ifdef __linux__
	#include <linux/limits.h>
#elif __APPLE__
	#include <sys/syslimits.h>
#endif

#define SLEEP_TIME           2		// Wait n seconds for thread response
#define MAX_THREADS          4		// Default max threads
#define MAX_SIZE    1073741824 		// Default max size per zip 


static int      max_files = 0;			// Max files per zip file
static uint64_t max_size  = MAX_SIZE;	// Max size per zip file

/* zip name control */
static int  skip_dot_files = 0;		// don't archive dot-files (-x)
static long depth_len = 0;		  	// count character do delete from input filename
static char *zip_filename = NULL;	// output filename (format mask)

/* working ....... */
static int      zipfile_idx = 0;	// index for zipfile name _0001, _0002 ... ... ...
static int	    total_files = 0;	// total files found
static uint64_t total_size = 0;		// total size for found files	

/* thread control */
// FIXME / TODO : mutex
struct thread_args {
	fBuffer_t *array;				// array of files
	char zipfile[PATH_MAX+1];		// zip filename
};
static int max_threads = MAX_THREADS;
static pthread_t *threads;		  	// array of thread id's --- TODO: "replace/control" with mutex

/* fBuffer */
static fBuffer_t *pfBuffer;			// pointer to start of array of files
static fBuffer_t *pfBuffer_last;	// pointer to last entry



// FIXME
int wait_thread() {
	for (int i=0; i<max_threads; i++) {
		if (threads[i] == 0) return i;
	}
	/* wait and print ....... */
	while (1) {
		printf(".");
		fflush(stdout);
		sleep(SLEEP_TIME);
		for (int i=0; i<max_threads; i++) {
			if (pthread_kill(threads[i], 0) != 0) {
				printf("\n");
				threads[i] = 0;
				return i;
			}
		}
	}
}


struct cleanup_handler_args {
	pthread_t 	thread_id;	
	fBuffer_t 	*array;
};
static void pt_cleanup_handler(void *p) {
	struct cleanup_handler_args *args = (struct cleanup_handler_args *)p;
	#ifdef DEBUG
		printf("  >>> exit_thread(%ld, ...)\n", (long)args->thread_id);
	#endif /* DEBUG */
	fbuffer_free(args->array);
}

void* pt_zipper(void *p) {
	struct thread_args *args = (struct thread_args *)p;
	
	struct cleanup_handler_args cleanup_args;
	cleanup_args.array = args->array;
	cleanup_args.thread_id = pthread_self();
	pthread_cleanup_push(pt_cleanup_handler, &cleanup_args);
	
	int error;
	zip_t *zip = zip_open(args->zipfile, ZIP_CREATE | ZIP_EXCL, &error);
	if (zip == NULL) {
		zip_error_t ziperror;
		zip_error_init_with_code(&ziperror, error);
		fprintf(stderr, "Failed to open output file %s: %s\n", args->zipfile, zip_error_strerror(&ziperror));
		exit(3);
	}

	char *name;
	zip_int64_t id;
	fBuffer_t *ptr = args->array;
	fBuffer_t *p;
	while (ptr) {
		p = ptr;
		ptr = ptr->_next;
		name = &p->path[depth_len];
	
		if (name[0] == '/') name++; /* stripp absolute path spec from name */

		/* add directory... */
		if (S_ISDIR(p->st_mode)) {
			id = zip_dir_add(zip, name, ZIP_FL_ENC_UTF_8);
			if (id < 0) {
				fprintf(stderr, "Failed to add directory %s to zip: %s\n", name, zip_strerror(zip));
				exit(4);
			}
			continue;
	   	}	
			
		/* add file... */
		#ifdef DEBUG
			fprintf(stderr, "DEBUG: zip: %s, file: %s\n", args->zipfile, p->path);
		#endif /* DEBUG */
		zip_source_t *source = zip_source_file(zip, p->path, 0, 0);
		if (source == NULL) {
			fprintf(stderr, "Failed to open file %s: %s\n", p->path, zip_strerror(zip));
			exit(3);
		}

		id = zip_file_add(zip, name, source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8); 
		if (id < 0) {
			zip_source_free(source);
			fprintf(stderr, "Failed to add file %s to zip: %s\n", name, zip_strerror(zip));
			exit(4);
		} 

		if (zip_set_file_compression(zip, id, ZIP_CM_STORE, 0) < 0) {
			fprintf(stderr, "Failed to set compressions flag for file %s: %s\n", name, zip_strerror(zip));
			exit(5);
		}
	}
	if (zip_close(zip) < 0) {
		zip_error_t *ziperror = zip_get_error(zip);
		fprintf(stderr, "Failed to write output file %s: %s\n", args->zipfile, zip_error_strerror(ziperror));
		exit(1);
	}


	pthread_exit(NULL);
	pthread_cleanup_pop(1);
	return NULL;
}


void do_zip() {
	struct thread_args *args = malloc(sizeof(struct thread_args));

	args->array = pfBuffer;

	zipfile_idx++;
	sprintf(args->zipfile  , zip_filename, zipfile_idx);	

	int idx = wait_thread();
	printf("start thread idx: %d", idx+1);
	pthread_create(&threads[idx], NULL, (void*)(void*)pt_zipper, args);
	printf(" (%ld) - %s\n", (long)threads[idx], args->zipfile);
}


/* add_file_cb callback */
void add_file_cb(char* fname, mode_t st_mode, off_t st_size) {
	//printf("add_file_cb: %s, %ld\n", fname, st_size);
	
	if (st_size < 0) {
		// TODO / FIXME 
		#ifdef __linux__
			printf("error: size = %ld - %s\n", st_size, fname);
		#elif __APPLE__
			printf("error: size = %lld - %s\n", st_size, fname);
		#endif
		exit(6);
	}

	
	#ifdef DEBUG
	fprintf(stderr, "DEBUG: add_file_cb: %s, (%s)\n", fname, S_ISDIR(st_mode) ? "Directory" : "File");
	#endif /* DEBUG */
	if ((max_size > 0 && total_size + st_size >= max_size) || (max_files > 0 && total_files >= max_files)) {
		#ifdef DEBUG
			fprintf(stderr, "DEBUG: add_file_cb: >max_size/total_size  : %ld/%ld\n", max_size, total_size);
	   		fprintf(stderr, "DEBUG: add_file_cb: >max_files/total_files: %d/%d\n", max_files, total_files);
		#endif /* DEBUG */
		do_zip();
		pfBuffer = NULL;
		pfBuffer_last = pfBuffer;
		total_files = 0;
		total_size = 0;
	}

	if (pfBuffer == NULL) {
		pfBuffer = fbuffer_new();
		pfBuffer_last = pfBuffer;
	}
	else
	{
		pfBuffer_last = fbuffer_append(pfBuffer_last);
	}
	pfBuffer_last->path = strdup(fname);
	pfBuffer_last->st_mode = st_mode;
	pfBuffer_last->st_size = st_size;
	if (!S_ISDIR(st_mode)) {
		total_files++;
		total_size += st_size;
	}
}


void usage(char* prog, int exitcode) {
	printf(
		"usage: %s Options DIRECTORY [DIRECTORY ...]\n\n"
		"Options:\n"
		"  -c MAX_FILES   Set max files for zip archives\n"
		"  -j THREADS     Max threads\n"
		"                 Default: %d\n"
		"  -k             keep (last)DIRECTORY name in zip file\n"
		"  -x             don't archive dot-files (/.*)\n"
		"  -o FILENAME    Output zip-filename format string\n"
		"                 e.g.: /tmp/test_%%03d.zip\n"
		"                 -> /tmp/test_001.zip, /tmp/test_002.zip...\n"
		"  -s SIZE        Max SIZE for zip-archive\n"
		"                 e.g.: -s 52428800 -> = 50M\n"
		"                 -s 25G, -s 100M...\n"
		"                 Default: %s\n"
		, prog, MAX_THREADS, size2str(MAX_SIZE,0)
	);
	exit(exitcode);
}


int main(int argc, char* argv[]) {
	int opt; 
	int keep = 0;
 
	while((opt = getopt(argc, argv, ":c:j:kxho:s:")) != -1)  
	{  
		switch(opt)  
		{  
			case 'c':   max_files = strtol(optarg, NULL, 0);
						break;
			case 'j':	max_threads = strtol(optarg, NULL, 0);
						break;
			case 'k':	keep = 1;
						break;
			case 'x':	skip_dot_files = 1;
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

	/* ---------------------------------------------------------------- */
	pfBuffer = NULL;	

	/* init threads */
	threads = calloc(max_threads, sizeof(pthread_t));

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

		dir_walker(resolved_path, skip_dot_files, (void(*)())add_file_cb, (void(*)())add_file_cb);
	}
	if (pfBuffer) do_zip();

	/* wait for threads */
	int count;
	do {
		printf("wait for thread(s) ");
		count = 0;
		for (int i=0; i<max_threads; i++) {
			if (threads[i] == 0) continue;
			if (pthread_kill(threads[i], 0) == 0) {
				printf("%d ", i+1);
				count++;
			} else threads[i] = 0;
		}
		if (count > 0) {
			printf("- %d thread(s) running...", count);
			sleep(SLEEP_TIME);
		}
		else printf("...finish");
		printf("\n");
	} while (count > 0);

	return 0;
}

/*
vim: ts=4:sw=4:
*/

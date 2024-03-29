#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"


/* Returns the size of filename (in bytes). */
int get_file_size(char *filename) {

    struct stat sbuf;

    if ((stat(filename, &sbuf)) == -1) {
        perror("stat");
        exit(1);
    }

    return sbuf.st_size;
}

/* A comparison function to use for qsort. */
int compare_freq(const void *rec1, const void *rec2) {

    struct rec *r1 = (struct rec *) rec1;
    struct rec *r2 = (struct rec *) rec2;

    if (r1->freq == r2->freq) {
        return 0;
    } else if (r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}

/* Returns the index of the record struct with the smallest 
 * frequency in the array pointed to by p and of size n. 
 */
int get_smallest_index(struct rec *p, int n) {
    int i;
    int smallest_freq = INT_MAX, smallest_index = -1;

    for (i = 0; i < n; i++) {
        if ((p + i)->freq != -1 && (p + i)->freq < smallest_freq) {
            smallest_freq = (p + i)->freq;
            smallest_index = i;
        }
    }
    return smallest_index;
}


/* The wrapper function for close */
void Close(int fd) {
    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }
}

/* The wrapper function for the malloc */
void *Malloc(int size) {
    void *ptr;

    if ((ptr = malloc((size_t) size)) == NULL) {
        perror("malloc");
        exit(1);
    }
    return ptr;
}


/* The wrapper function for fopen */
FILE *Fopen(char *source, char *mode) {
    FILE *fp;
    if ((fp = fopen(source, mode)) == NULL) {
        perror("fopen");
        exit(1);
    }
    return fp;
}

/* The wrapper function for fclose */
void Fclose(FILE *fp) {
    if (fclose(fp) != 0) {
        perror("fclose");
        exit(1);
    }
}



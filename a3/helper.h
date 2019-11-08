#ifndef _HELPER_H
#define _HELPER_H

#define SIZE 44

struct rec {
    int freq;
    char word[SIZE];
};

int get_file_size(char *filename);

int compare_freq(const void *rec1, const void *rec2);

int get_smallest_index(struct rec *p, int n);

void Close(int fd);

void *Malloc(int size);

FILE *Fopen(char *source, char *mode);

void Fclose(FILE *fp);


#endif /* _HELPER_H */

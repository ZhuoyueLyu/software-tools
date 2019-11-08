#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
//#include <sys/time.h>
#include "helper.h"

int main(int argc, char **argv) {

//    struct timeval starttime, endtime;
//    double timediff;
//    if ((gettimeofday(&starttime, NULL)) == -1) {
//        perror("gettimeofday");
//        exit(1);
//    }

    char ch;
    char *input_file = NULL, *output_file = NULL;
    int i, j;
    int n = 0;
    int rec_size = sizeof(struct rec), file_size;
    int pid;
    int result = 0;

    /* Check for the correct number of arguments. */
    if (argc != 7) {
        fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
        exit(1);
    }

    /* Read in all the arguments. */
    while ((ch = (char) getopt(argc, argv, "n:f:o:")) != -1) {
        switch (ch) {
            case 'n':
                n = atoi(optarg);
                break;
            case 'f':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
                exit(1);
        }
    }

    if (n == 0 || input_file == NULL || output_file == NULL) {
        fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
        exit(1);
    }
    /* Check if file size is 0. */
    if ((file_size = get_file_size(input_file))== 0) {
        fprintf(stderr, "%s: file empty\n", input_file);
        exit(0);
    }
    
    /* Check if file size is a multiple of the struct rec size. */
    if ((file_size = get_file_size(input_file)) % rec_size != 0) {
        fprintf(stderr, "%s: invalid input file\n", input_file);
        exit(1);
    }

    int num_recs = file_size / rec_size;

    /* avoid the case where some child process get 0 rec*/
    if (n >= num_recs) {
        n = num_recs;
    }

    int recs_remainder = num_recs % n;
    int status, fd[n][2];

    for (i = 0; i < n; i++) {
        if (pipe(fd[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) { /* The Child Process */
            /* Close read and write ends of all unused pipes. */
            for (j = 0; j < i; j++) {
                Close(fd[j][0]);
                Close(fd[j][1]);
            }

            /* Close read end of the used pipe */
            close(fd[i][0]);

            int recs_per_process;
            FILE *fp;

            if (i < recs_remainder) {
                recs_per_process = (num_recs / n) + 1;
            } else {
                recs_per_process = (num_recs / n);
            }
            /* The malloc space for every Children */
            struct rec *records = (struct rec *) Malloc(rec_size * recs_per_process);
            fp = Fopen(input_file, "r");
            int index = 0;

            if (i < recs_remainder) {/* first several processes */
                index = i * recs_per_process;
            } else {/* last several processes that has less recs assigned */
                index = recs_remainder * (recs_per_process + 1) + recs_per_process * (i - recs_remainder);
            }
            /* The place in the file where this child should start reading*/
            if (fseek(fp, index * rec_size, SEEK_SET) != 0) {
                perror("fseek");
                exit(1);
            }
            /* Read all the recs that this child should read*/
            if (fread(records, sizeof(struct rec), (size_t) recs_per_process, fp) == 0) {
                perror("fread");
                exit(1);
            }
            /* Sort the recs by their frequencies, from the smallest to the largest */
            qsort(records, (size_t) recs_per_process, (size_t) rec_size, compare_freq);

            for (j = 0; j < recs_per_process; j++) {
                if (write(fd[i][1], &records[j], (size_t) rec_size) == -1) {
                    perror("write");
                    exit(1);
                }
            }
            Close(fd[i][1]);
            Fclose(fp);
            free(records);
            exit(0);

        }
    }
    /* Close writing end of all pipes. */
    for (i = 0; i < n; i++) {
        Close(fd[i][1]);
    }

    int smallest_index, bytes_read, num_empty_pipes = 0;
    FILE *fp;
    struct rec merge_list[n];
    fp = Fopen(output_file, "w");

    /* Push the first element of each children into our merge list*/
    for (i = 0; i < n; i++) {
        if ((bytes_read = (int) read(fd[i][0], &merge_list[i], (size_t) rec_size)) == -1) {
            perror("read");
            exit(1);
        } else if (bytes_read == 0) {/* For the last several elements of each child*/
            merge_list[i].freq = -1;
            num_empty_pipes++;
        }
    }

    /* Push the smallest number in merge list*/
    while (num_empty_pipes < n) {
        smallest_index = get_smallest_index(merge_list, n);
        if (fwrite(&merge_list[smallest_index], (size_t) rec_size, 1, fp) == 0) {
            perror("fwrite");
            exit(1);
        }
        /* Push the next element of that child to the position of smallest_index*/
        if ((bytes_read = (int) read(fd[smallest_index][0], &merge_list[smallest_index], (size_t) rec_size)) == -1) {
            perror("read");
            exit(1);
        } else if (bytes_read == 0) {
            merge_list[smallest_index].freq = -1;
            num_empty_pipes++;
        }
    }

    /* Close reading end of all pipes. */
    for (i = 0; i < n; i++) {
        Close(fd[i][0]);
    }

    Fclose(fp);

    /* When the child failed. */
    for (i = 0; i < n; i++) {
        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {/* if the child process exit prematurely*/
                fprintf(stderr, "Child process terminated prematurely\n");
            }
        } else if (WIFSIGNALED(status)) {/* if the child process exit with signal*/
            fprintf(stderr, "Child terminated abnormally\n");
            result = 1;
        } else {
            printf("Shouldn't get here\n");
        }
    }

//    if ((gettimeofday(&endtime, NULL)) == -1) {
//        perror("gettimeofday");
//        exit(1);
//    }
//    timediff = (endtime.tv_sec - starttime.tv_sec) +
//               (endtime.tv_usec - starttime.tv_usec) / 1000000.0;
//    fprintf(stdout, "%.4f\n", timediff);

    return result;
}


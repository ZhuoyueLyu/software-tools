/* The purpose of this program is to get some practice writing signal handling
 * functions and observing the behaviour of signals.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/* Messages to print in the signal handling function. */
#define MESSAGE1 "%d reads were done\n"
#define MESSAGE2 "%d reads were done in %d seconds.\n"

/* global variables to store number of read operations and seconds elapsed. You won't use
 * the seconds variable until you add the timer in step 4.
 */
int num_reads, seconds;

void handler(int code) {
    fprintf(stderr, MESSAGE2, num_reads, seconds);
    exit(1);
}


/* The first command line argument is the number of seconds to set a timer to run.
 * The second argument is the name of a binary file containing 100 ints.
 * Assume both of these arguments are correct.
 */

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: time_reads s filename\n");
        exit(1);
    }
    seconds = strtol(argv[1], NULL, 10);

    FILE *fp;
    if ((fp = fopen(argv[2], "r")) == NULL) {
      perror("fopen");
      exit(1);
    }

	/* In an infinite loop, read an int from a random location in the file, and print
	 * it to stderr.
	 */
    struct sigaction sa;
    struct itimerval curr;

    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    
    curr.it_value.tv_sec = seconds;
    curr.it_value.tv_usec = 0;
    curr.it_interval.tv_sec = seconds;
    curr.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &curr, NULL) < 0) {
        fprintf(stderr, "There is error in getitimer\n");
        exit(1);
    }

    int num;
    for (;;) {
        int offset = (random() % 100);
        if (fseek(fp, offset * sizeof(int), SEEK_SET) != 0) {
            perror("seek");
            exit(1);
        }
        if (fread(&num, 1, sizeof(int), fp) <= 0) {
            perror("read");
            exit(1);
        }
        fprintf(stderr, "%d\n", num);
        num_reads++;
    }
    return 1; // something is wrong if we ever get here!
}

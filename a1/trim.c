#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Reads a trace file produced by valgrind and an address marker file produced
 * by the program being traced. Outputs only the memory reference lines in
 * between the two markers
 */

int main(int argc, char **argv) {
    
    if(argc != 3) {
         fprintf(stderr, "Usage: %s tracefile markerfile\n", argv[0]);
         exit(1);
    }

    // Addresses should be stored in unsigned long variables
    // unsigned long start_marker, end_marker;

	unsigned long start_marker;
	unsigned long  end_marker;	
	unsigned long current_marker;
	char operation;
	int memory_size;
	int signal = 0;

	FILE *fp = fopen(argv[2], "r");
	fscanf(fp, "%lx %lx", &start_marker, &end_marker);
	fclose(fp);

    /* For printing output, use this exact formatting string where the
     * first conversion is for the type of memory reference, and the second
     * is the address
     */
    // printf("%c,%#lx\n", VARIABLES TO PRINT GO HERE);
	FILE *ptr = fopen(argv[1], "r");
	while (fscanf(ptr, " %c %lx,%d\n", &operation, &current_marker, &memory_size) != EOF){
		if (current_marker == end_marker){
			signal = 0;
		}
		if (current_marker == start_marker){
                    signal = 1;
                }
		else if (signal == 1){ 
			printf("%c,%#lx\n", operation, current_marker);	
		}
	}

	fclose(ptr);

    return 0;
}

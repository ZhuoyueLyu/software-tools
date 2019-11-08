#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants that determine that address ranges of different memory regions

#define GLOBALS_START 0x400000
#define GLOBALS_END   0x700000
#define HEAP_START   0x4000000
#define HEAP_END     0x8000000
#define STACK_START 0xfff000000

int main(int argc, char **argv) {
    
    FILE *fp = NULL;

    if(argc == 1) {
        fp = stdin;

    } else if(argc == 2) {
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("fopen");
            exit(1);
        }
    } else {
        fprintf(stderr, "Usage: %s [tracefile]\n", argv[0]);
        exit(1);
    }

    /* Complete the implementation */

	char type;
	unsigned long marker;
	int count_TI = 0, count_TM = 0, count_TL = 0, count_TS = 0, count_LG = 0, count_LH = 0, count_LS = 0;


	while (fscanf(fp, "%c,%lx\n", &type, &marker) != EOF){

		if (type == 'I'){
			count_TI++;
		} else{
			if (type == 'M'){
				count_TM++;
			} else if (type == 'L'){
				count_TL++;
			} else if (type == 'S'){
				count_TS++;
			}
			if (GLOBALS_START <= marker && marker <= GLOBALS_END){
				count_LG++;
			} else if (HEAP_START <= marker && marker <= HEAP_END){
				count_LH++;
			} else if (marker >= STACK_START){
				count_LS++;
			}
		}
	}

	fclose(fp); 

    /* Use these print statements to print the ouput. It is important that 
     * the output match precisely for testing purposes.
     * Fill in the relevant variables in each print statement.
     * The print statements are commented out so that the program compiles.  
     * Uncomment them as you get each piece working.
     */
    
    printf("Reference Counts by Type:\n");
    printf("    Instructions: %d\n", count_TI);
    printf("    Modifications: %d\n", count_TM);
    printf("    Loads: %d\n", count_TL);
    printf("    Stores: %d\n", count_TS);
    printf("Data Reference Counts by Location:\n");
    printf("    Globals: %d\n", count_LG);
    printf("    Heap: %d\n", count_LH);
    printf("    Stack: %d\n", count_LS);
 

    return 0;
}

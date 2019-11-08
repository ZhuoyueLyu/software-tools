#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "life_helpers.c"

void print_state(char *state, int size);
void update_state(char *state, int size);


int main(int argc, char **argv) {

    if (argc != 3) {
    	fprintf(stderr, "Usage: USAGE: life initial n\n");
    	return 1;
    }
    //this is the length of the expression
    int size = strlen(argv[1]);
    //this is the number of states that we need to print
    int num_states = strtol(argv[2], NULL, 10);

    int i;

    for(i = 0; i < num_states; i++){
        print_state(argv[1], size);
        update_state(argv[1], size);
    }
    return 0;

}
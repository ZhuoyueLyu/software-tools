#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_state(char *state, int size){
    //This function's job is to print the characters of the 
    //given array following by a single newline character.
    printf("%s\n", state);
}

void update_state(char *state, int size){
//his function should update the state of the array according to the following rules:
//the first and last elements in the array never change
// an element whose immediate neighbours are either both "." or both "X" will become "."
// an element whose immediate neighbours are a combination of one "." and one "X" will become "X"
    int i;
    char temp[size + 1];
    strncpy(temp, state, sizeof(temp));
    temp[size] = '\0';
    for(i = 0; i <= size-1; i++){
        if(i == 0 || i == size - 1){
            ;
        }
        else if(temp[i-1] == temp[i+1]){
            state[i] = '.';
        }
        else{
            state[i] = 'X';
        }
    }
}
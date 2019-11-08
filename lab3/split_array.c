#include <stdio.h>
#include <stdlib.h>

/* Return a pointer to an array of two dynamically allocated arrays of ints.
   The first array contains the elements of the input array s that are
   at even indices.  The second array contains the elements of the input
   array s that are at odd indices.

   Do not allocate any more memory than necessary.
*/
int **split_array(const int *s, int length) {
    // this allocates space for the 2 pointers
    int **pointers = malloc(sizeof(int *) * 2); 
    int size_of_even = (length % 2) + (length / 2);
    int size_of_odd = length / 2;
    int i, even_count, odd_count;
    even_count = 0;
    odd_count = 0;
    // the first pointer points to a single integer
    pointers[0] = malloc(sizeof(int) * size_of_even);
    // the second pointer pointes to an array of 3 integers
    pointers[1] = malloc(sizeof(int) * size_of_odd);

    for(i = 0; i <= length-1; i++){
        //The case that the index is an even number
        if(i % 2 == 0){
            pointers[0][even_count] = s[i];
            even_count++;
        }
        //The case that the index is an odd number
        else{
            pointers[1][odd_count] = s[i];
            odd_count++;
        }
    }

    return pointers;

}

/* Return a pointer to an array of ints with size elements.
   - strs is an array of strings where each element is the string
     representation of an integer.
   - size is the size of the array
 */

int *build_array(char **strs, int size) {
    int *int_arr = malloc(sizeof(int)*size);
    int i;	
    for (i = 0; i <= size - 1; i++) {
        int_arr[i] = strtol(strs[i+1], NULL, 10);
    }
    return int_arr;
}


int main(int argc, char **argv) {
    /* Replace the comments in the next two lines with the appropriate
       arguments.  Do not add any additional lines of code to the main
       function or make other changes.
     */
    int *full_array = build_array(argv, argc-1);
    int **result = split_array(full_array, argc-1);

    printf("Original array:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%d ", full_array[i]);
    }
    printf("\n");

    printf("result[0]:\n");
    for (int i = 0; i < argc/2; i++) {
        printf("%d ", result[0][i]);
    }
    printf("\n");

    printf("result[1]:\n");
    for (int i = 0; i < (argc - 1)/2; i++) {
        printf("%d ", result[1][i]);
    }
    printf("\n");
    free(full_array);
    free(result[0]);
    free(result[1]);
    free(result);
    return 0;
}

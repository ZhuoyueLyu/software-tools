#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "family.h"

/* Number of word pointers allocated for a new family.
   This is also the number of word pointers added to a family
   using realloc, when the family is full.
*/
static int family_increment = 0;


/* Set family_increment to size, and initialize random number generator.
   The random number generator is used to select a random word from a family.
   This function should be called exactly once, on startup.
*/
void init_family(int size) {
    family_increment = size;
    srand(time(0));
}


/* Given a pointer to the head of a linked list of Family nodes,
   print each family's signature and words.

   Do not modify this function. It will be used for marking.
*/
void print_families(Family *fam_list) {
    int i;
    Family *fam = fam_list;

    while (fam) {
        printf("***Family signature: %s Num words: %d\n",
               fam->signature, fam->num_words);
        for (i = 0; i < fam->num_words; i++) {
            printf("     %s\n", fam->word_ptrs[i]);
        }
        printf("\n");
        fam = fam->next;
    }
}


/* Return a pointer to a new family whose signature is 
   a copy of str. Initialize word_ptrs to point to 
   family_increment+1 pointers, numwords to 0, 
   maxwords to family_increment, and next to NULL.
*/
Family *new_family(char *str) {
    int len = (int) strlen(str);
    Family *ptr = malloc(sizeof(Family));
    if (ptr == NULL) {
        perror("malloc");
        exit(1);
    }
    ptr->signature = malloc(sizeof(char *) * (len + 1));
    if (ptr->signature == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(ptr->signature, str);
    ptr->word_ptrs = malloc(sizeof(char *) * (family_increment + 1));
    if (ptr->word_ptrs == NULL) {
        perror("malloc");
        exit(1);
    }
    ptr->word_ptrs[family_increment] = NULL;
    ptr->num_words = 0;
    ptr->max_words = family_increment;
    ptr->next = NULL;
    return ptr;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) {
    if (fam->num_words >= fam->max_words) {
        fam->max_words += family_increment;
        fam->word_ptrs = realloc(fam->word_ptrs, sizeof(char *) * (fam->max_words + 1));
        if (fam->word_ptrs == NULL) {
            perror("realloc");
            exit(1);
        }
    }
    fam->word_ptrs[fam->num_words] = word;
    fam->num_words += 1;
}


/* Return a pointer to the family whose signature is sig;
   if there is no such family, return NULL.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_family(Family *fam_list, char *sig) {
    Family *curr = fam_list;
    while (curr) {
        if (strcmp(curr->signature, sig) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}


/* Return a pointer to the family in the list with the most words;
   if the list is empty, return NULL. If multiple families have the most words,
   return a pointer to any of them.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_biggest_family(Family *fam_list) {
    int largest_num = 0;
    Family *temp = NULL;
    Family *curr = fam_list;
    while (curr) {
        if (curr->num_words > largest_num) {
            largest_num = curr->num_words;
            temp = curr;
        }
        curr = curr->next;
    }
    return temp;
}


/* Deallocate all memory rooted in the List pointed to by fam_list. */
void deallocate_families(Family *fam_list) {
    Family *temp;
    while (fam_list) {
        temp = fam_list;
        free(temp->signature);
        free(temp->word_ptrs);
        fam_list = fam_list->next;
        free(temp);
    }

}


/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
    Family *famhead = NULL;
    // The length of each word in the list
    int length = (int) strlen(word_list[0]);
    // The character array to store the signature
    char signature[length + 1];
    // Set the last character to be null terminator
    signature[length] = '\0';
    // Set family_increment to the length by init_family
    init_family(length);
    // Looping over the word list
    int i = 0;
    while (word_list[i] != NULL) {
        // Check the signature of every words in the list
        for (int j = 0; j < length; j++) {
            if (word_list[i][j] == letter) {
                signature[j] = letter;
            } else {
                signature[j] = '-';
            }
        }
        // The first family
        if (i == 0) {
            famhead = new_family(signature);
            add_word_to_family(famhead, word_list[i]);
        }
            // If it's not the first family and
            // the family of this signature does not exist before
            // We create a new family
        else {
            // Set tem_family to be the result of find_family
            Family *tem_family = find_family(famhead, signature);
            if (tem_family == NULL) {
                Family *curr = famhead;
                while (curr->next != NULL) {
                    curr = curr->next;
                }
                curr->next = new_family(signature);
                add_word_to_family(curr->next, word_list[i]);
            }
                // If it's not the first family and
                // the family of that signature does exist
                // We add word to that family
            else {
                add_word_to_family(tem_family, word_list[i]);
            }
        }
        i++;
    }
    return famhead;
}


/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) {
    return fam->signature;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
    //Create a new word pointer by independently malloc 
    char **new_word_ptrs = malloc(sizeof(char *) * (fam->max_words + 1));
    if (new_word_ptrs == NULL) {
        perror("malloc");
        exit(1);
    }
    //Set the final pointer be NULL
    new_word_ptrs[fam->num_words] = NULL;
    //Make every single pointers in new_word_ptrs point to the same word
    //as fam->word_ptrs do
    for (int i = 0; i < fam->num_words; i++) {
        new_word_ptrs[i] = fam->word_ptrs[i];
    }
    return new_word_ptrs;
}


/* Return a pointer to a random word from fam. 
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
    // Initialization, should only be called once.
    srand(time(NULL));
    // Set r to be a random integer between 0 to fam->num_words - 1
    // And since all word families will contain at least one word, we start with index 0
    int r = rand() % (fam->num_words);
    return fam->word_ptrs[r];
}

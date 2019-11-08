#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#include "socket.h"
#include "gameplay.h"


#ifndef PORT
#define PORT 51111
#endif
#define MAX_QUEUE 5


void add_player(struct client **top, int fd, struct in_addr addr);

void remove_player(struct client **top, int fd);

void broadcast(struct game_state *game, char *outbuf);

void announce_turn(struct game_state *game);

void announce_winner(struct game_state *game, struct client *winner);

void advance_turn(struct game_state *game);

int read_input(int clientfd, char *buff, int size);

void disconnect(struct game_state *game, int fd, char *name);

void new_game(char *const *argv, struct game_state *game);

fd_set allset;


/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf) {
    struct client *curr = game->head;
    while (curr) {
        if (write(curr->fd, outbuf, strlen(outbuf)) == -1) { // Drop connection
            char name[strlen(curr->name) + 1];
            strcpy(name, curr->name);
            disconnect(game, curr->fd, name);
        }
        curr = curr->next;
    }
}

/* Announce who is the current player*/
void announce_turn(struct game_state *game) {
    /* Ask the current player to enter the move */
    char *msg_guess = "Your guess?\r\n";
    if (write(game->has_next_turn->fd, msg_guess, strlen(msg_guess)) == -1) { // Drop connection
        char name[strlen(game->has_next_turn->name) + 1];
        strcpy(name, game->has_next_turn->name);
        disconnect(game, game->has_next_turn->fd, name);
    }

    /* Tell everyone else */
    char msg_other[MAX_MSG];
    sprintf(msg_other, "It's %s's turn.\n", game->has_next_turn->name);
    printf("%s", msg_other);// Tell the server
    sprintf(msg_other, "It's %s's turn.\r\n", game->has_next_turn->name);
    struct client *curr = game->head;
    while (curr) {
        if (curr != game->has_next_turn) {
            if (write(curr->fd, msg_other, strlen(msg_other)) == -1) { // Drop connection
                char name[strlen(curr->name) + 1];
                strcpy(name, curr->name);
                disconnect(game, curr->fd, name);
            }
        }
        curr = curr->next;
    }
}

/* Announce the winner*/
void announce_winner(struct game_state *game, struct client *winner) {

    /* Tell the words and broadcast*/
    char msg_other[MAX_MSG];
    sprintf(msg_other, "The word was %s.\r\n", game->word);
    broadcast(game, msg_other);

    /* Ask the current player to enter the move */
    char *msg_win = "Game over! You win!\r\n";
    if (write(winner->fd, msg_win, strlen(msg_win)) == -1) { // Drop connection
        char name[strlen(winner->name) + 1];
        strcpy(name, winner->name);
        disconnect(game, winner->fd, name);
    }

    /* Tell everyone else */
    sprintf(msg_other, "Game over! %s won!\n", winner->name);
    printf("%s", msg_other);// Tell the server
    sprintf(msg_other, "Game over! %s won!\r\n", winner->name);
    struct client *curr = game->head;
    while (curr) {
        if (curr != game->has_next_turn) {
            if (write(curr->fd, msg_other, strlen(msg_other)) == -1) { // Drop connection
                char name[strlen(curr->name) + 1];
                strcpy(name, curr->name);
                disconnect(game, curr->fd, name);
            }
        }
        curr = curr->next;
    }
}

/* Move the has_next_turn pointer to the next active client */
void advance_turn(struct game_state *game) {
    /*If we reach the tail/first player, we should start from the head*/
    if (game->head == NULL) {
        game->has_next_turn = NULL;
    } else if (game->has_next_turn->next == NULL || game->has_next_turn == NULL) {
        game->has_next_turn = game->head;
    } else {
        game->has_next_turn = game->has_next_turn->next;
    }
}


/*
 *  Helper function: read an input from a player.
 *  Return -2 if read 0 byte (the client disconnect)
 */
int read_input(int clientfd, char *buff, int size) {
    int bytes_read = (int) read(clientfd, buff, (size_t) size);
    if (bytes_read == -1) {// Error checking
        perror("read");
        exit(1);
    } else if (bytes_read == 0) {
        return -2;
    }
    buff[bytes_read] = '\0';
    printf("[%d] Read %d bytes\n", clientfd, bytes_read);
    // it may take more than one read to get all of the data that was written
    while (strstr(buff, "\r\n") == NULL) {
        bytes_read += read(clientfd, &buff[bytes_read], (size_t) (size - bytes_read));
        buff[bytes_read] = '\0';
        printf("[%d] Read %d bytes\n", clientfd, bytes_read);
    }
    buff[bytes_read - 2] = '\0';
    printf("[%d] Found newline %s\n", clientfd, buff);
    return bytes_read;
}

/*
 * If the read/write failed, it means that the client has dropped connection,
 * and we should disconnect it.
 */
void disconnect(struct game_state *game, int fd, char *name) {
    /* broadcast the leaving message*/
    char msg[MAX_MSG];
    printf("%s has left\n", name);
    sprintf(msg, "Goodbye %s\r\n", name);
    /* If the current player drop connection, we should move to the next player*/
    int cur_fd = game->has_next_turn->fd;

    if(cur_fd == fd){
        game->has_next_turn = NULL;
    }
    /* Remove player*/
    remove_player(&(game->head), fd);
    if (cur_fd == fd) { advance_turn(game); }
    if (game->head != NULL) {
        broadcast(game, msg);
        announce_turn(game);
    }
}


/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }
    printf("Adding client %s\n", inet_ntoa(addr));
    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *top;
    *top = p;
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset 
 */
void remove_player(struct client **top, int fd) {
    struct client **p;

    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next);
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Disconnect from %s\n", inet_ntoa((*p)->ipaddr));
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                fd);
    }
}

/*
 * Start a new game when we have a winner or there is no more guesses
 */
void new_game(char *const *argv, struct game_state *game) {
    printf("Evaluating for game_over\n");
    broadcast(game, "\nLet's start a new game!\r\n");
    printf("New game\n");
    init_game(game, argv[1]);
    /* Display the game state */
    char msg[MAX_MSG];
    broadcast(game, status_message(msg, game));
    announce_turn(game);
}


int main(int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p;
    struct sockaddr_in q;
    fd_set rset;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <dictionary filename>\n", argv[0]);
        exit(1);
    }

    // Create and initialize the game state
    struct game_state game;

    srandom((unsigned int) time(NULL));
    // Set up the file pointer outside of init_game because we want to 
    // just rewind the file when we need to pick a new word
    game.dict.fp = NULL;
    game.dict.size = get_file_length(argv[1]);

    init_game(&game, argv[1]);

    // head and has_next_turn also don't change when a subsequent game is
    // started so we initialize them here.
    game.head = NULL;
    game.has_next_turn = NULL;

    /* A list of client who have not yet entered their name.  This list is
     * kept separate from the list of active players in the game, because
     * until the new playrs have entered a name, they should not have a turn
     * or receive broadcast messages.  In other words, they can't play until
     * they have a name.
     */
    struct client *new_players = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, MAX_QUEUE);

    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)) {
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_player(&new_players, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_player(&new_players, clientfd);
            };
        }
        int cur_fd;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if (FD_ISSET(cur_fd, &rset)) {
                // Check if this socket descriptor is an active player
                for (p = game.head; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {//handle input from an active client
                        if (p == game.has_next_turn) { // If the next player is p
                            /* Read from the input*/
                            if (read_input(p->fd, p->inbuf, MAX_WORD) == -2) { // Drop connection
                                char name[strlen(p->name) + 1];
                                strcpy(name, p->name);
                                disconnect(&game, p->fd, name);
                                break;
                            }
                            /* Check if the input is valid or not*/
                            if (strlen(p->inbuf) == 1 && islower(p->inbuf[0])) {//valid guess
                                /* Indicate whether we found a letter or not*/
                                int indicator = 0;
                                for (int i = 0; i < strlen(game.word); i++) {
                                    /*Guess again if the guess is right/have not been guessed before
                                     * and there are guesses left*/
                                    if (p->inbuf[0] == game.word[i] && game.letters_guessed[p->inbuf[0] - 'a'] == 0) {
                                        /* Update the state */
                                        game.guess[i] = p->inbuf[0];
                                        indicator = 1;
                                    }
                                }
                                /* if this is the last letter, announce winner*/
                                int win_indicator = 1;
                                for (int j = 0; j < strlen(game.word); j++) {
                                    if (game.guess[j] == '-') {
                                        win_indicator = 0;
                                    }
                                }
                                if (win_indicator == 1) {// If we reach the winning state
                                    announce_winner(&game, p);
                                    new_game(argv, &game);
                                    break;
                                }
                                /* If the guess is valid but not right/guessed before*/
                                if (indicator == 0) {
                                    char msg_other[MAX_MSG];
                                    sprintf(msg_other, "Letter %s is not in the word\n", p->inbuf);
                                    printf("%s", msg_other);// Tell the server
                                    sprintf(msg_other, "%s is not in the word\r\n", p->inbuf);
                                    if (write(p->fd, msg_other, strlen(msg_other)) == -1) { // Drop connection
                                        char name[strlen(p->name) + 1];
                                        strcpy(name, p->name);
                                        disconnect(&game, p->fd, name);
                                        break;
                                    }
                                    advance_turn(&game);
                                    game.guesses_left -= 1;
                                }
                                /* Update the letter guessed*/
                                game.letters_guessed[p->inbuf[0] - 'a'] = 1;

                                /* Tell everyone the guess*/
                                char msg[MAX_MSG];
                                sprintf(msg, "%s guesses: %s\r\n", p->name, p->inbuf);
                                broadcast(&game, msg);
                                /* Prompt the player whose turn it is and tell the others*/
                                /* Display the game state */
                                broadcast(&game, status_message(msg, &game));
                                announce_turn(&game);

                            } else {//invalid guess
                                char *msg_invalid = "Invalid guess. Please enter again!\r\n";
                                if (write(p->fd, msg_invalid, strlen(msg_invalid)) == -1) {// Drop connection
                                    char name[strlen(p->name) + 1];
                                    strcpy(name, p->name);
                                    disconnect(&game, p->fd, name);
                                    break;
                                }
                                /* Prompt the player whose turn it is and tell the others*/
                                /* Display the game state */
                                char msg[MAX_MSG];
                                broadcast(&game, status_message(msg, &game));
                                announce_turn(&game);
                            }

                        } else {// If the next player is not p
                            /* If p type something when it's not his turn*/
                            int input = read_input(p->fd, p->inbuf, MAX_WORD);
                            if (input == -2) {// Disconnect
                                char name[strlen(p->name) + 1];
                                strcpy(name, p->name);
                                disconnect(&game, p->fd, name);
                                break;
                            } else if (input > 0) {// Tell this client
                                char msg_other[MAX_MSG];
                                sprintf(msg_other, "It is not your turn to guess.\r\n");
                                if (write(p->fd, msg_other, strlen(msg_other)) == -1) { // Drop connection
                                    char name[strlen(p->name) + 1];
                                    strcpy(name, p->name);
                                    disconnect(&game, p->fd, name);
                                    break;
                                }
                                sprintf(msg_other, "Player %s tried to guess out of turn\r\n", p->name);
                                printf("%s", msg_other);
                            }
                        }
                        /* The game ends when the players have zero guess remaining*/
                        if (game.guesses_left <= 0) {
                            /* Tell the words and broadcast*/
                            char msg_other[MAX_MSG];
                            sprintf(msg_other, "No more guesses. The word was %s.\r\n", game.word);
                            broadcast(&game, msg_other);
                            new_game(argv, &game);
                            break;
                        }
                        break;
                    }
                }

                // Check if any new players are entering their names
                for (p = new_players; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {//handle input from an new client who has not entered an acceptable name
                        /* Read from the input*/
                        int input = read_input(p->fd, p->name, MAX_NAME);
                        if (input == -2) {
                            char msg_other[MAX_MSG];
                            sprintf(msg_other, "Client %s disconnected\n", inet_ntoa(q.sin_addr));
                            printf("%s", msg_other);
                            remove_player(&new_players, p->fd);
                            break;
                        }
                        /* Check if the input is empty/already exist*/
                        struct client *curr = game.head;
                        while (curr) {
                            if (strcmp(curr->name, p->name) == 0) {
                                if (write(p->fd, INVALID_MSG, strlen(INVALID_MSG)) ==
                                    -1) {// send invalid message
                                    fprintf(stderr, "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                                    remove_player(&new_players, p->fd);
                                };
                                break;
                            }
                            curr = curr->next;
                        }
                        /* Then we should move this client from new_players to the active list*/
                        struct client *previous = new_players;

                        /* If this is the first player*/
                        if (game.has_next_turn == NULL) {
                            game.has_next_turn = p;
                        }
                        /* If p is the first node in new_player list*/
                        if (previous == p) {
                            new_players = p->next;
                        } else {
                            while (previous && previous->next != p) { previous = previous->next; }
                            previous->next = p->next;
                        }
                        p->next = game.head;
                        game.head = p;
                        /* Broadcast to all the active clients and server */
                        char msg[MAX_MSG];
                        if (game.head != NULL) {
                            sprintf(msg, "%s has just joined.\r\n", game.head->name);
                            printf("%s", msg);
                            broadcast(&game, msg);
                        }
                        /* Prompt the player whose turn it is and tell the others*/
                        /* Display the game state */
                        if (write(cur_fd, status_message(msg, &game), strlen(status_message(msg, &game))) == -1) {
                            char name[strlen(p->name) + 1];
                            strcpy(name, p->name);
                            disconnect(&game, p->fd, name);
                            break;
                        }
                        announce_turn(&game);
                        break;
                    }
                }
            }
        }
    }
    return 0;
}



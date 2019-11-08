#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef PORT
  #define PORT 30000
#endif
#define BUF_SIZE 128

int main(void) {
    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }

    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INETgggggggggg, "127.0.0.1", &server.sin_addr) < 1) {
        perror("client: inet_pton");
        close(sock_fd);
        exit(1);
    }

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        exit(1);
    }


// -------------------------------------- MY STUFF

    printf("what is your username?\n");
    char username[BUF_SIZE];
    if(fgets(username, BUF_SIZE, stdin) == NULL){
        fprintf(stderr, "fgets");
        exit(1);
    }
    int num_written = write(sock_fd, &username, BUF_SIZE);
        if (num_written < 0) {
            perror("client: writing username");
            close(sock_fd);
            exit(1);
        }





// -------------------------------------




int max_fd = sock_fd;
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);
    FD_SET(STDIN_FILENO, &all_fds);




    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.
    char buf[BUF_SIZE + 1];
    while (1) {




         listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server: select");
            exit(1);
        }
        if (FD_ISSET(STDIN_FILENO, &listen_fds)) {
            int num_read = 0;
            if((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) == -1){
                    perror("read");
                    exit(1);
            } else if (num_read == 0){
                break;
            }
            buf[num_read] = '\0';

            if(write(sock_fd, buf, num_read ) == -1){
                perror("write");
                exit(1);
            }
            
            //fgets(buf, BUF_SIZE, stdin);
           // write(sock_fd, buf, BUF_SIZE);
        }
    
          if (FD_ISSET(sock_fd, &listen_fds)) {
            int num_read = 0;
              if((num_read = read(sock_fd, buf, BUF_SIZE)) == -1){
                  perror("read");
                  exit(1);
              }
              buf[num_read] = '\0';
            
            //fgets(buf, BUF_SIZE, sock_fd);
            printf("%s", buf);
            // if( write(STDIN_FILENO, buf, BUF_SIZE) == -1){
            //     perror("write");
            //     exit(1);

            // }
            
        }  


    }

    close(sock_fd);
    return 0;
}

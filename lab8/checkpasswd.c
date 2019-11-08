#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }

  int fd[2], status, n;
  if ((pipe(fd)) == -1 ) {
    perror("pipe");
    exit(1);
  }

  int r = fork();
  if (r < 0) {
    perror("fork");
    exit(1);
  } else if (r > 0) {
    if ((n = write(fd[1], user_id, MAX_PASSWORD)) == -1) {
      perror("write");
      exit(1);
    }
    if ((n = write(fd[1], password, MAX_PASSWORD)) == -1) {
      perror("write");
      exit(1);
    }

    if (close(fd[0]) == -1) {
      perror("close");
    }
    if (close(fd[1]) == -1) {
      perror("close");
    }
    
    wait(&status);
    if (WIFEXITED(status)) {
      int exit_val = WEXITSTATUS(status);
      if (exit_val == 0) {
        printf("%s", SUCCESS);
      } else if (exit_val == 2) {
        printf("%s", INVALID);
      } else if (exit_val == 3) {
        printf("%s", NO_USER);
      } else {
        printf("ERROR: Error happened or the length of id or pass is too long\n");
      }
    }
    
  } else {
    if (dup2(fd[0], fileno(stdin)) == -1) {
      perror("dup2");
      exit(1);
    }
    if (close(fd[0]) == -1) {
      perror("close");
    }
    if (close(fd[1]) == -1) {
      perror("close");
    }
    
    execl("./validate", NULL);
    fprintf(stderr, "ERROR: exec should not return\n");
  }
  return 0;
}

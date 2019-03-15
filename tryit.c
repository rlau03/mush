/*Ryan Lau
 Professor Nico
 tryit: takes a cmd-line arg and forks a child that tries to exec it.
        Then simply return the status of that exec. If it fails, it should
        print why via perror and exit with a non-zero status.*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
        pid_t child;
        int status;
        
        if (argc != 2) {
            fprintf(stderr,"usage: tryit command\n");
            exit(EXIT_FAILURE);
        }

        if ((child = fork())){
            /*parent*/
            if (-1 == wait(&status)){
                perror("wait");
                exit(-1);
            }
            if (WIFEXITED(status)){
                if(!WEXITSTATUS(status)){
                    printf("Process %d succeeded.\n",child);
                }
                else{
                    printf("Process %d exited with an error value.\n",status);
                }
                exit(WEXITSTATUS(status));
            }
            else {
                printf("Process %d exited with an error value.",status);
                return 3;
            }
        }

        else{
            execl(argv[1],argv[1],NULL);
            perror(argv[1]);
            exit(3);
        }
        return 0;
}


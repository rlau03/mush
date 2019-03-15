/*Ryan Lau
 Professor Nico
 forkit: forkit will start itself, announce itself, then split into 
        two processes. The child then announces itself and then exits.
        The parent identifies itself waits for the child, then signs off.*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
        pid_t child;
        int status;
        
        printf("Hello, world!\n");
        if ((child = fork())){
            printf("This is the parent, pid %d.\n", getpid());
            if (-1 == wait(&status)){
                perror("wait");
                exit(-1);
            }
        }

        else{
            printf("This is the child, pid %d.\n", getpid());
            exit(0);
        }

        printf("This is the parent, pid %d, signing off.\n",getpid());

        return 0;
}


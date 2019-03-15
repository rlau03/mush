/*Ryan Lau & Aaron VanderGraff
  Dr. Nico
  Asgn 6: Mush
        This assignment is to make a simple shell. We must implement:
        - Interactive batch processing 
            -> if no arguments, take cmds from stdin until ^D
            -> if args read commands from argv
        - Support for redirection
            -> <, >, and | to feed output of one command to input of another
        - Built-in cd command
            -> ***CD CANNOT BE RUN AS A CHILD PROCESS
        - Support for SIGINT
            -> When ^C is typed, shell doesn't die but terminate any running
                children and reset itself.
*/
#include "parseline.h"

void handler (int signum) {
        /*Kill all children*/
        printf("\nKilling Children \n");

}

int main(int argc, char *argv[]) {
        struct sigaction sa;

        /*Setting up signal*/
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);

        if (argc == 1){
            while(TRUE){
                if (feof(stdin)){
                    break;
                }
                else {
                    parse_stdin();
                }
            }
        }




	return 0;
}

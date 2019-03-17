/*Ryan Lau & Aaron VanderGraaff
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

pid_t main_pid;
int pflag = 0;

void handler (int signum) {
        pid_t current;
        /*Kill all children*/
        current = getpid();
        if (current != main_pid){
            exit(0);
        }
	fflush(NULL);
	if (!pflag) {
	    printf("\n8=====D ");
	    fflush(NULL);
	}
}

int main(int argc, char *argv[]) {
        struct sigaction sa;
	int i;
	FILE *fin;

        main_pid = getpid();
        
	/*Setting up signal*/
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGINT, &sa, NULL);

        if (argc == 1){
            while(TRUE){	
		pflag = 0;
	        printf("8=====D ");
                parse_stdin(NULL, 1, &pflag);
            }
        }
	else {
		fin = fopen(argv[1], "r");
		while(TRUE) {
			pflag = 0;
			printf("8=====D ");
			fflush(NULL);
			parse_stdin(fin, 0, &pflag);
		}
	}
	
	return 0;
}

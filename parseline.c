
#include "parseline.h"


void parse_stdin(char *cmd, int interactive, int *pflag) {
	char *tmp_command = NULL;
	char command[CMD_MAX];
	int num_of_cmds;
	char *piped_cmds[MAX_PIPE];
	int i;
	int j;
	pid_t *children;
	int pipes[MAX_PIPE][2];
	char *tkn;
	char tmpcd[CMD_MAX];
	
	char **temp_args;
	char *args[MAX_PIPE] = {NULL};
	temp_args = args;

	char tmp[CMD_MAX];
	*pflag = 0;
        

	if (interactive) {
        	tmp_command = read_command();
		*pflag = 1;
		strcpy(command, tmp_command);
		free(tmp_command);
	}
	else {
		strcpy(command, cmd);	
	}
	
	/* exit on exit command */
	if (!strcmp(command, "exit\n")) {
		exit(0);
	}
	strcpy(tmpcd, command);
	tkn = strtok(tmpcd, " ");
	if (!strcmp(tmpcd, "cd")) {
		tkn = strtok(NULL, " ");
		tkn[strlen(tkn)-1] = '\0';
		if(chdir(tkn) == -1) {
			perror("chdir");
		}
		return;
	}

	/*
        num_of_cmds = parse(command, piped_cmds, line_args);
	*/
	num_of_cmds = tokenize(command, piped_cmds);
	children = malloc(num_of_cmds*sizeof(pid_t));

	/* make the pipes. Want 1 less than the number of commands pipes */
	for (i=0; i<num_of_cmds-1; i++) {
		if ((pipe(pipes[i]) == -1)) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}


	/* fork my children */
	for (i=0; i<num_of_cmds; i++) {
		if (!(children[i] = fork())) {
			/* child i */
			parse_space(piped_cmds[i], temp_args, tmp);
			/*
			for (j=0; j < (num_of_cmds); j++) {
				printf("  %s  ", temp_args[i]); 
			}
			*/
			
			
			/* check for redirect on first one */
			if (i == 0 && strpbrk(piped_cmds[i], "<")) { 
				redir_in(piped_cmds[i]);
			}
			
			/* check for redirect on last one */
			if (i == num_of_cmds-1 && 
					strpbrk(piped_cmds[i], ">")) {
				/* redirect out */
				redir_out(piped_cmds[i]);
			}
		
			/* move stdouts except for the last one */
			if (i < (num_of_cmds - 1) ) {
				dup2(pipes[i][1], STDOUT_FILENO);
			}

			/* move stdins except for on the first one */
			if (i > 0) {
				dup2(pipes[i-1][0], STDIN_FILENO);
			}	
			/* close extras */
			for (j=0; j<(num_of_cmds-1); j++) {
				close(pipes[j][0]);
				close(pipes[j][1]);
			}
			execvp(temp_args[0], temp_args);
			printf("command: %s not recognized\n", temp_args[0]);
			exit(1);
		}
	}

	for (j=0; j<(num_of_cmds-1); j++) {
		close(pipes[j][0]);
		close(pipes[j][1]);
	}
	
	for (i=0; i<num_of_cmds; i++) {

		wait(NULL);
	}
	free(children);

}

void redir_in(char *command) {
	int infd;
	char tmp[CMD_MAX];
	char *tkn;
	int rflag;
	rflag = 0;

	strcpy(tmp, command);

	tkn = strtok(tmp, " ");
	while ((tkn = strtok(NULL, " "))) {
		if (rflag == 1) {
			break;
		}
		if (*tkn == '<') {
			rflag = 1;
		}
		
	}
       		
	if ((infd = open(tkn, O_RDONLY, 0)) == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	dup2(infd, STDIN_FILENO);
	if (close(infd) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	}
}

void redir_out(char *command) {
	int outfd;
	char tmp[CMD_MAX];
	char *tkn;
	int rflag;
	rflag = 0;

	strcpy(tmp, command);

	tkn = strtok(tmp, " ");
	while ((tkn = strtok(NULL, " "))) {
		if (rflag == 1) {
			break;
		}
		if (*tkn == '>') {
			rflag = 1;
		}
	}

	if ((outfd = open(tkn, O_WRONLY|O_CREAT|O_TRUNC, 
					S_IRUSR|S_IWUSR)) == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}
	dup2(outfd, STDOUT_FILENO);
	if (close(outfd) == -1 ) {
		perror("close");
		exit(EXIT_FAILURE);
	}
}

int parse_space(char *command, char **line_args, char *tmp) {
	char *tkn;
	int i;
	int rflag;
	i = 0;
	rflag = 0;
	strcpy(tmp, command);
	tkn = strtok(tmp, " ");
	line_args[i++] = tkn;
	while((tkn = strtok(NULL, " "))) {
		if (rflag == 1) {
			rflag = 0;
		}
		else if ((*tkn ==  '<') || (*tkn == '>')) {
			rflag = 1;
		}
		else {
			line_args[i++] = tkn;
		}
	}
	return i;
}
	

char *read_command() {
	char buf[SIZE] = {'\0'};
	char *command = NULL; 
	size_t temp;
	size_t len;
	temp = SIZE-1;
	len = 0;

	while (temp==SIZE-1 && buf[SIZE-2]!='\n') {
		/* Read chunks of size SIZE into a buffer */
		if (!fgets(buf, SIZE, stdin)) {
			/*perror("fgets");
			exit(EXIT_FAILURE);*/
                        /* Commented EXIT_FAILURE, fgets returns NULL on ^D*/
                        printf("^D\n");
                        exit(0);
		}
		/* increment our total length by the length of the buffer */
		temp = strlen(buf);
		len += temp;
		/* Allocate space for the command */
		if (!(command = realloc(command, len+1))) {
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		/* put a null terminator at the end of the command before 
		   realloc so strcat's behavior is defined (only needed for 
		   first loop).
		*/
		command[len-temp] = '\0';
		/* Concatenate buf to command */
		strcat(command, buf);
	}
	return command;
}

int count_char(char *s, char c) {
	/* Counts how many occurences of c there are in s, returns the count */

	int i;
	int count;
	count = 0;
	for (i=0; i<strlen(s); i++) {
	    if (s[i] == c) {
	        count++;
	    }
	}
	return count;
}


int tokenize(char *command, char **piped_cmds) {
	/* Tokenizes command by pipes and puts them into piped_cmds array.
	   Also handles errors.
	*/

	int num_of_cmds;
	int i;
	int j;
	int rcount;
	int lcount;
	char *tkn;
        char tmp_cmd[CMD_MAX];
	char *line_args[MAX_ARGS];
	int oflag;
	int iflag;
	rcount = 0;
	lcount = 0;
	i = 0;
	num_of_cmds = 0;

	if (strlen(command) > CMD_MAX) {
	    printf("command too long\n");
	    exit(EXIT_FAILURE);
	}
	if (command[strlen(command)-1] == '\n') {
		command[strlen(command)-1] = '\0'; /*strip newline*/
	}

        tkn = strtok(command, "|");

        while(tkn) {
	    if (i > MAX_PIPE -1) {
		printf("pipeline too deep\n");
		exit(EXIT_FAILURE);
	    }
            piped_cmds[i] = tkn;
            tkn = strtok(NULL, "|");
	    if (tkn && (tkn[0] == ' ' && strlen(tkn) == 1)) {
	        printf("invalid null command\n");
	        exit(EXIT_FAILURE);
	    }
            i++;
            (num_of_cmds)++;
        }

	for (i=0; i < num_of_cmds; i++) {
	    strcpy(tmp_cmd, piped_cmds[i]);
	    tkn = strtok(tmp_cmd, " ");
            if (strpbrk(piped_cmds[i], "<>")){
	        if ((lcount = count_char(piped_cmds[i], '<')) > 1) {
		    printf("%s: bad input redirection\n", tkn);
		    exit(EXIT_FAILURE);
		}
		if ((rcount = count_char(piped_cmds[i], '>')) > 1) {
		    printf("%s: bad output redirection\n", tkn);
		    exit(EXIT_FAILURE);
		}
		if (i > 0 && lcount) {
	            printf("%s: ambiguous input\n", tkn);
		    exit(EXIT_FAILURE);
                }
                if (i != (num_of_cmds - 1) && rcount){
                    printf("%s: ambiguous output\n", tkn);
                    exit(EXIT_FAILURE);
                }
	    }
	    j = 0;
	    oflag = 0;
	    iflag = 0;
	    while(tkn) {
	        if (j > MAX_ARGS-1) {
	    	printf("%s: too many arguments\n", line_args[0]);
	    	exit(EXIT_FAILURE);
	        }
		if (iflag) {
		    if (!strcmp(tkn, "<") || !strcmp(tkn, ">")) {
		        printf("bad input redirection\n");
			exit(EXIT_FAILURE);
		    }
		    else {
		        iflag = 0;
		    }
		}
		if (oflag) {
		    if (!strcmp(tkn, "<") || !strcmp(tkn, ">")) {
		        printf("bad output redirection\n");
			exit(EXIT_FAILURE);
		    }
		    else {
		        oflag = 0;
		    }
		}
		if (!strcmp(tkn, "<")) {
	  	    iflag = 1;
		}
		else if (!strcmp(tkn, ">")) {
		    oflag = 1;
		}
	        line_args[j++] = tkn;
	        tkn = strtok(NULL," ");
	    }
	    if (iflag) {
	        printf("bad input redirection\n");
		exit(EXIT_FAILURE);
	    }
	    else if (oflag) {
	        printf("bad output redirection\n");
		exit(EXIT_FAILURE);
            }
     }
     return num_of_cmds;
}


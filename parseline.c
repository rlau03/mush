
#include "parseline.h"


void parse_stdin() {
	char *command = NULL;
	int num_of_cmds;
	char *piped_cmds[MAX_PIPE];
	int i;
	int j;
	char *line_args[MAX_ARGS];
	pid_t *children;
	int pipes[MAX_PIPE][2];
	
	char **temp_args;
	char *args[MAX_PIPE] = {NULL};
	temp_args = args;

        command = read_command();
        num_of_cmds = parse(command, piped_cmds, line_args);
	children = malloc(num_of_cmds*sizeof(pid_t));

	/* make the pipes. Want 1 less than the number of commands pipes */
	for (i=0; i<num_of_cmds-1; i++) {
		if ((pipe(pipes[i]) == -1)) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}


	/* fork my children */
	/* Need to implement file redirection somewhere */

	for (i=0; i<num_of_cmds; i++) {
		if (!(children[i] = fork())) {
			/* child i */
			parse_space(piped_cmds[i], temp_args);
			for (j=0; j < (num_of_cmds-1); j++) {
				printf("  %s  ", temp_args[i]); 
			}
			/* move stdouts except for the last one */
			if (i < (num_of_cmds - 1)) {
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
	free(command);

}

int parse_space(char *command, char **line_args) {
	char *tkn;
	int i;
	i = 0;
	tkn = strtok(command, " ");
	line_args[i++] = tkn;
	while((tkn = strtok(NULL, " "))) {
		line_args[i++] = tkn;
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

int parse(char *command, char **piped_cmds, char **line_args) {
	/*This function parses the command inputed and prints it out in 
	  different stages of the pipeline, noting the input and output
	  locations and argc and argv array along the way
        */

        /*Tokenizing command line: Storing each command in an array index.
                useful for knowing what stage of process*/ 

        int num_of_cmds = 0; /*This is how many pipe commands there are*/
        int num_of_args = 0; /*This is per pipe command*/
        char tmp_output[CMD_MAX];
        char arg_output[CMD_MAX]="";
        int i=0;
        int j=0;
        char input_str[SIZE];
        char output_str[SIZE];
	char tmp_str[CMD_MAX];
	char tmp[CMD_MAX];
	
	tokenize(command, piped_cmds, &num_of_cmds);
       
        for (i=0; i < num_of_cmds; i++) {
            printf("\n--------\n");
            printf("Stage %d: \"%s\"\n",i, piped_cmds[i]);
            printf("--------\n");
            /*DO PROCESSING IN HERE */
	    strcpy(tmp_str, piped_cmds[i]);
            if (strpbrk(tmp_str, "<>")){
		
		redirect(piped_cmds, line_args, 
				input_str, output_str, i, &j, 
				&num_of_args, num_of_cmds, tmp);
            }
            else {
		no_redirect(piped_cmds, line_args, 
				input_str, output_str, i,
				&j, &num_of_args, num_of_cmds, tmp);
            }
            printf("%12s","input: ");
            printf("%s",input_str);
            printf("%12s","output: ");
            printf("%s",output_str);
            printf("%12s","argc: ");
            printf("%d\n", num_of_args);
            printf("%12s","argv: ");
	    strcpy(arg_output, "\0");
            for (j=0; j < num_of_args; j++) {
                    sprintf(tmp_output, "\"%s\",", line_args[j]);
                    strcat(arg_output, tmp_output);
            }
            arg_output[strlen(arg_output)-1] = '\n';
            printf("%s",arg_output);
            num_of_args = 0;
	    j = 0;
        }
	return num_of_cmds;
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

void redirect(char **piped_cmds, char **line_args, char *input_str, 
		char *output_str, int i, int *j, int *num_of_args,
	       	int num_of_cmds, char *tmp) {
	/* Handles the parsing for a command including redirection.
	   Builds array of output strings in line_argsto be printed in parse()
	*/

	char *tkn;
	char *split_line;
        char input_cpy[CMD_MAX];
        char tmp_cmd[CMD_MAX];
	int rflag;

	strcpy(tmp_cmd, piped_cmds[i]);
	strcpy(input_cpy, tmp_cmd);
	if((split_line = strchr(tmp_cmd,'<'))){
	    /*String contains < if it makes it here*/
	    *split_line = '\0'; 
	    /*This assignment to NULL splits the string
		ie: strchr("test1 < test2", '<') = "< test2"*/
	    strcpy(input_cpy, split_line + 1);
	    strcpy(tmp_cmd,input_cpy);
	    tkn = strtok(tmp_cmd, " ");
	    sprintf(input_str,"%s\n", tkn);
	}
	else {
	    if (i == 0) {
		strcpy(input_str,"original stdin\n");                  
	    }
	    else {
		sprintf(input_str,"pipe from stage %d\n",i-1);
	    }
	}


	if((split_line = strchr(input_cpy,'>'))) {
	    *split_line = '\0'; 
	    strcpy(tmp_cmd, split_line+1);
	    tkn = strtok(tmp_cmd, " ");
	    sprintf(output_str,"%s\n", tkn);
	}
	else {
	    sprintf(input_str, "%s\n", input_cpy);
	    if (i == (num_of_cmds - 1)) {
		strcpy(output_str,"original stdout\n");
	    }
	    else {
		sprintf(output_str,"pipe to stage %d\n",i+1);
	    }
	}
	rflag = 0;
        strcpy(tmp, piped_cmds[i]);
	tkn = strtok(tmp, " ");
	
	while(NULL != tkn) {
	    if (rflag) {
		rflag = 0;
	    }
	    else if (!strcmp(tkn, ">") || !strcmp(tkn, "<")) {
		rflag = 1;
	    }
	    else {
		printf("j: %d\n", *j);
		line_args[(*j)++] = tkn;
		(*num_of_args)++;
	    }
	    tkn = strtok(NULL," ");
	} 
}

void no_redirect(char **piped_cmds, char **line_args, char *input_str, 
		char *output_str, int i, int *j, int *num_of_args,
	       	int num_of_cmds, char *tmp) {
	/* Handles the parsing for a command without redirection.
	   Builds array of output strings in line_argsto be printed in parse()
	*/

	char *tkn;
	char tmp_cmd[CMD_MAX];
                
	if (i == 0) {
	    strcpy(input_str,"original stdin\n");                   
	}
	else {
	    sprintf(input_str,"pipe from stage %d\n",i-1);
	}

	if (i == (num_of_cmds - 1)) {
	    strcpy(output_str,"original stdout\n");
	}
	else {
	    sprintf(output_str,"pipe to stage %d\n",i+1);
	}
	strcpy(tmp_cmd, piped_cmds[i]);
	tkn = strtok(piped_cmds[i], " ");
	while(NULL != tkn) {
	    line_args[(*j)++] = tkn;
	    tkn = strtok(NULL," ");
	    (*num_of_args)++;
	} 
}

void tokenize(char *command, char **piped_cmds, int *num_of_cmds) {
	/* Tokenizes command by pipes and puts them into piped_cmds array.
	   Also handles errors.
	*/

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

	if (strlen(command) > CMD_MAX) {
	    printf("command too long\n");
	    exit(EXIT_FAILURE);
	}
        
	command[strlen(command)-1] = '\0'; /*strip newline*/
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
            (*num_of_cmds)++;
        }

	for (i=0; i < *num_of_cmds; i++) {
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
                if (i != (*num_of_cmds - 1) && rcount){
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
}


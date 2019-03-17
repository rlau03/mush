
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#define SIZE 25 
#define CMD_MAX 512 
#define MAX_ARGS 10
#define MAX_PIPE 10
#define TRUE 1

typedef struct arguments arguments;
struct arg_list {
	char *args[MAX_ARGS];
};

void parse_stdin();
void redir_in(char *command); 
void redir_out(char *command); 
int parse_space(char *command, char **line_args, char *tmp); 
char *read_command(void); 
int count_char(char *s, char c); 
int tokenize(char *command, char **piped_cmds);

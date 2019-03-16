
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
int parse_space(char *command, char **line_args); 
char *read_command(void); 
int parse(char *command, char **piped_cmds,char **line_args); 
int count_char(char *s, char c); 
void redirect(char **piped_cmds, char **line_args, char *input_str, 
		char *output_str, int i, int *j, int *num_of_args, 
		int num_of_cmds, char* tmp); 
void no_redirect(char **piped_cmds, char **line_args, char *input_str, 
		char *output_str, int i, int *j, int *num_of_args, 
		int num_of_cmds, char* tmp); 
void tokenize(char *command, char **piped_cmds, int *num_of_cmds);

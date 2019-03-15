
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#define SIZE 25 
#define CMD_MAX 512 
#define MAX_ARGS 10
#define MAX_PIPE 10
#define TRUE 1

void parse_stdin();
char *read_command(void); 
void parse(char *command); 
int count_char(char *s, char c); 
void redirect(char **piped_cmds, char **line_args, char *input_str, 
		char *output_str, int i, int *j, int *num_of_args, 
		int num_of_cmds); 
void no_redirect(char **piped_cmds, char **line_args, char *input_str, 
		char *output_str, int i, int *j, int *num_of_args, 
		int num_of_cmds); 
void tokenize(char *command, char **piped_cmds, int *num_of_cmds);

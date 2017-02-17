#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN  256
#define MAXWORD 80

void split_cmd(char *, int *, char *[]);
void print_arg(char *);
void print_args(int, char *[]);

int main(void)
{
	int ac = 0;
	char *av[MAXWORD];
	char c, lbuf[MAXLEN + 1];

	for (;;) {
		memset(lbuf, '\0' , MAXLEN);
		fprintf(stdout, "mysh$ ");
		fgets(lbuf, MAXLEN + 1, stdin);
		
		if (strchr(lbuf, '\n') == NULL) {
			fprintf(stderr, "Error!\n");
			while ((c = getchar()) != '\n') {}
		}

		split_cmd(lbuf, &ac, av);
		print_args(ac, av);
	}
	
	return 0;
}

void split_cmd(char *cmd, int *ac, char *av[])
{
	int i;
	
	for (i = 0; i < MAXLEN;) {
		if (cmd[i] == '\n') {
			cmd[i] = '\0';
			break;
		}
		if (isblank(cmd[i])) {
			cmd[i] = '\0';
			i++;
		} else {
			av[*ac] = &cmd[i++];
			(*ac)++;
			while (!isblank(cmd[i])) {
				i++;
			}
		}
	}
}

void print_args(int ac, char *av[])
{
	int i;

	for (i = 0; i < ac; i++) {
		printf("av[%d] = %s\n", i, av[i]);
	}
}

// int -> uint8_t

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN  256
#define MAXBUF  1024
#define MAXWORD 80

void split_cmd(char *, int *, char *[], char *);
void print_arg(char *);
void print_args(int, char *[]);

int main(void)
{
	int ac = 0;
	char *av[MAXWORD];
	char c, lbuf[MAXLEN + 1], buf[MAXBUF];

	for (;;) {
		ac = 0;
		memset(lbuf, '\0' , sizeof lbuf);
		memset(buf, 0, sizeof buf);
		fprintf(stdout, "mysh$ ");
		fgets(lbuf, MAXLEN + 1, stdin);
		
		if (strchr(lbuf, '\n') == NULL) {
			fprintf(stderr, "Error!\n");
			while ((c = getchar()) != '\n') {}
		}

		split_cmd(lbuf, &ac, av, buf);
		print_args(ac, av);
	}
	
	return 0;
}

void split_cmd(char *cmd, int *ac, char *av[], char *buf)
{
	int i = 0, j = 0;

	while (i < MAXLEN) {
		if (cmd[i] == '\n') {
			break;
		} else if (cmd[i] == '|' || cmd[i] == '<' || cmd[i] == '>'
				   || cmd[i] == '&') {
			av[*ac] = &buf[j];
			(*ac)++;
			buf[j++] = cmd[i++];
			buf[j++] = '\0';
		} else if (isblank(cmd[i])) {
			i++;
		} else {
			av[*ac] = &buf[j];
			(*ac)++;
			buf[j++] = cmd[i++];
			while (!isblank(cmd[i]) && cmd[i] != '|' && cmd[i] != '<'
				   && cmd[i] != '>' && cmd[i] != '&') {
				buf[j++] = cmd[i++];
			}
			buf[j++] = '\0';
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

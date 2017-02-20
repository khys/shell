#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN  256
#define MAXBUF  1024
#define MAXWORD 80

void split_cmd(char *, int *, char *[], char *);
void split_proc(int *, char *[], int);
void print_arg(char *);
void print_args(int, char *[]);
int count_pipe(int, char *[]);

int main(void)
{
	int ac, p_num;
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
		printf("Total Process = %d\n", count_pipe(ac, av) + 1);		
		for (p_num = 0; p_num < count_pipe(ac, av) + 1; p_num++) {
			split_proc(&ac, av, p_num);
		}
		// print_args(ac, av);
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

void split_proc(int *ac, char *av[], int p_num)
{
	int i, j, p_ac_init, p_ac = 0;
	
	for (j = 0; j < p_num + 1; j++) {
		if (j == p_num) {
			p_ac_init = p_ac;
		}
		while (p_ac < *ac) {
			if (strcmp(av[p_ac], "|") == 0) {
				p_ac++;
				break;
			}
			p_ac++;
		}
	}
	if (strcmp(av[p_ac - 1], "|") == 0) {
		p_ac--;
	}
	printf("Proc %d\n", p_num + 1);
	printf("Pac = %d\n", p_ac - p_ac_init);
	for (i = p_ac_init; i < p_ac; i++) {
		printf("Pav[%d] = %s\n", i - p_ac_init, av[i]);
   	}
	putchar('\n');
}

void print_args(int ac, char *av[])
{
	int i;

	for (i = 0; i < ac; i++) {
		printf("av[%d] = %s\n", i, av[i]);
	}
}

int count_pipe(int ac, char *av[])
{
	int i, cnt = 0;

	for (i = 0; i < ac; i++) {
		if (strcmp(av[i], "|") == 0) {
			cnt++;
		}
	}

	return cnt;
}

// int -> uint8_t

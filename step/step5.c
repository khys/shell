#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#define MAXLEN  256
#define MAXBUF  1024
#define MAXWORD 80
#define MAXCMD  20

void split_cmd(char *, int *, char *[], char *);
char **split_proc(int *, char *[], int);
void print_arg(char *);
void print_args(int, char *[]);
int count_pipe(int, char *[]);
void exec_proc(char **);

int main(void)
{
	int ac, p_num, pipe_num;
	char *av[MAXWORD];
	char c, lbuf[MAXLEN + 1], buf[MAXBUF];
	char **cmd_list[MAXCMD];

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
		if (strcmp(av[0], "exit") == 0) {
			return 0;
		} else if (strcmp(av[0], "cd") == 0) {
			if (ac == 1) {
				chdir(getenv("HOME"));
			} else {
				if (chdir(av[1]) != 0) {
					if (errno == ENOENT) {
						fprintf(stderr,
								"cd: %s: No such file or directory\n",
								av[1]);
					} else {
						fprintf(stderr, "cd: Undefined error\n");
					}
				}
			}
			continue;
		}
		pipe_num = count_pipe(ac, av) + 1;
		printf("Total Process = %d\n", pipe_num);
		for (p_num = 0; p_num < pipe_num; p_num++) {
			cmd_list[p_num] = split_proc(&ac, av, p_num);
			exec_proc(cmd_list[p_num]);
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
				   && cmd[i] != '>' && cmd[i] != '&' && cmd[i] != '\n') {
				buf[j++] = cmd[i++];
			}
			buf[j++] = '\0';
		}
	}
}

char **split_proc(int *ac, char *av[], int p_num)
{
	int i, j, p_ac_init, p_ac = 0;

	for (j = 0; j < p_num + 1; j++) {
		if (j == p_num) {
			p_ac_init = p_ac;
		}
		while (p_ac < *ac) {
			if (av[p_ac] == NULL) {
				p_ac++;
				break;
			} else if (strcmp(av[p_ac], "|") == 0) {
				av[p_ac] = NULL;
				p_ac++;
				break;
			}
			p_ac++;
		}
	}
	if (av[p_ac - 1] == NULL) {
		p_ac--;
	} else {
		av[p_ac] = NULL;
	}
	printf("Proc %d\n", p_num + 1);
	printf("Pac = %d\n", p_ac - p_ac_init);
	for (i = p_ac_init; i < p_ac; i++) {
		printf("Pav[%d] = %s\n", i - p_ac_init, av[i]);
	}
	putchar('\n');

	return &av[p_ac_init];
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

void exec_proc(char **av)
{
	int stat;
	pid_t pid, res;

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(1);
	} else if (pid == 0) {
		execvp(av[0], av);
		exit(1);
	} else {
		res = wait(&stat);
		return;
	}
}

// int -> uint8_t

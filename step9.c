#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAXLEN  256
#define MAXBUF  1024
#define MAXWORD 80
#define MAXCMD  20

struct io_list {
    int ispipe;
    int pipe_fdp[2];
    char *def_in;
    char *def_out;
};

void split_cmd(char *, int *, char *[], char *);
char **split_proc(int *, char *[], int, struct io_list *);
void print_arg(char *);
void print_args(int, char *[]);
int count_pipe(int, char *[]);
void exec_proc(char **, int, struct io_list *);
int built_in(int, char *[]);

int p_ac_init = 0, p_ac = 0;

int main(void)
{
    int ac, p_num, pipe_num;
    char *av[MAXWORD];
    char c, lbuf[MAXLEN + 1], buf[MAXBUF];
    char **cmd_list[MAXCMD];
    struct io_list io[MAXCMD];
    // struct sigaction sa;
    sigset_t unblock_mask, block_mask;
    
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);
    sigprocmask(SIG_SETMASK, &block_mask, &unblock_mask);
    
    for (;;) {
        ac = 0;
        p_ac_init = 0;
        p_ac = 0;

        memset(av, 0, sizeof av);
        memset(lbuf, '\0' , sizeof lbuf);
        memset(buf, 0, sizeof buf);
        memset(io, 0, sizeof io);

        fprintf(stdout, "mysh$ ");
        fgets(lbuf, MAXLEN + 1, stdin);

        if (strchr(lbuf, '\n') == NULL) {
            fprintf(stderr, "Error!\n");
            while ((c = getchar()) != '\n') {}
        }
        split_cmd(lbuf, &ac, av, buf);
        switch (built_in(ac, av)) {
        case 0:
            continue;
            break;
        case -1:
            exit(-1);
            break;
        default:
            break;
        }
        pipe_num = count_pipe(ac, av) + 1;
        printf("Total Process = %d\n", pipe_num);
        for (p_num = 0; p_num < pipe_num; p_num++) {
            printf("Proc %d\n", p_num + 1);
            cmd_list[p_num] = split_proc(&ac, av, p_num, io);
            exec_proc(cmd_list[p_num], p_num, io);
        }
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

char **split_proc(int *ac, char *av[], int p_num, struct io_list *io)
{
    // int i;
    char **cmd_init;

    cmd_init = &av[p_ac_init];

    while (p_ac < *ac) {
        if (av[p_ac] == NULL) {
            break;
        } else if (strcmp(av[p_ac], "|") == 0) {
            io[p_num + 1].ispipe = 1;
            av[p_ac] = NULL;
            p_ac++;
            p_ac_init = p_ac;
            break;
        } else if (strcmp(av[p_ac], "<") == 0) {
            if (p_ac_init == p_ac) {
                p_ac_init += 2;
            }
            av[p_ac] = NULL;
            p_ac++;
            io[p_num].def_in = av[p_ac];
            p_ac++;
            continue;
        } else if (strcmp(av[p_ac], ">") == 0) {
            if (p_ac_init == p_ac) {
                p_ac_init += 2;
            }
            av[p_ac] = NULL;
            p_ac++;
            io[p_num].def_out = av[p_ac];
            p_ac++;
            continue;
        } else {
            p_ac++;
        }
    }
    // printf("Pac = %d\n", p_ac - p_ac_init);
    // for (i = p_ac_init; i < p_ac; i++) {
    //    printf("Pav[%d] = %s\n", i - p_ac_init, av[i]);
    //}
    //putchar('\n');

    return cmd_init;
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

int built_in(int ac, char *av[])
{
    if (av[0] == NULL) {
        return 0;
    } else if (strcmp(av[0], "exit") == 0) {
        return -1;
    } else if (strcmp(av[0], "cd") == 0) {
        if (ac == 1) {
            strcmp(av[1], getenv("HOME"));
        }
        if (chdir(av[1]) != 0) {
            if (errno == ENOENT) {
                fprintf(stderr,
                        "cd: %s: No such file or directory\n", av[1]);
            } else {
                fprintf(stderr, "cd: Undefined error\n");
            }
        }
        return 0;
    }
    return 1;
}

void exec_proc(char **av, int p_num, struct io_list *io)
{
    int stat, fd;
    pid_t pid, res;

    if (io[p_num + 1].ispipe != 0) {
        pipe(io[p_num + 1].pipe_fdp);
    }
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        if (io[p_num].def_in != NULL) {
            fd = open(io[p_num].def_in, O_RDONLY);
            if (fd == -1) {
                perror("open");
                return;
            }
            close(0);
            dup(fd);
            close(fd);
        }
        if (io[p_num].def_out != NULL) {
            fd = open(io[p_num].def_out, O_WRONLY|O_CREAT|O_TRUNC, 0644);
            if (fd == -1) {
                perror("open");
                return;
            }
            close(1);
            dup(fd);
            close(fd);
        }
        if (io[p_num + 1].ispipe != 0) {
            close(1);
            dup(io[p_num + 1].pipe_fdp[1]);
            close(io[p_num + 1].pipe_fdp[0]);
            close(io[p_num + 1].pipe_fdp[1]);
        }
        if (io[p_num].ispipe != 0) {
            close(0);
            dup(io[p_num].pipe_fdp[0]);
            close(io[p_num].pipe_fdp[0]);
            close(io[p_num].pipe_fdp[1]);
        }
        printf("%s", *av);
        execvp(*av, av);
        exit(1);
    }
    if (io[p_num].ispipe != 0) {
        close(io[p_num].pipe_fdp[0]);
        close(io[p_num].pipe_fdp[1]);
    }
    res = wait(&stat);
    return;
}

// int -> uint8_t

// makefile is needed

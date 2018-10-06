#include <stdio.h>
#include <string.h>

#define MAXLEN 10

int main(void)
{
	char c, lbuf[MAXLEN + 1];

	for (;;) {
		memset(lbuf, '\0' , MAXLEN);
		fprintf(stdout, "mysh$ ");
		fgets(lbuf, MAXLEN + 1, stdin);
		if (strchr(lbuf, '\n') == NULL) {
			fprintf(stderr, "Error!\n");
			while ((c = getchar()) != '\n') {}
		} else {
			printf("%s", lbuf);
		}
	}
	
	return 0;
}

// int -> uint8_t

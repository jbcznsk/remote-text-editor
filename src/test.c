//Used for basic input/output stream
#include <stdio.h>
//Used for handling directory files
#include <dirent.h>
//For EXIT codes and error handling
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include<unistd.h> 



int main(int argc, const char *argv[])
{

int a;
int b;
a = 10;
	char *entrada = malloc(100);

	fgets(entrada,100, stdin);

	printf("tamanho: %ld\n", strlen(entrada));

	char str[] = "strtok needs to be called several times to split a string";
	int init_size = strlen(entrada);
	char delim[] = " ";

	char *ptr = strtok(entrada, delim);

	while(ptr != NULL)
	{
		printf("'%s'\n", ptr);
		ptr = strtok(NULL, delim);
	}

	return 0;
}

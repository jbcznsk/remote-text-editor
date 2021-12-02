#ifndef __FUNCS__
#define __FUNCS__

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include<unistd.h>

char*  ls();
void lls();

int  cd(char *diretorio);
int lcd(char *diretorio);

#endif

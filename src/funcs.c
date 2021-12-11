#include "funcs.h"
#include "utils.h"

/* cd <diretorio> - troca de diretorio no servidor */
int
cd(char *diretorio)
{
    chdir(diretorio);

    if (errno == EACCES ){                                 // Sem acesso
        return 1;
    }  else if (errno == ELOOP  || errno == ENAMETOOLONG ){ // Outros
        return 5;
    }
    
    return 0;

}

/* lcd <diretorio> - troca de diretorio no cliente */
int
lcd(char *diretorio)
{
    chdir(diretorio);

    if (errno == EACCES ){                                 // Sem acesso
        return 1;
    } else if (errno == ENOENT  || errno == ENOTDIR){      // Inexistente
        return 2;
    } else if (errno == ELOOP  || errno == ENAMETOOLONG ){ // Outros
        return 5;
    }

    return 0;
}



/* lista os arquivos do diretorio corrente no  cliente*/
void
lls()
{
   struct dirent *d;
	DIR *dh = opendir(".");
    
	while ((d = readdir(dh)) != NULL)
		printf("%s  \n", d->d_name);

}


/* lista os arquivos do diretorio corrente no servidor */
char*
ls()
{
    int tamanho = 0;
    char *retorno = malloc(1);
    strcpy(retorno, "");
    struct dirent *d;
	DIR *dh = opendir(".");

	while ((d = readdir(dh)) != NULL){
        tamanho += tamanhoString(d->d_name)+2;
        retorno = realloc(retorno, tamanho);
        strcat(retorno,d->d_name);
        strcat(retorno,"\n");
    }
    closedir(dh);
    return retorno;
}

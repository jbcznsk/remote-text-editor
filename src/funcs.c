#include "funcs.h"
#include "utils.h"

/* cd <diretorio> - troca de diretorio no servidor */
int
cd(char *diretorio)
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

/* lcd <diretorio> - troca de diretorio no cliente */
int
lcd(char *diretorio)
{
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
ls(){
    int tamanho = 0;
    char *retorno = malloc(1);
    strcpy(retorno, "");
    struct dirent *d;
	DIR *dh = opendir(".");
    
	while ((d = readdir(dh)) != NULL){
        tamanho += tamanhoString(d->d_name)+1;
        retorno = realloc(retorno, tamanho);
        strcat(retorno,d->d_name);
        strcat(retorno,"\n");
    }
    // printf("%s", retorno);
    return retorno;
}

/* ver <arquivo> - mostra o conteudo do arquivo fonte C do servidor na tela do cliente.
As linhas devem ter numeracao */
void
ver(char *arquivo)
{

}

/* linha <linha> <arquivo> - mostra a linha do arquivo */
// usar o sed
void 
linha(int linha, char *arquivo)
{
    
}

/* linhas <inicio> <fim> <arquivo> - mostra as linhas entre o inicio e o fim*/
void 
linhas(int inicio, int fim, char *arquivo)
{

}

/* edito <linha> <arquivo> "<texto>" 
para add nova linha, botar ultima linha + 1*/
void 
edit(int linha, char *arquivo, char *texto)
{ 

}

/* compilar <opcoes> <arquivo> */
void
compilar(/*?*/ char *arquivo)
{
    char comando[100];
    sprintf(comando, "gcc %s", arquivo);
    system(comando);
}
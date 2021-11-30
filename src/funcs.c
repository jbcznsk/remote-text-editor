#include "funcs.h"
#include "utils.h"

/* cd <diretorio> - troca de diretorio no servidor */
void
cd(char *diretorio)
{
    chdir(diretorio);
}

/* lcd <diretorio> - troca de diretorio no cliente */
void
lcd(char *diretorio)
{

}

/* lista os arquivos do diretorio corrente no servidor */
void
ls(pacote_t *pacote, int *qtdPacotes)
{
   struct dirent *d;
	DIR *dh = opendir(".");
    
	while ((d = readdir(dh)) != NULL)
		printf("%s  \n", d->d_name);

}

/* lista os arquivos do diretorio corrente no cliente */
void
lls(){
 struct dirent *d;
	DIR *dh = opendir(".");
    
	while ((d = readdir(dh)) != NULL)
		printf("%s  \n", d->d_name);
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
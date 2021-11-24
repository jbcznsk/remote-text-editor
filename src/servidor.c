#include "utils.h"


int
main()
{
    int soquete;
    struct sockaddr_ll endereco;

    configuraInicio(&soquete, &endereco);

    char dados[100];
    
    struct sockaddr *end;

    int tamanhoEndereco = sizeof(endereco);

    int a = recvfrom(soquete, dados, 100, 0,(struct sockaddr*)&endereco, &tamanhoEndereco);

    fprintf(stderr, "%d", a);

    char b[4] = "opa";

    printf("%s\n",dados);

    perror(b);

    return 0;
}
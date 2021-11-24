#include "utils.h"
#include "funcs.h"


int
main()
{
    int soquete;
    struct sockaddr_ll endereco;

    configuraInicio(&soquete, &endereco);

    char dados[100] = "1234567890123456789";

    padding(dados);
    
    int a = sendto(soquete, dados, 100, 0,(struct sockaddr*)&endereco, sizeof(struct sockaddr_ll));

    fprintf(stderr, "%d", a);

    char b[4] = "opa";

    perror(b);

    lls();

    compilar("test.c");

    return 0;
}
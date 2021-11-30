#include "utils.h"
#include "funcs.h"






int
main()
{
    int sequencializacao = 0;
    int soquete;
    struct sockaddr_ll endereco;

    configuraInicio(&soquete, &endereco);

    // --- Loop começa aqui?



    // Ler o comando do terminal
    // fazer o(s) pacote(s)
    // enviar
    // tratar ack/nack


    char dados2[15] = "..";
    pacote_t pacote;
    pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR,2,sequencializacao,CD,dados2,calculaParidade(pacote));
    imprimePacote(pacote);
    int tamanhoEnvio = enviaPacote(pacote, soquete, endereco);
    fprintf(stdout, "Tamanho envio: %d bytes\n", tamanhoEnvio);

    // if (/*ACK*/){
    //     sequencializacao++;
    // }

    perror("status");

    // lls();

    // compilar("test.c");

    return 0;
}
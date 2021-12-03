#include "utils.h"
#include "funcs.h"

#define DEBUG

int getIntDados(pacote_t pacote, int deslocamento)
{
    int indice = 4*deslocamento;
    int inteiro;

    inteiro  = pacote.dados[indice+0] <<  24;
    inteiro |= pacote.dados[indice+1] <<  16;
    inteiro |= pacote.dados[indice+2] <<   8;
    inteiro |= pacote.dados[indice+3] <<   0;

    return inteiro;
}

void 
lsServidor(int soquete, int *sequencializacao)
{
    int tamanhoConteudo, tamanhoRestante, tamanhoAtual, qtdPacotes, contadorPacotes, seq;
    char *conteudo;
    pacote_t pacoteEnvio;

    conteudo = ls();
    puts(conteudo);
    seq = *sequencializacao;
    tamanhoConteudo = tamanhoString(conteudo);
    printf("Tamanho Conteudo: %d\n", tamanhoConteudo);
    tamanhoRestante = tamanhoConteudo;
    qtdPacotes = (tamanhoConteudo / 15) + ((tamanhoConteudo % 15) ? 1 : 0);
    contadorPacotes = 0;

    printf("¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");
    printf("Quantidade Pacotes: %d\n", qtdPacotes);
    printf("¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");

    while (contadorPacotes < qtdPacotes)
    {

        // montar o pacote
        if (tamanhoConteudo > 15)
        {
            tamanhoAtual = 15;
            tamanhoConteudo -= 15;
        }
        else
        {
            tamanhoAtual = tamanhoConteudo;
        }

        char *mensagemAtual = (char *)malloc(sizeof(char) * tamanhoAtual);
        for (int i = 0; i < tamanhoAtual; i++)
            mensagemAtual[i] = conteudo[contadorPacotes * 15 + i];

        pacoteEnvio = empacota(INIT_MARK,
                               CLIENT_ADDR,
                               SERVER_ADDR,
                               tamanhoAtual,
                               seq,
                               CLS,
                               mensagemAtual);

        // enviar a mensagem
        struct sockaddr_ll endereco;
        enviaPacote(pacoteEnvio, soquete, endereco);
        aumentaSequencia(&seq);

        // esperar o ACK ou NACK
        pacote_t pacoteRecebido;
        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
            imprimePacote(pacoteRecebido);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
#ifdef DEBUG
        printf("<<< RECEBENDO PACOTE <<<\n");
        // imprimePacote(pacote);
        printf("=======================\n");
#endif

        if (getTipoPacote(pacoteRecebido) == ACK){
        // -> ACK  = contador++
            puts("ACK!");
            contadorPacotes++;
        } else {
        // -> NACK = reenvia o mesmo pacote;
            puts("NACK!");
            tamanhoRestante += 15;
        }

        free(mensagemAtual);
        printf("¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");
        printf("Contador Pacotes: %d\n", contadorPacotes);
        printf("¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");
    }

    // enviar Fim de Transmissao
    pacoteEnvio = empacota(INIT_MARK,
                               CLIENT_ADDR,
                               SERVER_ADDR,
                               0,
                               seq,
                               FIM_TRANS,
                               NULL);

        // enviar a mensagem
        struct sockaddr_ll endereco;
        enviaPacote(pacoteEnvio, soquete, endereco);

    *sequencializacao = seq;
    free(conteudo);
}

void 
linhaServidor(int soquete, int *sequencializacao, pacote_t pacote)
{
    int seq = *sequencializacao;
    struct sockaddr_ll endereco;
    char *nomeArquivo = getDadosPacote(pacote);

    enviarACKParaCliente(soquete, endereco, seq);
    aumentaSequencia(&seq);

    pacote_t pacoteRecebido;
    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
        imprimePacote(pacoteRecebido);
    } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

    int linha = getIntDados(pacote, 0);

    // rodar o sed com o nome e a linha em um arquivo
    // abrir esse arquivo
    // ler a linha
    // pegar o tamanho da linha <<< a partir daqui acho que ja fica igual o LS
    // calcular a quantidade de pacotes
    // ...

    
    // enviar Fim de Transmissao
    pacoteEnvio = empacota(INIT_MARK,
                               CLIENT_ADDR,
                               SERVER_ADDR,
                               0,
                               seq,
                               FIM_TRANS,
                               NULL);

    // enviar a mensagem
    enviaPacote(pacoteEnvio, soquete, endereco);
    free(nomeArquivo);
    *sequencializacao = seq;
}

int main()
{
    pacote_t pacote;
    int sequencializacao = 0;

    int soquete;
    struct sockaddr_ll endereco;
    configuraInicio(&soquete, &endereco);

    // Apenas para desenvolvimento
    int a = 0;

    // --------------- //

    while (1)
    {
        // Aqui eu sempre vou ter um pacote válido (falta a paridade mas fodase)
        int a = 0;
        do
        {
            printf("---> %d\n", a++);
            pacote = lerPacote(soquete, endereco);
            imprimePacote(pacote);
        } while (!(validarLeituraServidor(pacote) && validarSequencializacao(pacote, sequencializacao)));

#ifdef DEBUG
        printf("<<< RECEBENDO PACOTE <<<\n");
        // imprimePacote(pacote);
        printf("=======================\n");
#endif

        switch (getTipoPacote(pacote))
        {
        case CD:
            printf("===== Comando: CD ===== \n");
            enviarACKParaCliente(soquete, endereco, sequencializacao);
            break;

        case LS:
            printf("===== Comando: LS ===== \n");
            lsServidor(soquete, &sequencializacao);
            printf("=====   FIM  : LS ===== \n");
            break;

        case LINHA:
            printf("===== Comando: LINHA ===== \n");
            // linhaServidor();
            printf("=====   FIM  : LINHA ===== \n");
            break;

        default:
            break;
        }
    }

    return 0;
}
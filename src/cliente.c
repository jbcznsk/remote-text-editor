#include "utils.h"
#include "funcs.h"

int validaTipoPacote(pacote_t pacote)
{
    int tipo = getTipoPacote(pacote);

    if (
        tipo != ACK &&
        // tipo != NACK &&
        tipo != FIM_TRANS &&
        // tipo != CLS &&
        // tipo != CA &&
        tipo != ERRO)
        return 1;
    return 0;
}

int validarLeituraCliente(pacote_t pacote)
{
    return ((getEnderecoOrigem(pacote) == SERVER_ADDR) && (getEnderecoDestino(pacote) == CLIENT_ADDR));
}

void enviarACKParaServidor(int soquete, struct sockaddr_ll endereco, int sequencializacao)
{
    pacote_t pacote;
    char dados[15] = "oi jorge";
    pacote = empacota(INIT_MARK,SERVER_ADDR,CLIENT_ADDR, 0, sequencializacao, ACK, dados);
    enviaPacote(pacote, soquete, endereco);
}


int main()
{
    int sequencializacao = 0;
    int soquete;
    struct sockaddr_ll endereco;
    pacote_t pacoteRecebido;

    configuraInicio(&soquete, &endereco);

    // lls();

    //------------------------------------

    char dados2[15] = "";
    pacote_t pacote;

    pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 0, sequencializacao, LS, dados2);
    // imprimePacote(pacote);
    int tamanhoEnvio = enviaPacote(pacote, soquete, endereco);
    // fprintf(stdout, "Tamanho envio: %d bytes\n", tamanhoEnvio);
    // perror("status");
    
    // char tipo = getTipoPacote(pacote);

    char *retornoLS = malloc(1);
    retornoLS[0] = '\0';
    int tamanhoLS = 0;

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
        if (validarLeituraCliente(pacoteRecebido) /*&& validarSequencializacao(pacoteRecebido, sequencializacao)*/)
        {
            printf("Tipo do pacote recebido :");
            printByte(getTipoPacote(pacoteRecebido));

            if (!confereParidade(pacoteRecebido))
            {
                puts("Paridade zoada");
                // enviarNackParaServidor(soquete, endereco, serializacao);
            }
            else
            {
                switch (getTipoPacote(pacoteRecebido))
                {
                case ACK:
                    printf("ACK!\n");
                    aumentaSequencia(&sequencializacao);
                    break;

                case NACK:
                    printf("NACK!\n");
                    enviaPacote(pacote, soquete, endereco);
                    break;

                case ERRO:
                    printf("ERRO!\n");
                    // Tratar erro ?
                    break;

                case CLS:
                    printf("Conteudo LS!\n");

                    // aumentaSequencia(&sequencializacao);

                    // // Concatena a resposta
                    // int tamanhoDados = getTamanhoPacote(pacoteRecebido);
                    // retornoLS = realloc(retornoLS, tamanhoLS+tamanhoDados+1);
                    // puts(pacoteRecebido.dados);
                    // for (int i = 0 ; i < tamanhoDados; i++)
                    //     retornoLS[tamanhoLS+i] = pacoteRecebido.dados[i];
                    // tamanhoLS += tamanhoDados;
                    // puts(retornoLS);

                    // // Envia o ACK
                    // enviarACKParaServidor(soquete, endereco, sequencializacao);

                    do
                    {
                        if (validarLeituraCliente(pacoteRecebido) &&  (sequencializacao == getSequenciaPacote(pacoteRecebido))){

                            int tamanhoDados = getTamanhoPacote(pacoteRecebido);
                            retornoLS = realloc(retornoLS, tamanhoLS+tamanhoDados+1);

                            for (int i = 0 ; i < tamanhoDados; i++)
                                retornoLS[tamanhoLS+i] = pacoteRecebido.dados[i];

                            tamanhoLS += tamanhoDados;
                            
                            puts("RETORNO ATE AGORA:");
                            puts(retornoLS);
                            puts("_____________");

                            aumentaSequencia(&sequencializacao);
                        }
                        pacoteRecebido = lerPacote(soquete, endereco);
                    } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);
                    printf("222 FIM DA TRANSMISSAO!\n");

                    printf("%s", retornoLS);
                    break;

                case CA:
                    printf("Conteudo Arquivo!\n");
                    // Concatenar a resposta e continuar lendo os pacotes
                    break;

                case FIM_TRANS:
                    printf("FIM DA TRANSMISSAO!\n");

                    printf("%s", retornoLS);


                    break;

                default:
                    break;
                }
            }
        }
    } while (validaTipoPacote(pacoteRecebido));


    return 0;
}
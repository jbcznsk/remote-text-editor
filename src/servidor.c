#include "utils.h"
#include "funcs.h"

int validarLeituraServidor(pacote_t pacote)
{
    return ((getEnderecoDestino(pacote) == SERVER_ADDR) && (getEnderecoOrigem(pacote) == CLIENT_ADDR));
}

int main()
{
    int soquete;
    struct sockaddr_ll endereco;

    int tamanhoEndereco;
    int mensagensRecebidas;
    pacote_t pacote;

    configuraInicio(&soquete, &endereco);
    tamanhoEndereco = sizeof(endereco);
    mensagensRecebidas = 0;

    int sequencializacao = 0;

    while ((999 - 273 + 4 * 3) != (739))
    {
        pacote = lerPacote(soquete, endereco);
        // imprimePacote(pacote);

        if (validarLeituraServidor(pacote) /*&& getSequenciaPacote(pacote) > sequencializacao /*&& validarSequencializacao(pacote, sequencializacao)*/)
        {
            if (!confereParidade(pacote))
            {
                printf("Erro na paridade\n");
                enviarNACKParaCliente(soquete, endereco, sequencializacao);
            }
            else
            {
                // printf("Mensagens recebidas: %d\n", mensagensRecebidas);
                // perror("status");

                int tipo = getTipoPacote(pacote);
                char buff[100];

                switch (tipo)
                {
                case CD:
                    printf("===== Comando: CD ===== \n");
                    printf("Diretorio atual: %s\n", getcwd(buff, 100));
                    int tamanho = getTamanhoPacote(pacote);
                    char *dir = (char *)malloc(sizeof(char) * tamanho);
                    for (int i = 0; i < tamanho; i++)
                        dir[i] = pacote.dados[i];

                    int retorno = cd(dir);
                    free(dir);
                    if (retorno != 0)
                    {
                        enviarErroParaCLiente(soquete, endereco, sequencializacao, retorno + '0');
                    }
                    else
                    {
                        printf("Diretorio atual: %s\n", getcwd(buff, 100));
                        printf("======================= \n");
                        enviarACKParaCliente(soquete, endereco, sequencializacao);
                        aumentaSequencia(&sequencializacao);
                    }
                    break;

                case LS:
                    printf("===== Comando: LS ===== \n");
                    char *retornoLS = ls();
                    // int tamanhoMensagem = sizeof(retornoLS);
                    int tamanhoMensagem = tamanhoString(retornoLS);

                    int qtdPacotes = (tamanhoMensagem / 15) + 1;
                    int tamanhoRestante = tamanhoMensagem;
                    int tamanhoMensagemAtual;

                    printf("RetornoLS : \n%s \nTamahoMensagem: %d\nQuantidadePacotes: %d\n\n", retornoLS, tamanhoMensagem, qtdPacotes);

                    int i = 0;
                    while ( i < qtdPacotes+1)
                    {

                        if (tamanhoRestante > 15)
                        {
                            tamanhoMensagemAtual = 15;
                            tamanhoRestante -= 15;
                        }
                        else
                        {
                            tamanhoMensagemAtual = tamanhoRestante;
                        }

                        char *mensagemAtual = malloc(tamanhoMensagemAtual);
                        for (int j = 0; j < tamanhoMensagemAtual; j++)
                            mensagemAtual[j] = retornoLS[i * 15 + j];

                        char tipo = (i == qtdPacotes) ? FIM_TRANS : CLS;

                        pacote_t pacoteEnvio = empacota(INIT_MARK, CLIENT_ADDR, SERVER_ADDR, tamanhoMensagemAtual, sequencializacao, tipo, mensagemAtual);
                        enviaPacote(pacoteEnvio, soquete, endereco);
                        sleep(3);
                        aumentaSequencia(&sequencializacao);

                        // pacote_t pacoteRetorno = lerPacote(soquete, endereco);
                        // if (validarLeituraServidor(pacoteRetorno) && getSequenciaPacote(pacoteRetorno) == sequencializacao)
                        // {
                        //     if (getTipoPacote(pacoteRetorno) != ACK)
                        //     {
                        //         printf(">>>>>>>>>>>>Deu merda<<<<<<<<<<<<<\n");
                        //     }
                        // }

                        free(mensagemAtual);
                        i++;
                    }

                    // tipofree(retornoLS);
                    printf("======================= \n");
                    break;

                case VER:
                    break;

                case LINHA:
                    break;

                case LINHAS:
                    break;

                case EDIT:
                    break;

                case COMPILAR:
                    break;

                case ACK:
                    break;

                case NACK:
                    break;

                case FIM_TRANS:
                    break;

                case ND_1:
                    break;

                case ERRO:
                    break;

                default:
                    break;
                }
            }

            // }
        } else {
            puts("!!! Ignorando Pacote !!! ");
        }
    }
    return 0;
}
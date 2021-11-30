#include "utils.h"
#include "funcs.h"

int main()
{
    int soquete;
    struct sockaddr_ll endereco;

    int mensagensRecebidas = 0;

    configuraInicio(&soquete, &endereco);

    struct sockaddr *end;

    int tamanhoEndereco = sizeof(endereco);
    pacote_t pacote;

    while (1 != 2)
    {

        int a = recvfrom(soquete, &pacote, 100, 0, (struct sockaddr *)&endereco, &tamanhoEndereco);
        int sequencializacao = (pacote.SeqTipo & 0b11110000) >> 4;

        if (sequencializacao == mensagensRecebidas)
        {
            mensagensRecebidas++;
            fprintf(stdout, "%d\nQuantidadeMensagens: %d\n", a, mensagensRecebidas);

            perror("status");

            imprimePacote(pacote);

            int tipo = pacote.SeqTipo & 15;
            char buff[100];

            switch (tipo)
            {
            case CD:
                printf("===== Comando: CD ===== \n");
                printf("Diretorio atual: %s\n", getcwd(buff, 100));
                int tamanho = pacote.EdEoTam & 0b00001111;
                char *dir = (char*) malloc (sizeof(char)*tamanho);
                for (int i = 0; i < tamanho; i++)
                    dir[i] = pacote.dados[i];
                
                cd(dir);
                free(dir);
                printf("Diretorio atual: %s\n", getcwd(buff, 100));
                printf("======================= \n");

                break;


            case LS:
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

            case CLS:
                break;

            case CA:
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
    }
    return 0;
}
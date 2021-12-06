#include "utils.h"
#include "funcs.h"

// #define DEBUG

void cdCliente(int soquete, int *sequencializacao, char *diretorio)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           tamanhoString(diretorio),
                           seq,
                           CD,
                           diretorio);

    struct sockaddr_ll endereco;
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
        imprimePacote(pacoteRecebido);
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
    printf(" <<<< Recebendo pacote\n");

    if (getTipoPacote(pacoteRecebido) == ACK)
    {
        aumentaSequencia(&seq);
    }
    else if (getTipoPacote(pacoteRecebido) == NACK)
    {
        cdCliente(soquete, sequencializacao, diretorio);
    }

    *sequencializacao = seq;
}

void lsCliente(int soquete, int *sequencializacao)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;
    char *retornoLS = malloc(1);
    int tamanhoLS = 0;

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           0,
                           seq,
                           LS,
                           NULL);

    struct sockaddr_ll endereco;
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
        // printf("<<< RECEBENDO PACOTE <<<\n");
        // imprimePacote(pacoteRecebido);

        if (getTipoPacote(pacoteRecebido) == CLS)
        {

            aumentaSequencia(&seq);

            int tamanhoDados = getTamanhoPacote(pacoteRecebido);
            retornoLS = realloc(retornoLS, tamanhoLS + tamanhoDados);
            for (int i = 0; i < tamanhoDados; i++)
                retornoLS[tamanhoLS + i] = pacoteRecebido.dados[i];

            tamanhoLS += tamanhoDados;

            enviarACKParaServidor(soquete, endereco, seq);
        }

    } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

    puts(retornoLS);

    aumentaSequencia(&seq);
    *sequencializacao = seq;
}

void linhaCliente(int soquete, int *sequencializacao, char *arquivo, int nrLinha)
{

    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;
    char *retornoLinha = malloc(1);
    int tamanhoLinha = 0;

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           tamanhoString(arquivo),
                           seq,
                           LINHA,
                           arquivo);

    struct sockaddr_ll endereco;
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
    printf("<<< RECEBENDO PACOTE <<<\n");
    imprimePacote(pacoteRecebido);

    if (getTipoPacote(pacoteRecebido) == ACK)
    {
        puts("ACK! Passando o numero da linha");
        aumentaSequencia(&seq);

        char dados[4];

        for (int i = 0; i < 4; i++)
            dados[i] = nrLinha >> (24 - 8 * i);

        pacoteEnvio = empacota(INIT_MARK,
                               SERVER_ADDR,
                               CLIENT_ADDR,
                               4,
                               seq,
                               LIF,
                               dados);

        enviaPacote(pacoteEnvio, soquete, endereco);

        do
        {

            do
            {
                pacoteRecebido = lerPacote(soquete, endereco);
            } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

            int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

            retornoLinha = realloc(retornoLinha, tamanhoLinha + tamanhoRetorno);

            for (int i = 0; i < tamanhoRetorno; i++)
                retornoLinha[tamanhoLinha + i] = pacoteRecebido.dados[i];

            tamanhoLinha += tamanhoRetorno;

            aumentaSequencia(&seq);
            enviarACKParaServidor(soquete, endereco, seq);

        } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

        puts("FIM TRANSMISSAO");
        printf("***** CONTEUDO LINHA: %s\n", retornoLinha);
    }

    *sequencializacao = seq;
}

void linhasCliente(int soquete, int *sequencializacao, char *arquivo, int linhaInicial, int linhaFinal)
{

    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;
    char *retornoLinha = malloc(1);
    int tamanhoLinha = 0;

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           tamanhoString(arquivo),
                           seq,
                           LINHAS,
                           arquivo);

    struct sockaddr_ll endereco;
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
    printf("<<< RECEBENDO PACOTE <<<\n");
    imprimePacote(pacoteRecebido);

    if (getTipoPacote(pacoteRecebido) == ACK)
    {
        puts("ACK! Passando o numero da linha");
        aumentaSequencia(&seq);

        char dados[8];

        for (int i = 0; i < 4; i++)
            dados[i] = linhaInicial >> (24 - 8 * i);

        for (int i = 0; i < 4; i++)
            dados[i + 4] = linhaFinal >> (24 - 8 * i);

        pacoteEnvio = empacota(INIT_MARK,
                               SERVER_ADDR,
                               CLIENT_ADDR,
                               8,
                               seq,
                               LIF,
                               dados);

        enviaPacote(pacoteEnvio, soquete, endereco);

        do
        {

            do
            {
                pacoteRecebido = lerPacote(soquete, endereco);
            } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

            int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

            retornoLinha = realloc(retornoLinha, tamanhoLinha + tamanhoRetorno);

            for (int i = 0; i < tamanhoRetorno; i++)
                retornoLinha[tamanhoLinha + i] = pacoteRecebido.dados[i];

            tamanhoLinha += tamanhoRetorno;

            aumentaSequencia(&seq);
            enviarACKParaServidor(soquete, endereco, seq);

        } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

        puts("FIM TRANSMISSAO");
        printf("***** CONTEUDO LINHAS: %s\n", retornoLinha);
    }

    *sequencializacao = seq;
}

void verCliente(int soquete, int *sequencializacao, char *arquivo)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;
    char *retornoVer = malloc(1);
    int tamanhoVer = 0;

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           tamanhoString(arquivo),
                           seq,
                           VER,
                           arquivo);

    struct sockaddr_ll endereco;
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
    printf("<<< RECEBENDO PACOTE <<<\n");
    imprimePacote(pacoteRecebido);

    do
    {
        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

        if (getTipoPacote(pacoteRecebido) == CA)
        {

            int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

            retornoVer = realloc(retornoVer, tamanhoVer + tamanhoRetorno);

            for (int i = 0; i < tamanhoRetorno; i++)
                retornoVer[tamanhoVer + i] = pacoteRecebido.dados[i];

            tamanhoVer += tamanhoRetorno;

            aumentaSequencia(&seq);
            enviarACKParaServidor(soquete, endereco, seq);
        }

    } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

    puts("FIM TRANSMISSAO");
    printf("***** CONTEUDO VER:\n%s", retornoVer);

    free(retornoVer);
    *sequencializacao = seq;
}

void editarCliente(int soquete, int *sequencializacao, char *arquivo, char *conteudo, int nrLinha)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           tamanhoString(arquivo),
                           seq,
                           EDIT,
                           arquivo);

    struct sockaddr_ll endereco;
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
    printf(" <<<< Recebendo pacote\n");
    imprimePacote(pacoteRecebido);

    if (getTipoPacote(pacoteRecebido) == ACK)
    {
        aumentaSequencia(&seq);

        char dados[4];
        for (int i = 0; i < 4; i++)
            dados[i] = nrLinha << (24 - 8 * i);

        pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 4, seq, LIF, dados);
        enviaPacote(pacoteEnvio, soquete, endereco);

        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
        printf(" <<<< Recebendo pacote\n");
        imprimePacote(pacoteRecebido);

        if (getTipoPacote(pacoteRecebido) == ACK)
        {
            aumentaSequencia(&seq);

            pacote_t pacoteEnvio;
            int tamanhoRestante = tamanhoString(conteudo);
            int qtdPacotes = (tamanhoRestante / 15) + ((tamanhoRestante % 15) ? 1 : 0);
            int tamanhoAtual = 0;
            int contadorPacotes = 0;

            while (contadorPacotes < qtdPacotes)
            {
                if (tamanhoRestante > 15)
                {
                    tamanhoAtual = 15;
                    tamanhoRestante -= 15;
                }
                else
                {
                    tamanhoAtual = tamanhoRestante;
                }

                char *conteudoAtual = malloc(tamanhoAtual);
                for (int i = 0; i < tamanhoAtual; i++)
                {
                    conteudoAtual[i] = conteudo[contadorPacotes * 15 + i];
                }

                pacoteEnvio = empacota(INIT_MARK,
                                       SERVER_ADDR,
                                       CLIENT_ADDR,
                                       tamanhoAtual,
                                       seq,
                                       CA,
                                       conteudoAtual);

                struct sockaddr_ll endereco;
                enviaPacote(pacoteEnvio, soquete, endereco);
                aumentaSequencia(&seq);

                do
                {
                    pacoteRecebido = lerPacote(soquete, endereco);
                } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
                printf(" <<<< Recebendo pacote\n");
                imprimePacote(pacoteRecebido);

                if (getTipoPacote(pacoteRecebido) == ACK)
                {
                    // -> ACK  = contador++
                    puts("ACK!");
                    contadorPacotes++;
                }
                else
                {
                    // -> NACK = reenvia o mesmo pacote;
                    puts("NACK!");
                    tamanhoRestante += 15;
                }
            }
        }
        else
        {
            // reenvia o numero da linha
        }
    }
    else
    {
        // reenvia o nome do arquivo
    }

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           0,
                           seq,
                           FIM_TRANS,
                           NULL);
    enviaPacote(pacoteEnvio, soquete, endereco);

    *sequencializacao = seq;
}

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

int getComando(char *comando)
{

    if (!strcmp(comando, "cd"))
    {
        return CD;
    }
    else if (!strcmp(comando, "ls"))
    {
        return LS;
    }
    else if (!strcmp(comando, "ver"))
    {
        return VER;
    }
    else if (!strcmp(comando, "linha"))
    {
        return LINHA;
    }
    else if (!strcmp(comando, "linhas"))
    {
        return LINHAS;
    }
    else if (!strcmp(comando, "edit"))
    {
        return EDIT;
    }
    else if (!strcmp(comando, "compilar"))
    {
        return COMPILAR;
    }
    else
    {
        return -1;
    }
}

int main()
{
    int sequencializacao = 0;
    int soquete;
    struct sockaddr_ll endereco;
    pacote_t pacoteRecebido;
    char *diretorio, *arquivo, *texto;
    char *buff;
    char *entrada = malloc(100);
    int linha, linhaFinal;

    configuraInicio(&soquete, &endereco);

    char delim[] = " ";
    char delimEdit[] = "\"";

    while (1)
    {
        fgets(entrada, 100, stdin);
        entrada[strlen(entrada) - 1] = '\0';
        if (strlen(entrada) == 0)
            continue; // Trata o enter vazio :)

        char delim[] = " ";
        char *cmd = strtok(entrada, delim);
        printf("Comando : %s\n", cmd);

        int comando = getComando(cmd);

        switch (comando)
        {
        case CD:
            diretorio = strtok(NULL, delim);
            printf("O diretirio é: %s\n", diretorio);
            cdCliente(soquete, &sequencializacao, diretorio);
            break;

        case LS:
            lsCliente(soquete, &sequencializacao);
            break;

        case VER:
            arquivo = strtok(NULL, delim);
            verCliente(soquete, &sequencializacao, arquivo);
            break;

        case LINHA:
            linha = atoi(strtok(NULL, delim));
            arquivo = strtok(NULL, delim);
            linhaCliente(soquete, &sequencializacao, arquivo, linha);
            break;

        case LINHAS:
            linha = atoi(strtok(NULL, delim));
            linhaFinal = atoi(strtok(NULL, delim));
            arquivo = strtok(NULL, delim);
            linhasCliente(soquete, &sequencializacao, arquivo, linha, linhaFinal);
            break;

        case EDIT:
            linha = atoi(strtok(NULL, delim));
            arquivo = strtok(NULL, delim);
            texto = strtok(NULL, delimEdit);
            editarCliente(soquete, &sequencializacao, arquivo, texto, linha);
            break;

        case COMPILAR:

            break;

        default:
            printf("Comando inválido\n");
        }
    }

    // ========= EDITAR

    // pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 11, sequencializacao, EDIT, dados2);
    // enviaPacote(pacote, soquete, endereco);

    // do
    // {
    //     pacoteRecebido = lerPacote(soquete, endereco);
    //     imprimePacote(pacoteRecebido);
    // } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, sequencializacao)));
    // printf(" <<<< Recebendo pacote\n");

    // if (getTipoPacote(pacoteRecebido) == ACK)
    // {
    //     aumentaSequencia(&sequencializacao);

    //     int nrLinha = 1;

    //     dados2[0] = nrLinha >> 24;
    //     dados2[1] = nrLinha >> 16;
    //     dados2[2] = nrLinha >> 8;
    //     dados2[3] = nrLinha;

    //     pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 4, sequencializacao, LIF, dados2);
    //     enviaPacote(pacote, soquete, endereco);

    //     do
    //     {
    //         pacoteRecebido = lerPacote(soquete, endereco);
    //         imprimePacote(pacoteRecebido);
    //     } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, sequencializacao)));

    //     if (getTipoPacote(pacoteRecebido) == ACK)
    //     {
    //         aumentaSequencia(&sequencializacao);
    //         char novaLinha[40] = "essa é uma nova linha";

    //         pacote_t pacoteEnvio;
    //         int tamanhoRestante = tamanhoString(novaLinha);
    //         int qtdPacotes = (tamanhoRestante / 15) + ((tamanhoRestante % 15) ? 1 : 0);
    //         int tamanhoAtual = 0;
    //         int contadorPacotes = 0;

    //         while (contadorPacotes < qtdPacotes)
    //         {
    //             if (tamanhoRestante > 15)
    //             {
    //                 tamanhoAtual = 15;
    //                 tamanhoRestante -= 15;
    //             }
    //             else
    //             {
    //                 tamanhoAtual = tamanhoRestante;
    //             }

    //             char *conteudoAtual = malloc(tamanhoAtual);
    //             for (int i = 0; i < tamanhoAtual; i++)
    //             {
    //                 conteudoAtual[i] = novaLinha[contadorPacotes * 15 + i];
    //             }

    //             pacoteEnvio = empacota(INIT_MARK,
    //                                    SERVER_ADDR,
    //                                    CLIENT_ADDR,
    //                                    tamanhoAtual,
    //                                    sequencializacao,
    //                                    CA,
    //                                    conteudoAtual);

    //             struct sockaddr_ll endereco;
    //             enviaPacote(pacoteEnvio, soquete, endereco);
    //             aumentaSequencia(&sequencializacao);

    //             do
    //             {
    //                 pacoteRecebido = lerPacote(soquete, endereco);
    //                 imprimePacote(pacoteRecebido);
    //             } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, sequencializacao)));

    //             if (getTipoPacote(pacoteRecebido) == ACK)
    //             {
    //                 // -> ACK  = contador++
    //                 puts("ACK!");
    //                 contadorPacotes++;
    //             }
    //             else
    //             {
    //                 // -> NACK = reenvia o mesmo pacote;
    //                 puts("NACK!");
    //                 tamanhoRestante += 15;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         // reenvia o numero da linha
    //     }
    // }
    // else
    // {
    //     // reenvia o nome do arquivo
    // }

    // pacote_t pacoteEnvio;
    // pacoteEnvio = empacota(INIT_MARK,
    //                        CLIENT_ADDR,
    //                        SERVER_ADDR,
    //                        0,
    //                        sequencializacao,
    //                        FIM_TRANS,
    //                        NULL);

    // // enviar a mensagem
    // enviaPacote(pacoteEnvio, soquete, endereco);

    // ========= FIM EDITAR

    // ======== VER

    // do
    // {
    //     pacoteRecebido = lerPacote(soquete, endereco);
    //     imprimePacote(pacoteRecebido);

    //     if (validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, sequencializacao) && getTipoPacote(pacoteRecebido) == CA)
    //     {
    //         int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

    //         retornoVer = realloc(retornoVer, tamanhoVer + tamanhoRetorno);

    //         for (int i = 0; i < tamanhoRetorno; i++)
    //         {
    //             retornoVer[tamanhoVer + i] = pacoteRecebido.dados[i];
    //         }
    //         tamanhoVer += tamanhoRetorno;

    //         aumentaSequencia(&sequencializacao);
    //         enviarACKParaServidor(soquete, endereco, sequencializacao);
    //     }
    // } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

    // puts("FIM TRANSMISSAO");
    // printf("***** CONTEUDO VER: \n%s\n", retornoVer);

    // ======== FIM VER

    // ============= LINHAS
    // do
    // {
    //     pacoteRecebido = lerPacote(soquete, endereco);
    //     imprimePacote(pacoteRecebido);
    // } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, sequencializacao)));

    // puts("Pacote Recebido!");

    // if (getTipoPacote(pacoteRecebido) == ACK)
    // {
    //     puts("ACK! Passando o numero da linha");
    //     aumentaSequencia(&sequencializacao);

    //     int nrLinha = 1;
    //     int nrLinhaFinal = 4;

    //     dados2[0] = nrLinha >> 24;
    //     dados2[1] = nrLinha >> 16;
    //     dados2[2] = nrLinha >> 8;
    //     dados2[3] = nrLinha;

    //     dados2[4] = nrLinhaFinal >> 24;
    //     dados2[5] = nrLinhaFinal >> 16;
    //     dados2[6] = nrLinhaFinal >> 8;
    //     dados2[7] = nrLinhaFinal;

    //     pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 8, sequencializacao, LIF, dados2);
    //     printf("NR da linha = %d até \n", getIntDados(pacote, 0),getIntDados(pacote, 1) );

    //     enviaPacote(pacote, soquete, endereco);

    //     do
    //     {
    //         pacoteRecebido = lerPacote(soquete, endereco);
    //         imprimePacote(pacoteRecebido);

    //         if (validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, sequencializacao) && getTipoPacote(pacoteRecebido) == CA)
    //         {
    //             int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

    //             retornoLinha = realloc(retornoLinha, tamanhoLinha + tamanhoRetorno);

    //             for (int i = 0; i < tamanhoRetorno; i++)
    //             {
    //                 retornoLinha[tamanhoLinha + i] = pacoteRecebido.dados[i];
    //             }
    //             tamanhoLinha += tamanhoRetorno;

    //             aumentaSequencia(&sequencializacao);
    //             enviarACKParaServidor(soquete, endereco, sequencializacao);
    //         }
    //     } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

    //     puts("FIM TRANSMISSAO");
    //     printf("***** CONTEUDO LINHA: %s\n", retornoLinha);
    // }

    // ============= FIM LINHAS

    //     do
    //     {
    //         pacoteRecebido = lerPacote(soquete, endereco);
    //         if (validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, sequencializacao))
    //         {

    //             if (!confereParidade(pacoteRecebido))
    //             {
    //                 puts("Paridade zoada");
    //                 // enviarNackParaServidor(soquete, endereco, serializacao);
    //             }
    //             else
    //             {

    // #ifdef DEBUG
    //                 printf("<<< RECEBENDO PACOTE <<<\n");
    //                 imprimePacote(pacote);
    //                 printf("=======================\n");
    // #endif
    //                 switch (getTipoPacote(pacoteRecebido))
    //                 {
    //                 case ACK:
    //                     printf("ACK!\n");
    //                     aumentaSequencia(&sequencializacao);
    //                     break;

    //                 case NACK:
    //                     printf("NACK!\n");
    //                     enviaPacote(pacote, soquete, endereco);
    //                     break;

    //                 case ERRO:
    //                     printf("ERRO!\n");
    //                     // Tratar erro ?
    //                     break;

    //                 case CLS:
    //                     printf("Conteudo LS!\n");

    //                     // aumentaSequencia(&sequencializacao);

    //                     // // Concatena a resposta
    //                     // int tamanhoDados = getTamanhoPacote(pacoteRecebido);
    //                     // retornoLS = realloc(retornoLS, tamanhoLS+tamanhoDados+1);
    //                     // puts(pacoteRecebido.dados);
    //                     // for (int i = 0 ; i < tamanhoDados; i++)
    //                     //     retornoLS[tamanhoLS+i] = pacoteRecebido.dados[i];
    //                     // tamanhoLS += tamanhoDados;
    //                     // puts(retornoLS);

    //                     // // Envia o ACK
    //                     // enviarACKParaServidor(soquete, endereco, sequencializacao);

    //                     do
    //                     {
    //                         if (validarLeituraCliente(pacoteRecebido) && (sequencializacao == getSequenciaPacote(pacoteRecebido)))
    //                         {

    //                             aumentaSequencia(&sequencializacao);

    //                             int tamanhoDados = getTamanhoPacote(pacoteRecebido);
    //                             retornoLS = realloc(retornoLS, tamanhoLS + tamanhoDados + 1);

    //                             for (int i = 0; i < tamanhoDados; i++)
    //                                 retornoLS[tamanhoLS + i] = pacoteRecebido.dados[i];

    //                             tamanhoLS += tamanhoDados;

    //                             enviarACKParaServidor(soquete, endereco, sequencializacao);

    //                             // puts("RETORNO ATE AGORA:");
    //                             // puts(retornoLS);
    //                             // puts("_____________");

    //                             // aumentaSequencia(&sequencializacao);
    //                         }
    //                         pacoteRecebido = lerPacote(soquete, endereco);
    //                     } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);
    //                     printf("222 FIM DA TRANSMISSAO!\n");

    //                     printf("%s", retornoLS);
    //                     break;

    //                 case CA:
    //                     printf("Conteudo Arquivo!\n");

    //                     pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 0, sequencializacao, LINHA, dados2);

    //                     break;

    //                 case FIM_TRANS:
    //                     printf("FIM DA TRANSMISSAO!\n");

    //                     printf("%s", retornoLS);

    //                     break;

    //                 default:
    //                     break;
    //                 }
    //             }
    //         }
    //     } while (validaTipoPacote(pacoteRecebido));

    return 0;
}
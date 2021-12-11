#include "utils.h"
#include "funcs.h"

// #define DEBUG

void lancarErro(int tipoErro)
{
    switch (tipoErro)
    {
    case ERR_AP:
        printf("ERRO: Você não tem permissão para executar essa ação\n");
        break;

    case ERR_DI:
        printf("ERRO: O diretório enviado não existe\n");
        break;

    case ERR_AI:
        printf("ERRO: O arquivo alvo não existe\n");
        break;

    case ERR_LI:
        printf("ERRO: A linha alvo não existe\n");
        break;
    }
}

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
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

    imprimePacote(pacoteRecebido);

    if (getTipoPacote(pacoteRecebido) == ACK)
    {
        aumentaSequencia(&seq);
    }
    else if (getTipoPacote(pacoteRecebido) == NACK)
    {
        // envia o comando novamente
        aumentaSequencia(&seq);
        cdCliente(soquete, &seq, diretorio);
    }
    else if (getTipoPacote(pacoteRecebido) == ERRO)
    {
        puts("ERRO!");
        int tipoErro = getIntDados(pacoteRecebido, 0);
        printf("tipo: %d\n", tipoErro);
        lancarErro(tipoErro);
        aumentaSequencia(&seq);
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
        printf("<<< RECEBENDO PACOTE <<<\n");
        imprimePacote(pacoteRecebido);

        if (getTipoPacote(pacoteRecebido) == CLS)
        {
            aumentaSequencia(&seq);

            if (confereParidade(pacoteRecebido))
            {
                int tamanhoDados = getTamanhoPacote(pacoteRecebido);
                retornoLS = realloc(retornoLS, tamanhoLS + tamanhoDados);
                for (int i = 0; i < tamanhoDados; i++)
                    retornoLS[tamanhoLS + i] = pacoteRecebido.dados[i];

                tamanhoLS += tamanhoDados;

                enviarACKParaServidor(soquete, endereco, seq);
            }
            else
            {
                enviarNACKParaServidor(soquete, endereco, seq);
            }
        }

    } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);
    retornoLS = realloc(retornoLS, tamanhoLS + 1);
    retornoLS[tamanhoLS] = '\0';

    puts(retornoLS);

    aumentaSequencia(&seq);
    *sequencializacao = seq;
}

void linhaCliente(int soquete, int *sequencializacao, char *arquivo, int nrLinha)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    struct sockaddr_ll endereco;
    int seq = *sequencializacao;
    char *retornoLinha = malloc(1);
    int tamanhoLinha = 0;

    pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, tamanhoString(arquivo), seq, LINHA, arquivo);
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

    if (getTipoPacote(pacoteRecebido) == ACK)
    {
        aumentaSequencia(&seq);

        char dados[4];
        for (int i = 0; i < 4; i++)
            dados[i] = nrLinha >> (24 - 8 * i);

        do
        {
            pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 4, seq, LIF, dados);
            enviaPacote(pacoteEnvio, soquete, endereco);

            do
            {
                pacoteRecebido = lerPacote(soquete, endereco);
            } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
            imprimePacote(pacoteRecebido);
            aumentaSequencia(&seq);
        } while (getTipoPacote(pacoteRecebido) == NACK);

        if (!(getTipoPacote(pacoteRecebido) == FIM_TRANS))
        {
            do
            {
                if (getTipoPacote(pacoteRecebido) == CA)
                {
                    int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

                    retornoLinha = realloc(retornoLinha, tamanhoLinha + tamanhoRetorno);

                    for (int i = 0; i < tamanhoRetorno; i++)
                        retornoLinha[tamanhoLinha + i] = pacoteRecebido.dados[i];

                    tamanhoLinha += tamanhoRetorno;

                    enviarACKParaServidor(soquete, endereco, seq);
                }

                do
                {
                    pacoteRecebido = lerPacote(soquete, endereco);
                } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

                aumentaSequencia(&seq);

            } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);
        }

        if (strlen(retornoLinha) == 0)
        {
            lancarErro(4);
            *sequencializacao = seq;
            return;
        }
        else
        {
            puts(retornoLinha);
        }
    }
    else if (getTipoPacote(pacoteRecebido) == NACK)
    {
        aumentaSequencia(&seq);
        linhaCliente(soquete, &seq, arquivo, nrLinha);
    }
    else if (getTipoPacote(pacoteRecebido) == ERRO)
    {
        int tipoErro = getIntDados(pacoteRecebido, 0);
        lancarErro(tipoErro);
        aumentaSequencia(&seq);
        *sequencializacao = seq;
        return;
    }

    *sequencializacao = seq;
}

void linhasCliente(int soquete, int *sequencializacao, char *arquivo, int linhaInicial, int linhaFinal)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    struct sockaddr_ll endereco;
    int seq = *sequencializacao;
    char *retornoLinha = malloc(1);
    int tamanhoLinha = 0;

    pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, tamanhoString(arquivo), seq, LINHAS, arquivo);
    enviaPacote(pacoteEnvio, soquete, endereco);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
    } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

    if (getTipoPacote(pacoteRecebido) == ACK)
    {
        aumentaSequencia(&seq);

        char dados[8];
        for (int i = 0; i < 4; i++)
            dados[i] = linhaInicial >> (24 - 8 * i);
        for (int i = 0; i < 4; i++)
            dados[i + 4] = linhaFinal >> (24 - 8 * i);

        do
        {
            pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 8, seq, LIF, dados);
            enviaPacote(pacoteEnvio, soquete, endereco);

            do
            {
                pacoteRecebido = lerPacote(soquete, endereco);
            } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
            imprimePacote(pacoteRecebido);
            aumentaSequencia(&seq);
        } while (getTipoPacote(pacoteRecebido) == NACK);

        if (!(getTipoPacote(pacoteRecebido) == FIM_TRANS))
        {
            do
            {
                if (getTipoPacote(pacoteRecebido) == CA)
                {
                    int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

                    retornoLinha = realloc(retornoLinha, tamanhoLinha + tamanhoRetorno);

                    for (int i = 0; i < tamanhoRetorno; i++)
                        retornoLinha[tamanhoLinha + i] = pacoteRecebido.dados[i];

                    tamanhoLinha += tamanhoRetorno;

                    enviarACKParaServidor(soquete, endereco, seq);
                }

                do
                {
                    pacoteRecebido = lerPacote(soquete, endereco);
                } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

                aumentaSequencia(&seq);

            } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);
        }

        if (strlen(retornoLinha) == 0)
        {
            lancarErro(4);
            *sequencializacao = seq;
            return;
        }
        else
        {
            puts(retornoLinha);
        }
    }
    else if (getTipoPacote(pacoteRecebido) == NACK)
    {
        aumentaSequencia(&seq);
        linhasCliente(soquete, &seq, arquivo, linhaInicial, linhaFinal);
    }
    else if (getTipoPacote(pacoteRecebido) == ERRO)
    {
        int tipoErro = getIntDados(pacoteRecebido, 0);
        lancarErro(tipoErro);
        aumentaSequencia(&seq);
        *sequencializacao = seq;
        return;
    }

    *sequencializacao = seq;
}

void verCliente(int soquete, int *sequencializacao, char *arquivo)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;
    char *retornoVer = malloc(1);
    int tamanhoVer = 0;
    struct sockaddr_ll endereco;

    do
    {
        pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, tamanhoString(arquivo), seq, VER, arquivo);

        enviaPacote(pacoteEnvio, soquete, endereco);
        aumentaSequencia(&seq);

        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
        printf("<<< RECEBENDO PACOTE <<<\n");
        imprimePacote(pacoteRecebido);

    } while (getTipoPacote(pacoteRecebido) == NACK);

    if (getTipoPacote(pacoteRecebido) == ERRO)
    {
        lancarErro(getIntDados(pacoteRecebido, 0));
        aumentaSequencia(&seq);
        *sequencializacao = seq;
        return;
    }

    do
    {
        if (getTipoPacote(pacoteRecebido) == CA)
        {

            int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

            retornoVer = realloc(retornoVer, tamanhoVer + tamanhoRetorno);

            for (int i = 0; i < tamanhoRetorno; i++)
                retornoVer[tamanhoVer + i] = pacoteRecebido.dados[i];

            tamanhoVer += tamanhoRetorno;

            enviarACKParaServidor(soquete, endereco, seq);

            do
            {
                pacoteRecebido = lerPacote(soquete, endereco);
            } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
            aumentaSequencia(&seq);
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
    struct sockaddr_ll endereco;
    int seq = *sequencializacao;

    // ENVIA O PACOTE COM O NOME DO ARQUIVO
    do
    {
        pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, tamanhoString(arquivo), seq, EDIT, arquivo);
        enviaPacote(pacoteEnvio, soquete, endereco);

        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

        aumentaSequencia(&seq);

    } while (getTipoPacote(pacoteRecebido) == NACK);

    if (getTipoPacote(pacoteRecebido) == ERRO)
    {
        lancarErro(getIntDados(pacoteRecebido, 0));
        aumentaSequencia(&seq);
        *sequencializacao = seq;
        return;
    }

    char dados[4];
    for (int i = 0; i < 4; i++)
        dados[i] = nrLinha << (24 - 8 * i);

    // ENVIA O PACOTE COM O NÚMERO DA LINHA
    do
    {
        pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 4, seq, LIF, dados);
        enviaPacote(pacoteEnvio, soquete, endereco);

        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraCliente(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

        aumentaSequencia(&seq);

    } while (getTipoPacote(pacoteRecebido) == NACK);

    if (getTipoPacote(pacoteRecebido) == ERRO)
    {
        lancarErro(getIntDados(pacoteRecebido, 0));
        aumentaSequencia(&seq);
        *sequencializacao = seq;
        return;
    }

    // Envia o conteúdo

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
            tamanhoAtual = tamanhoRestante;

        char *conteudoAtual = malloc(tamanhoAtual);
        for (int i = 0; i < tamanhoAtual; i++)
            conteudoAtual[i] = conteudo[contadorPacotes * 15 + i];

        pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, tamanhoAtual, seq, CA, conteudoAtual);
        enviaPacote(pacoteEnvio, soquete, endereco);
        aumentaSequencia(&seq);

        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

        if (getTipoPacote(pacoteRecebido) == ACK)
            contadorPacotes++;
        else
            tamanhoRestante += 15;
    }

    pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 0, seq, FIM_TRANS, NULL);
    enviaPacote(pacoteEnvio, soquete, endereco);
    aumentaSequencia(&seq);

    *sequencializacao = seq;
}

void velhoeditarCliente(int soquete, int *sequencializacao, char *arquivo, char *conteudo, int nrLinha)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;

    pacoteEnvio = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, tamanhoString(arquivo), seq, EDIT, arquivo);

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

void compilarCliente(int soquete, int *sequencializacao, char *arquivo, char **argumentos, int nrArgumentos)
{
    pacote_t pacoteEnvio, pacoteRecebido;
    int seq = *sequencializacao;
    char *args = malloc(1);

    pacoteEnvio = empacota(INIT_MARK,
                           SERVER_ADDR,
                           CLIENT_ADDR,
                           tamanhoString(arquivo),
                           seq,
                           COMPILAR,
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

        for (int i = 0; i < nrArgumentos; i++)
        {
            char *argumentoAtual = argumentos[i];
            args = realloc(args, strlen(args) + strlen(argumentoAtual) + 1);
            args = strcat(args, " ");
            args = strcat(args, argumentoAtual);
        }

        puts(args);

        pacote_t pacoteEnvio;
        int tamanhoRestante = strlen(args);
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
                conteudoAtual[i] = args[contadorPacotes * 15 + i];
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

    // A partir daqui vai receber o retorno da compilacao
    char *retornoVer = malloc(1);
    int tamanhoVer = 0;

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
    printf("***** RetornoCompilacao:\n%s", retornoVer);

    aumentaSequencia(&seq);
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
    else if (!strcmp(comando, "lls"))
    {
        return LLS;
    }
    else if (!strcmp(comando, "lcd"))
    {
        return LCD;
    }
    else if (!strcmp(comando, "seq"))
    {
        return VER_SEQ;
    }
    else if (!strcmp(comando, "clr"))
    {
        return CLR;
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
    char **argumentos = (char **)malloc(sizeof(char *) * 100);
    int linha, linhaFinal, nrArgumentos;

    configuraInicio(&soquete, &endereco);

    char delim[] = " ";
    char delimEdit[] = "\"";

    while (1)
    {
        printf("Comando: ");
        fgets(entrada, 100, stdin);
        entrada[strlen(entrada) - 1] = '\0';
        if (strlen(entrada) == 0)
            continue; // Trata o enter vazio :)

        char delim[] = " ";
        char *cmd = strtok(entrada, delim);
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
            nrArgumentos = 0;
            argumentos[nrArgumentos] = strtok(NULL, delim);
            while (argumentos[nrArgumentos] != NULL)
            {
                nrArgumentos++;
                argumentos[nrArgumentos] = strtok(NULL, delim);
            }

            arquivo = argumentos[nrArgumentos - 1];
            compilarCliente(soquete, &sequencializacao, arquivo, argumentos, nrArgumentos - 1);
            break;

        case VER_SEQ:
            printf("sequencia atual: %d\n", sequencializacao);
            break;

        case LCD:
            diretorio = strtok(NULL, delim);
            printf("O diretorio é: %s\n", diretorio);
            int retorno = cd(diretorio);
            if (retorno)
                lancarErro(retorno);
            break;

        case LLS:
            printf("%s", ls());
            break;

        case CLR:
            for (int i = 0; i < 100; i++)
                putchar('\n');
            break;

        default:
            printf("Comando inválido\n");
        }
    }

    return 0;
}
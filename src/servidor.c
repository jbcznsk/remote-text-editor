#include "utils.h"
#include "funcs.h"

#define DEBUG

void cdServidor(int soquete, int *sequencializacao, pacote_t pacote)
{
    struct sockaddr_ll endereco;
    char buff[100];
    printf("Diretorio atual: %s\n", getcwd(buff, 100));
    int tamanho = getTamanhoPacote(pacote);
    char *dir = (char *)malloc(sizeof(char) * tamanho);
    for (int i = 0; i < tamanho; i++)
        dir[i] = pacote.dados[i];

    int retorno = cd(dir);
    free(dir);
    if (retorno != 0)
    {
        printf("ERRO: %d\n", retorno);
        enviarErroParaCLiente(soquete, endereco, *sequencializacao, retorno);
        aumentaSequencia(sequencializacao);
    }
    else if (!confereParidade(pacote))
    {
        enviarNACKParaCliente(soquete, endereco, *sequencializacao);
        aumentaSequencia(sequencializacao);
    }
    else
    {
        printf("Diretorio atual: %s\n", getcwd(buff, 100));
        printf("======================= \n");
        enviarACKParaCliente(soquete, endereco, *sequencializacao);
        aumentaSequencia(sequencializacao);
    }
}

void lsServidor(int soquete, int *sequencializacao)
{
    int tamanhoRestante, tamanhoAtual, qtdPacotes, contadorPacotes, seq;
    char *conteudo;
    pacote_t pacoteEnvio;

    conteudo = ls();
    puts(conteudo);
    seq = *sequencializacao;
    tamanhoRestante = strlen(conteudo);
    qtdPacotes = (tamanhoRestante / 15) + ((tamanhoRestante % 15) ? 1 : 0);
    contadorPacotes = 0;

    printf("¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");
    printf("Quantidade Pacotes: %d\n", qtdPacotes);
    printf("¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨¨\n");

    while (contadorPacotes < qtdPacotes)
    {
        // montar o pacote
        if (tamanhoRestante > 15)
        {
            tamanhoAtual = 15;
            tamanhoRestante -= 15;
        }
        else
        {
            tamanhoAtual = tamanhoRestante;
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
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
        printf("<<< RECEBENDO PACOTE <<<\n");
        imprimePacote(pacoteRecebido);

        if (getTipoPacote(pacoteRecebido) == ACK)
        {
            puts("ACK!");
            contadorPacotes++;
        }
        else if (getTipoPacote(pacoteRecebido) == NACK)
        {
            puts("NACK!");
            tamanhoRestante += 15;
        }

        free(mensagemAtual);
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

    aumentaSequencia(&seq);
    *sequencializacao = seq;
    free(conteudo);
}

void linhaServidor(int soquete, int *sequencializacao, pacote_t pacote)
{
    int seq = *sequencializacao;
    struct sockaddr_ll endereco;
    int tamanhoRestante, tamanhoAtual, qtdPacotes, contadorPacotes;
    pacote_t pacoteEnvio;

    if (!confereParidade(pacote))
    {
        enviarNACKParaCliente(soquete, endereco, seq);
        aumentaSequencia(&seq);
        return;
    }

    char *nomeArquivo = getDadosPacote(pacote);
    printf("Nome do arquivo que vai ser lido: %s\n", nomeArquivo);
    FILE *ARQ = fopen(nomeArquivo, "rb");
    if (!ARQ)
    {
        enviarErroParaCLiente(soquete, endereco, seq, ERR_AI);
        aumentaSequencia(&seq);
        *sequencializacao = seq;
        return;
    }

    enviarACKParaCliente(soquete, endereco, seq);
    aumentaSequencia(&seq);

    pacote_t pacoteRecebido;
    int valida;
    do
    {
        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
        imprimePacote(pacoteRecebido);

        valida = confereParidade(pacoteRecebido);
        if (!valida){
            enviarNACKParaCliente(soquete, endereco, seq);
            aumentaSequencia(&seq);
        } 

    } while (!valida);

    int linha = getIntDados(pacoteRecebido, 0);
    printf("NR da linha = %d\n", linha);
    char comando[100];
    sprintf(comando, "sed -n %dp %s > linha", linha, nomeArquivo);
    system(comando);

    char *conteudoLinha;
    long tamanhoLinha;
    FILE *L = fopen("linha", "rb");

    if (L)
    {
        fseek(L, 0, SEEK_END);
        tamanhoLinha = ftell(L);
        fseek(L, 0, SEEK_SET);
        conteudoLinha = malloc(tamanhoLinha);
        if (conteudoLinha)
        {
            fread(conteudoLinha, 1, tamanhoLinha, L);
        }
        fclose(L);
    }

    puts(conteudoLinha);

    // Envio do conteudo da linha

    contadorPacotes = 0;
    tamanhoRestante = tamanhoLinha;
    qtdPacotes = (tamanhoLinha / 15) + ((tamanhoLinha % 15) ? 1 : 0);

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

        char *linhaAtual = malloc(tamanhoAtual);

        for (int i = 0; i < tamanhoAtual; i++)
        {
            linhaAtual[i] = conteudoLinha[contadorPacotes * 15 + i];
        }

        pacoteEnvio = empacota(INIT_MARK,
                               CLIENT_ADDR,
                               SERVER_ADDR,
                               tamanhoAtual,
                               seq,
                               CA,
                               linhaAtual);

        struct sockaddr_ll endereco;
        enviaPacote(pacoteEnvio, soquete, endereco);
        aumentaSequencia(&seq);

        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
            imprimePacote(pacoteRecebido);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

        if (getTipoPacote(pacoteRecebido) == ACK)
        {
            puts("ACK!");
            contadorPacotes++;
        }
        else
        {
            puts("NACK!");
            tamanhoRestante += 15;
        }
    }

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
    aumentaSequencia(&seq);
    *sequencializacao = seq;
    system("rm linha");
}

void linhasServidor(int soquete, int *sequencializacao, pacote_t pacote)
{
    int seq = *sequencializacao;
    struct sockaddr_ll endereco;
    int tamanhoRestante, tamanhoAtual, qtdPacotes, contadorPacotes;
    pacote_t pacoteEnvio;

    if (!confereParidade(pacote))
    {
        enviarNACKParaCliente(soquete, endereco, seq);
        aumentaSequencia(&seq);
        return;
    }

    char *nomeArquivo = getDadosPacote(pacote);
    printf("Nome do arquivo que vai ser lido: %s\n", nomeArquivo);
    FILE *ARQ = fopen(nomeArquivo, "rb");
    if (!ARQ)
    {
        enviarErroParaCLiente(soquete, endereco, seq, ERR_AI);
        aumentaSequencia(&seq);
        *sequencializacao = seq;
        return;
    }

    enviarACKParaCliente(soquete, endereco, seq);
    aumentaSequencia(&seq);

    pacote_t pacoteRecebido;
    int valida;
    do
    {
        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
        imprimePacote(pacoteRecebido);

        valida = confereParidade(pacoteRecebido);
        if (!valida){
            enviarNACKParaCliente(soquete, endereco, seq);
            aumentaSequencia(&seq);
        } 

    } while (!valida);
        
    int linhaInicial = getIntDados(pacoteRecebido, 0);
    int linhaFinal = getIntDados(pacoteRecebido, 1);
    printf("NR das linhas = %d até %d\n", linhaInicial, linhaFinal);
    char comando[100];
    sprintf(comando, "sed -n %d,%dp %s > linha", linhaInicial, linhaFinal, nomeArquivo);
    system(comando);

    char *conteudoLinha;
    long tamanhoLinha;
    FILE *L = fopen("linha", "rb");

    if (L)
    {
        fseek(L, 0, SEEK_END);
        tamanhoLinha = ftell(L);
        fseek(L, 0, SEEK_SET);
        conteudoLinha = malloc(tamanhoLinha);
        if (conteudoLinha)
            fread(conteudoLinha, 1, tamanhoLinha, L);
        fclose(L);
    }

    puts(conteudoLinha);

    // Envio do conteudo da linha

    contadorPacotes = 0;
    tamanhoRestante = tamanhoLinha;
    qtdPacotes = (tamanhoLinha / 15) + ((tamanhoLinha % 15) ? 1 : 0);

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

        char *linhaAtual = malloc(tamanhoAtual);

        for (int i = 0; i < tamanhoAtual; i++)
        {
            linhaAtual[i] = conteudoLinha[contadorPacotes * 15 + i];
        }

        pacoteEnvio = empacota(INIT_MARK,
                               CLIENT_ADDR,
                               SERVER_ADDR,
                               tamanhoAtual,
                               seq,
                               CA,
                               linhaAtual);

        struct sockaddr_ll endereco;
        enviaPacote(pacoteEnvio, soquete, endereco);
        aumentaSequencia(&seq);

        do
        {
            pacoteRecebido = lerPacote(soquete, endereco);
            imprimePacote(pacoteRecebido);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

        if (getTipoPacote(pacoteRecebido) == ACK)
        {
            puts("ACK!");
            contadorPacotes++;
        }
        else
        {
            puts("NACK!");
            tamanhoRestante += 15;
        }
    }

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
    aumentaSequencia(&seq);
    *sequencializacao = seq;
    system("rm linha");
}

void verServidor(int soquete, int *sequencializacao, pacote_t pacote)
{
    int seq = *sequencializacao;
    struct sockaddr_ll endereco;
    char *nomeArquivo = getDadosPacote(pacote);
    int tamanhoRestante, tamanhoAtual, qtdPacotes, contadorPacotes;
    pacote_t pacoteEnvio, pacoteRecebido;

    char comando[100];
    sprintf(comando, "cat -n %s | sed 's/^ *//g' > VER", nomeArquivo);
    system(comando);

    char *conteudo;
    long tamanhoArquivo;
    FILE *ARQUIVO = fopen("VER", "rb");

    if (ARQUIVO)
    {
        fseek(ARQUIVO, 0, SEEK_END);
        tamanhoArquivo = ftell(ARQUIVO);
        fseek(ARQUIVO, 0, SEEK_SET);
        conteudo = malloc(tamanhoArquivo);
        if (conteudo)
        {
            fread(conteudo, 1, tamanhoArquivo, ARQUIVO);
        }
        fclose(ARQUIVO);
    }

    puts(conteudo);

    contadorPacotes = 0;
    tamanhoRestante = tamanhoArquivo;
    qtdPacotes = (tamanhoArquivo / 15) + ((tamanhoArquivo % 15) ? 1 : 0);

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
                               CLIENT_ADDR,
                               SERVER_ADDR,
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
            imprimePacote(pacoteRecebido);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
#ifdef DEBUG
        printf("<<< RECEBENDO PACOTE <<<\n");
        // imprimePacote(pacote);
        printf("=======================\n");
#endif

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

    pacoteEnvio = empacota(INIT_MARK,
                           CLIENT_ADDR,
                           SERVER_ADDR,
                           0,
                           seq,
                           FIM_TRANS,
                           NULL);

    // enviar a mensagem
    enviaPacote(pacoteEnvio, soquete, endereco);
    aumentaSequencia(&seq);
    free(nomeArquivo);
    *sequencializacao = seq;
    system("rm VER");
}

void editarServidor(int soquete, int *sequencializacao, pacote_t pacote)
{
    int seq = *sequencializacao;
    struct sockaddr_ll endereco;
    char *nomeArquivo = getDadosPacote(pacote);
    int nrLinha;
    pacote_t pacoteRecebido;

    char *novaLinha = malloc(1);
    int tamanhoNovaLinha = 0;

    enviarACKParaCliente(soquete, endereco, seq);
    aumentaSequencia(&seq);

    printf("Arquivo que vai ser editado :%s\n", nomeArquivo);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
        imprimePacote(pacoteRecebido);
    } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));

    nrLinha = getIntDados(pacoteRecebido, 0);

    printf("Linha que vai ser editada: %d\n", nrLinha);

    enviarACKParaCliente(soquete, endereco, seq);
    aumentaSequencia(&seq);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
        imprimePacote(pacoteRecebido);

        if (validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq) && getTipoPacote(pacoteRecebido) == CA)
        {
            int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

            novaLinha = realloc(novaLinha, tamanhoNovaLinha + tamanhoRetorno);

            for (int i = 0; i < tamanhoRetorno; i++)
            {
                novaLinha[tamanhoNovaLinha + i] = pacoteRecebido.dados[i];
            }
            tamanhoNovaLinha += tamanhoRetorno;

            aumentaSequencia(&seq);
            enviarACKParaServidor(soquete, endereco, seq);
        }
    } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

    int nrDeLinhas = 0;
    char c;
    FILE *ARQUIVO = fopen(nomeArquivo, "rb");
    if (ARQUIVO)
    {
        do
        {
            c = getc(ARQUIVO);
            putc(c, stdout);
            if (c == '\n')
                nrDeLinhas++;
        } while (c != EOF);
    }
    else
    {
        enviarErroParaCLiente(soquete, endereco, seq, ERR_AI);
        aumentaSequencia(&seq);
        *sequencializacao = seq;
    }
    fclose(ARQUIVO);

    printf("Numero de linhas do arquivo: %d\n", nrDeLinhas);

    if (nrLinha > nrDeLinhas + 1) // erro de linha invalida
    {
        puts("LINHA INVALIDA");
        enviarErroParaCLiente(soquete, endereco, seq, ERR_LI);
    }
    else if (nrLinha == nrDeLinhas + 1) // append no final do arquivo
    {
        puts("APPENDS");
        char comando[100];
        sprintf(comando, "sed -i '%d a\\%s' %s", nrLinha - 1, novaLinha, nomeArquivo);
        system(comando);
    }
    else // substitui a linha
    {
        puts("SUBSTITUICAO");
        char comando[100];
        sprintf(comando, "sed -i '%dc\\%s' %s", nrLinha, novaLinha, nomeArquivo);
        system(comando);
    }

    *sequencializacao = seq;
    free(nomeArquivo);
}

void compilarServidor(int soquete, int *sequencializacao, pacote_t pacote)
{
    int seq = *sequencializacao;
    struct sockaddr_ll endereco;
    char *nomeArquivo = getDadosPacote(pacote);
    int tamanhoRestante, tamanhoAtual, qtdPacotes, contadorPacotes;
    pacote_t pacoteEnvio, pacoteRecebido;

    char *novaLinha = malloc(1);
    int tamanhoNovaLinha = 0;

    enviarACKParaCliente(soquete, endereco, seq);
    aumentaSequencia(&seq);

    printf("Arquivo que vai ser compilado :%s\n", nomeArquivo);

    do
    {
        pacoteRecebido = lerPacote(soquete, endereco);
        imprimePacote(pacoteRecebido);

        if (validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq) && getTipoPacote(pacoteRecebido) == CA)
        {
            int tamanhoRetorno = getTamanhoPacote(pacoteRecebido);

            novaLinha = realloc(novaLinha, tamanhoNovaLinha + tamanhoRetorno);

            for (int i = 0; i < tamanhoRetorno; i++)
            {
                novaLinha[tamanhoNovaLinha + i] = pacoteRecebido.dados[i];
            }
            tamanhoNovaLinha += tamanhoRetorno;

            aumentaSequencia(&seq);
            enviarACKParaServidor(soquete, endereco, seq);
        }
    } while (getTipoPacote(pacoteRecebido) != FIM_TRANS);

    char comando[200];
    sprintf(comando, "gcc %s %s 2>&1 | cat > retornoCompilar", novaLinha, nomeArquivo);
    system(comando);

    // Enviar o retorno para o cliente

    char *conteudo;
    long tamanhoArquivo;
    FILE *ARQUIVO = fopen("retornoCompilar", "rb");

    if (ARQUIVO)
    {
        fseek(ARQUIVO, 0, SEEK_END);
        tamanhoArquivo = ftell(ARQUIVO);
        fseek(ARQUIVO, 0, SEEK_SET);
        conteudo = malloc(tamanhoArquivo);
        if (conteudo)
        {
            fread(conteudo, 1, tamanhoArquivo, ARQUIVO);
        }
        fclose(ARQUIVO);
    }

    puts(conteudo);

    contadorPacotes = 0;
    tamanhoRestante = tamanhoArquivo;
    qtdPacotes = (tamanhoArquivo / 15) + ((tamanhoArquivo % 15) ? 1 : 0);

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
                               CLIENT_ADDR,
                               SERVER_ADDR,
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
            imprimePacote(pacoteRecebido);
        } while (!(validarLeituraServidor(pacoteRecebido) && validarSequencializacao(pacoteRecebido, seq)));
#ifdef DEBUG
        printf("<<< RECEBENDO PACOTE <<<\n");
        // imprimePacote(pacote);
        printf("=======================\n");
#endif

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

    pacoteEnvio = empacota(INIT_MARK,
                           CLIENT_ADDR,
                           SERVER_ADDR,
                           0,
                           seq,
                           FIM_TRANS,
                           NULL);

    // enviar a mensagem
    enviaPacote(pacoteEnvio, soquete, endereco);

    system("rm retornoCompilar");

    aumentaSequencia(&seq);
    *sequencializacao = seq;
    free(nomeArquivo);
}

int main()
{
    puts("Iniciando Servidor...");
    pacote_t pacote;
    int sequencializacao = 0;

    int soquete;
    struct sockaddr_ll endereco;
    configuraInicio(&soquete, &endereco);

    // --------------- //

    while (1)
    {
        printf("sequencia atual: %d\n", sequencializacao);
        // Aqui eu sempre vou ter um pacote válido (falta a paridade mas fodase)
        do
        {
            pacote = lerPacote(soquete, endereco);
        } while (!(validarLeituraServidor(pacote) && validarSequencializacao(pacote, sequencializacao)));
        printf("<<< RECEBENDO PACOTE <<<\n");
        imprimePacote(pacote);

        switch (getTipoPacote(pacote))
        {
        case CD:
            printf("===== Comando: CD ===== \n");
            cdServidor(soquete, &sequencializacao, pacote);
            printf("=====   FIM  : CD ===== \n");
            break;

        case LS:
            printf("===== Comando: LS ===== \n");
            lsServidor(soquete, &sequencializacao);
            printf("=====   FIM  : LS ===== \n");
            break;

        case LINHA:
            printf("===== Comando: LINHA ===== \n");
            linhaServidor(soquete, &sequencializacao, pacote);
            printf("=====   FIM  : LINHA ===== \n");
            break;

        case LINHAS:
            printf("===== Comando: LINHAS ===== \n");
            linhasServidor(soquete, &sequencializacao, pacote);
            printf("=====   FIM  : LINHAS ===== \n");
            break;

        case VER:
            printf("===== Comando: VER ===== \n");
            verServidor(soquete, &sequencializacao, pacote);
            printf("=====   FIM  : VER ===== \n");
            break;

        case EDIT:
            printf("===== Comando: EDITAR ===== \n");
            editarServidor(soquete, &sequencializacao, pacote);
            printf("=====   FIM  : EDITAR ===== \n");
            printf("******* SEQ ATUAL : %d\n", sequencializacao);
            break;

        case COMPILAR:
            printf("===== Comando: COMPILAR ===== \n");
            compilarServidor(soquete, &sequencializacao, pacote);
            printf("=====   FIM  : COMPILAR ===== \n");
            printf("******* SEQ ATUAL : %d\n", sequencializacao);
            break;

        default:
            break;
        }
    }

    return 0;
}
#include "utils.h"

#define DEBUG

void configuraInicio(int *soquete, struct sockaddr_ll *endereco)
{

    struct ifreq ir;
    struct packet_mreq mr;
    char *device = "lo";

    /*cria socket*/
    *soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (*soquete == -1)
    {
        printf("Erro no Socket\n");
        exit(-1);
    }

    struct timeval timeout;      
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    if (setsockopt (*soquete, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                sizeof timeout) < 0)
        perror("setsockopt failed\n");

    if (setsockopt (*soquete, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                sizeof timeout) < 0)
        perror("setsockopt failed\n");

//     int timeout = 10000;  // user timeout in milliseconds [ms]
// setsockopt (*soquete, 6, 18, (char*) &timeout, sizeof (timeout));

    /*dispositivo eth0*/
    memset(&ir, 0, sizeof(struct ifreq));
    // memcpy(ir.ifr_name, device, sizeof(device));
    memcpy(ir.ifr_name, device, 4);
    if (ioctl(*soquete, SIOCGIFINDEX, &ir) == -1)
    {
        printf("Erro no ioctl\n");
        exit(-1);
    }

    /*IP do dispositivo*/
    memset(endereco, 0, sizeof(*endereco));
    endereco->sll_family = AF_PACKET;
    endereco->sll_protocol = htons(ETH_P_ALL);
    endereco->sll_ifindex = ir.ifr_ifindex;
    if (bind(*soquete, (struct sockaddr *)endereco, sizeof(*endereco)) == -1)
    {
        printf("Erro no bind\n");
        exit(-1);
    }

    /*Modo Promiscuo*/
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(*soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)
    {
        printf("Erro ao fazer setsockopt\n");
        exit(-1);
    }
}


void printByte(char c)
{
    for (int i = 7; i >= 0; i--)
        printf("%d", ((c >> i) & 1) ? 1 : 0);
    printf("\n");
}

void printBits(char c, int ini, int fim)
{
    for (int i = ini; i >= fim; i--)
        printf("%d", ((c >> i) & 1) ? 1 : 0);
    printf("\n");
}

void imprimePacote(pacote_t pacote)
{

    printf("MI: ");
    printByte(pacote.MI);

    printf("Endereço Destino: ");
    printBits(pacote.EdEoTam, 7, 6);

    printf("Endereço Origem: ");
    printBits(pacote.EdEoTam, 5, 4);

    printf("Tamanho: ");
    printBits(pacote.EdEoTam, 3, 0);

    printf("Sequencia: ");
    printBits(pacote.SeqTipo, 7, 4);

    printf("Tipo: ");
    printBits(pacote.SeqTipo, 3, 0);

    int t = pacote.EdEoTam & 0b00001111;

    printf("Dados: ");
    for (int i = 0; i < t; i++)
        printf("%c", pacote.dados[i]);
    printf("\n");

    printf("Paridade: ");
    printByte(pacote.paridade);
}

char getTamanhoPacote(pacote_t pacote)
{
    return pacote.EdEoTam & 0b00001111;
}

char getSequenciaPacote(pacote_t pacote)
{
    return (pacote.SeqTipo & 0b11110000) >> 4;
}

char getTipoPacote(pacote_t pacote)
{
    return pacote.SeqTipo & 0b00001111;
}

char getEnderecoDestino(pacote_t pacote)
{
    return (pacote.EdEoTam >> 6) & 0b00000011;
}

char getEnderecoOrigem(pacote_t pacote)
{
    return (pacote.EdEoTam & 0b00110000) >> 4;
}

char *getDadosPacote(pacote_t pacote)
{
    char *dados = malloc (getTamanhoPacote(pacote)+1);
    for (int i = 0; i < getTamanhoPacote(pacote); i++)
        dados[i] = pacote.dados[i];
    dados[getTamanhoPacote(pacote)] = '\0';
    return dados;
}

char calculaParidade(pacote_t pacote)
{
    char tamanho   = getTamanhoPacote(pacote);
    char sequencia = getSequenciaPacote(pacote);
    char tipo      = getTipoPacote(pacote);

    char paridade = tamanho ^ sequencia ^ tipo;

    for (int i = 0; i < tamanho; i++){
        paridade ^= pacote.dados[i];
    }
    return paridade;
}

int confereParidade(pacote_t pacote)
{
    return (calculaParidade(pacote) == pacote.paridade);
}

pacote_t empacota(char MI, char enderecoDestino, char enderecoOrigem, char tamanho, char sequencia, char tipo, char dados[15])
{

    pacote_t pacote;
    for (int i = 0; i < 81; i++) pacote.pad[i] = '0';

    pacote.MI = MI;

    pacote.EdEoTam = enderecoDestino << 6;
    pacote.EdEoTam |= enderecoOrigem << 4;
    pacote.EdEoTam |= tamanho;

    pacote.SeqTipo = sequencia << 4;
    pacote.SeqTipo |= tipo;

    for (int i = 0; i < tamanho; i++)
        pacote.dados[i] = dados[i];

    if (tamanho == 0){
        for (int i = 0; i < tamanho; i++)
        pacote.dados[i] = ' ';
    }

    pacote.paridade = calculaParidade(pacote);

    return pacote;
}

int enviaPacote(pacote_t pacote, int soquete, struct sockaddr_ll endereco)
{   
    #ifdef DEBUG
        printf(">>> ENVIANDO PACOTE >>>\n");
        imprimePacote(pacote);
        printf("=======================\n");
    #endif

    return send(soquete, &pacote, 100, 0);
    //return sendto(soquete, &pacote, 100, 0, (struct sockaddr *)&endereco, sizeof(struct sockaddr_ll));
}

pacote_t lerPacote(int soquete, struct sockaddr_ll endereco)
{
    pacote_t pacote;
    // int tamanhoEndereco = sizeof(endereco);
    // recvfrom(soquete, &pacote, 100, 0, (struct sockaddr *)&endereco, &tamanhoEndereco);
    recv(soquete, &pacote, 100, 0);
    #ifdef DEBUG_1
        printf("<<< RECEBENDO PACOTE <<<\n");
        imprimePacote(pacote);
        printf("=======================\n");
    #endif
    return pacote;
}

void enviarACKParaCliente(int soquete, struct sockaddr_ll endereco, int sequencializacao)
{
    pacote_t pacote;
    pacote = empacota(INIT_MARK, CLIENT_ADDR, SERVER_ADDR, 0, sequencializacao, ACK, NULL);
    enviaPacote(pacote, soquete, endereco);
}

void enviarNACKParaCliente(int soquete, struct sockaddr_ll endereco, int sequencializacao)
{
    pacote_t pacote;
    pacote = empacota(INIT_MARK, CLIENT_ADDR, SERVER_ADDR, 0, sequencializacao, NACK, NULL);
    enviaPacote(pacote, soquete, endereco);
}

void enviarErroParaCLiente(int soquete, struct sockaddr_ll endereco, int sequencializacao, int erro){
    pacote_t pacote;
    char dados[4] = {erro};

    dados[0] = erro << 24;
    dados[1] = erro << 16;
    dados[2] = erro <<  8;
    dados[3] = erro      ;

    pacote = empacota(INIT_MARK, CLIENT_ADDR, SERVER_ADDR, 4, sequencializacao, ERRO, dados);
    enviaPacote(pacote, soquete, endereco);
}

void enviarACKParaServidor(int soquete, struct sockaddr_ll endereco, int sequencializacao)
{
    pacote_t pacote;
    pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 0, sequencializacao, ACK, NULL);
    enviaPacote(pacote, soquete, endereco);
}

void enviarNACKParaServidor(int soquete, struct sockaddr_ll endereco, int sequencializacao)
{
    pacote_t pacote;
    pacote = empacota(INIT_MARK, SERVER_ADDR, CLIENT_ADDR, 0, sequencializacao, NACK, NULL);
    enviaPacote(pacote, soquete, endereco);
}

double timestamp(void)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return((double)(tp.tv_sec*1000.0 + tp.tv_usec/1000.0));
}

int tamanhoString(char *string){
    int t = 0;
    while(string[t++] != '\0');
    return t-1;
}

void aumentaSequencia(int *sequencia){
    *sequencia = (*sequencia + 1) % 16;
    // printf("Seq atual: %d\n", *sequencia);
}

int validarSequencializacao(pacote_t pacote, int sequencializacao){
    return  getSequenciaPacote(pacote) == sequencializacao;
}

int validarLeituraCliente(pacote_t pacote)
{
    return ((getEnderecoOrigem(pacote) == SERVER_ADDR) && (getEnderecoDestino(pacote) == CLIENT_ADDR));
}

int validarLeituraServidor(pacote_t pacote)
{
    return ((getEnderecoDestino(pacote) == SERVER_ADDR) && (getEnderecoOrigem(pacote) == CLIENT_ADDR));
}

int getIntDados(pacote_t pacote, int deslocamento)
{
    int indice = 4 * deslocamento;
    int inteiro;

    inteiro =  pacote.dados[indice + 0] << 24;
    inteiro |= pacote.dados[indice + 1] << 16;
    inteiro |= pacote.dados[indice + 2] << 8;
    inteiro |= pacote.dados[indice + 3] << 0;

    return inteiro;
}
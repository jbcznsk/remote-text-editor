#include "utils.h"

void configuraInicio(int *soquete, struct sockaddr_ll *endereco){

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

    /*dispositivo eth0*/
    memset(&ir, 0, sizeof(struct ifreq)); 
    memcpy(ir.ifr_name, device, sizeof(device));
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

void padding(char *dados){
    for (int i = 19; i < 99; i++) 
        dados[i] = '0';
    dados[99] = '\0';
}

void printByte(char c){
    for(int i = 7; i >= 0; i--)
        printf("%d", ((c >> i) & 1) ? 1 : 0);
    printf("\n");
}

void printBits(char c, int ini, int fim){
    for(int i = ini; i >= fim; i--)
        printf("%d", ((c >> i) & 1) ? 1 : 0);
    printf("\n");
}

void imprimePacote(pacote_t pacote){
    

    printf("MI: ");
    printByte(pacote.MI);
    
    printf("Endereço Destino: ");
    printBits(pacote.EdEoTam, 7,6);

    printf("Endereço Origem: ");
    printBits(pacote.EdEoTam, 5,4);

    printf("Tamanho: ");
    printBits(pacote.EdEoTam, 3,0);

    printf("Sequencia: ");
    printBits(pacote.SeqTipo, 7,4);

    printf("Tipo: ");
    printBits(pacote.SeqTipo, 3,0);

    int t = pacote.EdEoTam & 0b00001111;

    printf("Dados: ");
    for (int i = 0; i < t; i++)
        printf("%c", pacote.dados[i]);
    printf("\n");

    printf("Paridade: ");
    printByte(pacote.paridade);
}

char getTamanhoPacote(pacote_t pacote){
    return pacote.EdEoTam & 0b00001111;
}

char getSequenciaPacote(pacote_t pacote){
    return (pacote.SeqTipo & 0b11110000) >> 4;
}

char getTipoPacote(pacote_t pacote){
    return pacote.SeqTipo & 0b00001111;
}

char calculaParidade(pacote_t pacote){
    
    char tamanho    = getTamanhoPacote(pacote);
    char sequencia  = getSequenciaPacote(pacote);
    char tipo       = getTipoPacote(pacote);

    char paridade = tamanho ^ sequencia ^ tipo;

    for (int i = 0; i < tamanho; i++)
        paridade ^= pacote.dados[i];

    return paridade;
}

pacote_t empacota(char MI, char enderecoDestino, char enderecoOrigem, char tamanho, char sequencia, char tipo, char dados[15], char paridade){
    
    pacote_t pacote;

    pacote.MI = MI;
    
    pacote.EdEoTam =  enderecoDestino << 6;
    pacote.EdEoTam |= enderecoOrigem  << 4;
    pacote.EdEoTam |= tamanho;

    pacote.SeqTipo =  sequencia << 4;
    pacote.SeqTipo |= tipo;

    for (int i = 0; i < 15; i++) 
        pacote.dados[i] = dados[i];

    pacote.paridade = paridade;

    return pacote;
}

int enviaPacote(pacote_t pacote, int soquete, struct sockaddr_ll endereco){
    return sendto(soquete, &pacote, 100, 0,(struct sockaddr*)&endereco, sizeof(struct sockaddr_ll));
}
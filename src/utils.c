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

#ifndef __UTILS__
#define __UTILS__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>

#define INIT_MARK 0b01111110

#define SERVER_ADDR 0b10
#define CLIENT_ADDR 0b01

#define SEQ_SIZE 4 

#define STD_MSG_SIZE 19

#define CD        0b0000 //  0
#define LS        0b0001 //  1
#define VER       0b0010 //  2
#define LINHA     0b0011 //  3
#define LINHAS    0b0100 //  4
#define EDIT      0b0101 //  5
#define COMPILAR  0b0110 //  6
#define ND_0      0b0111 //  7
#define ACK       0b1000 //  8
#define NACK      0b1001 //  9
#define LIF       0b1010 // 10
#define CLS       0b1011 // 11
#define CA        0b1100 // 12
#define FIM_TRANS 0b1101 // 13
#define ND_1      0b1110 // 14
#define ERRO      0b1111 // 15

#define ERR_AP 1
#define ERR_DI 2
#define ERR_AI 3
#define ERR_LI 4


void configuraInicio(int *soquete, struct sockaddr_ll *endereco);
void padding(char *dados);


#endif
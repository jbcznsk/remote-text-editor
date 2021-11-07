#include <stdio.h>
#include <stdlib.h>

#define INIT_MARK 0b01111110

#define SERVER_ADDR 0b10
#define CLIENT_ADDR 0b01

#define SEQ_SIZE 4

#define CD        0b0000 
#define LS        0b0001
#define VER       0b0010
#define LINHA     0b0011
#define LINHAS    0b0100
#define EDIT      0b0101
#define COMPILAR  0b0110
#define ND_0      0b0111
#define ACK       0b1000
#define NACK      0b1001
#define LIF       0b1010
#define CLS       0b1011
#define CA        0b1100
#define FIM_TRANS 0b1101
#define ND_1      0b1110
#define ERRO      0b1111

#define ERR_AP 1
#define ERR_DI 2
#define ERR_AI 3
#define ERR_LI 4

int
main()
{

    printf("eu nao faço ideia do que fazer\n");

    return 0;
}
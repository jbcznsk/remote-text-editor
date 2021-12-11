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
#include <strings.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>

#define INIT_MARK 0b01111110

#define SERVER_ADDR 0b10
#define CLIENT_ADDR 0b01

#define VER_SEQ 90
#define CLR     91

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

#define LCD       16
#define LLS       17 

#define ERR_AP 1
#define ERR_DI 2
#define ERR_AI 3
#define ERR_LI 4

// tamanho : 19 bytes + 81 = 100 bytes por pacote
typedef struct pacote_s{
    char MI;
    char EdEoTam;
    char SeqTipo;
    char dados[15];
    char paridade;
    
    // Apenas para consistência na hora da comunicação
    char pad[81];
} pacote_t;


void configuraInicio(int *soquete, struct sockaddr_ll *endereco);
void padding(char *dados);
void printByte(char c);
void printBits(char c, int ini, int fim);
void imprimePacote(pacote_t pacote);

int getTamanhoPacote(pacote_t pacote);
char getSequenciaPacote(pacote_t pacote);
char getTipoPacote(pacote_t pacote);
char getEnderecoOrigem(pacote_t pacote);
char getEnderecoDestino(pacote_t pacote);
char *getDadosPacote(pacote_t pacote);


char calculaParidade(pacote_t pacote);
int confereParidade(pacote_t pacote);

int enviaPacote(pacote_t pacote, int soquete, struct sockaddr_ll endereco);
pacote_t lerPacote(int soquete, struct sockaddr_ll endereco);

pacote_t empacota(char MI, char enderecoDestino, char enderecoOrigem, char tamanho, char sequencia, char tipo, char dados[15]);

void  enviarACKParaCliente(int soquete, struct sockaddr_ll endereco, int sequencializacao);
void enviarNACKParaCliente(int soquete, struct sockaddr_ll endereco, int sequencializacao);
void enviarErroParaCLiente(int soquete, struct sockaddr_ll endereco, int sequencializacao, int erro);


void enviarACKParaServidor(int soquete, struct sockaddr_ll endereco, int sequencializacao);
void enviarNACKParaServidor(int soquete, struct sockaddr_ll endereco, int sequencializacao);


int tamanhoString(char *string);
void aumentaSequencia(int *sequencia);
int validarSequencializacao(pacote_t pacote, int sequencializacao);

int validarLeituraCliente(pacote_t pacote);
int validarLeituraServidor(pacote_t pacote);

int getIntDados(pacote_t pacote, int deslocamento);



#endif
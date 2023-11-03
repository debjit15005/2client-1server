#ifndef PACKETDEF_H
#define PACKETDEF_H


#define BUFFERLENGTH 256
#define TIMEOUT 2000
#define PDR 10

typedef struct pkt{
    int type;
    int size;
    int seq_no;
    int client;
    char payload[BUFFERLENGTH];
    int last;
} packet;




#endif
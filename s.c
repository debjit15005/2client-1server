#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h> // for open
#include<unistd.h>
#include<math.h>
#include<time.h>
#include "packetDef.h"

// void die(char* s){
//     die(s);
//     exit(1);
// }



int main(){
    srand(time(0));
    packet data,ack1,ack2;
    data.type = 0;
    ack1.type = 1;
    ack2.type = 1;
    int state = 0;
    struct sockaddr_in si_other;
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(5050);
    si_other.sin_addr.s_addr = inet_addr("127.0.0.1");
    int sock;
    int client1 = 0;
    int client2 = 0;
    int recvlen = 0;
    int lastseq1 = 0;
    int lastseq2 = 0;
    FILE* fp = fopen("list.txt","w+");
    if(fp == NULL){
        printf("Error in opening file");
        return 1;
    }
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Error in socket creation");
        return 1;
    }
    if(bind(sock, (struct sockaddr*)&si_other,sizeof(si_other)) == -1){
        printf("Error in binding");
        return 1;
    }
    printf("Started Listening\n");
    if(listen(sock, 10) == -1){
        printf("Failed to listen\n");
        return -1; 
    }
    client1 = accept(sock,(struct sockaddr*)NULL ,NULL);
    int flag = 0;
    int num = 0;
    while(1){
        num = rand()%100;
        // printf("Number is %d\n",num);
        switch(state){
            case 0:
            if((recvlen = recv(client1, &data, sizeof(data), 0)) == -1){
                    printf("Error while receiveing data\n");
                    return 1;
            }
            // printf("Data recieved\n");
            if(data.last == 1)
                return 1;
            if(num<=PDR){
                printf("DROP PACKET: Seq. No.: %d\n",data.seq_no);
                state = 0;
                break;
            }
            if(data.seq_no != lastseq1){
                state = 0;
                if(send(client1, &ack1, sizeof(ack1), 0) == -1){
                        printf("Couldnt send data\n");
                        return 1;
                }
                break;
            }
            printf("RCVD packet: Seq. No. = %d, Size = %d from Client = %d\n",data.seq_no,data.size,data.client);
            fprintf(fp,"%s,",data.payload);
            fflush(fp);
            ack1.seq_no = data.seq_no;
            ack1.client = 1;
            if(send(client1, &ack1, sizeof(ack1), 0) == -1){
                    printf("Couldnt send ack\n");
                    return 1;
            }
            printf("SENT ACK: Seq. No.: %d to Client = %d\n",ack1.seq_no,ack1.client);
            state = 1;
            lastseq1 += data.size;
            break;

            case 1:

            if(flag == 0){
                client2 = accept(sock,(struct sockaddr*)NULL ,NULL);
                flag = 1;
            }
            if((recvlen = recv(client2, &data, sizeof(data), 0)) == -1){
                    printf("Error while receiveing data\n");
                    return 1;
            }
            if(num<=PDR){
                printf("DROP PACKET: Seq. No.: %d\n",lastseq2);
                state = 1;
                break;
            }
            if(data.seq_no != lastseq2){
                state = 1;
                if(send(client2, &ack2, sizeof(ack2), 0) == -1){
                        printf("Couldnt send data\n");
                        return 1;
                }
                break;
            }
            printf("RCVD packet: Seq. No. = %d, Size = %d from Client = %d\n",data.seq_no,data.size,data.client);
            fprintf(fp,"%s, ",data.payload);
            fflush(fp);
            ack2.seq_no = data.seq_no;
            ack2.client = 2;
            if(send(client2, &ack2, sizeof(ack2), 0) == -1){
                        printf("Couldnt send data\n");
                        return 1;
            }
            printf("SENT ACK: Seq. No.: %d to Client = %d\n",ack2.seq_no,ack2.client);
            state = 0;
            lastseq2 += data.size;
            break;
        }

    }
    fclose(fp);
    close(sock);
}
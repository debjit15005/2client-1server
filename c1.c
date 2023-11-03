#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h> 
#include<unistd.h>
#include<math.h>
#include<sys/poll.h>
#include<time.h>
#include "packetDef.h"




int main(){
    srand(time(0));
    struct sockaddr_in serv_addr;
    int sockfd;
    packet data, ack;

    data.type = 0;
    ack.type = 1;
    int state = 0;
    int msize = 0;
    int prevEnd = 0;
    int ret = 0;
    int last_seq = 0;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5050); 
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int slen = sizeof(serv_addr);
    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0) {
        printf("\n Error : Couldn't create socket \n");
        return 1; 
    }
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0) {
        printf("\n Error : Connection Failed \n");
        return 1; 
    }
    char buf[1000];
    FILE* fp = fopen("name.txt","rb");
    if(fp == NULL){
        printf("Error in opening file\n");
        return 1;
    }
    struct pollfd pfds;
    pfds.fd = sockfd;
    pfds.events = POLLIN;
    int poll_;
    int num = 0;
    while(fgets(buf,1000,fp)!=NULL){
        char* token;
        
        char delim[] = ",.";
        token = strtok(buf,delim);
        int state = 0;
        while(token!=NULL){
        num = rand()%100;
        switch(state)
            {
                case 0:
                //need to send packet
                if(ret == 1){
                    ret = 0;
                    printf("Retransmit packet: Seq No. = %d, Size = %d BYTES\n",data.seq_no,data.size);
                    if(send(sockfd, &data, sizeof(data), 0) == -1){
                        printf("Couldn't send data\n");
                        return 1;
                    }
                    state = 1;
                }
                else{
                    strcpy(data.payload,token);
                    msize = strlen(data.payload);
                    data.seq_no = prevEnd;
                    data.size = msize;
                    data.client = 1;
                    data.last = 0;
                    if(send(sockfd, &data, sizeof(data), 0) == -1){
                        printf("Couldnt send data\n");
                        return 1;
                    }
                    state = 1;
                    printf("Sent packet: Seq No. = %d, Size = %d BYTES\n",data.seq_no,data.size);
                }
                break;

                case 1:
                //wait for ack
                poll_ = poll(&pfds, 1, TIMEOUT);
                if(poll_ > 0){
                    if(recv(sockfd, &ack, sizeof(ack), 0)==-1){
                        return 1;
                    }
                    if(num<10){
                        printf("DROP ACK: Seq. No. = %d\n",ack.seq_no);
                        ret = 1;
                        state = 0;
                        break;
                    }
                    printf("Recieved ACK: Seq No. = %d\n",ack.seq_no);
                    if(ack.seq_no==prevEnd){
                        ret = 0;
                        prevEnd = prevEnd+data.size;
                        token = strtok(NULL,delim);
                    }
                    else{
                        ret = 1;
                    }
                    state = 0;
                    break;
                }
                else if(poll_ == 0){
                    ret = 1;
                    state = 0;
                    break;
                }
                // if(recv(sockfd, &ack, sizeof(ack), 0)==-1){
                //         printf("Error while reciveing ack\n");
                //         return 1;
                //     }
                //     printf("RCVD ACK: Seq No. = %d\n",data.seq_no);
                //     
                // break;
            }
        }
        data.last = 1;
        if(send(sockfd, &data, sizeof(data), 0) == -1){
                        printf("Couldnt send data\n");
                        return 1;
        }
    }
    fclose(fp);
    close(sockfd);
    return 0;
}

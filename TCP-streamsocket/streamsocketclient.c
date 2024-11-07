#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>


#define PORT "3490"
#define MAXDATASIZE 100     // max number of bytes we can get at once

void *get_in_addr(struct sockaddr *sa){
    if(sa->sa_family==AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);

}

int main(int argc, char *argv[]){
    int sockfd, new_fd;
    char buff[MAXDATASIZE];
    struct addrinfo hints, *res, *p;
    char s[INET6_ADDRSTRLEN];
    int numbytes;
    int rv;

    if(argc!=2){
        fprintf(stderr,"Usage error, usage ->\n./streamsocketclient hostname\n");
        exit(1);
    }

    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(rv=getaddrinfo(argv[1],PORT,&hints,&res)!=0){
        perror("getaddrinfo");
        exit(1);
    }

    for(p=res;p!=NULL;p=p->ai_next){
        if((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
            perror("sockfd");
            continue;
        }
        if((connect(sockfd,p->ai_addr,p->ai_addrlen))==-1){
            close(sockfd);
            perror("connect");
            continue;
        }
        break;
    }
    if(p==NULL){
        fprintf(stderr, "client failed to connect\n");
        exit(1);
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr),s,sizeof(s));
    printf("client : connecting to server at %s\n",s);

    freeaddrinfo(res);

    if((numbytes=recv(sockfd, buff, MAXDATASIZE-1,0))==-1){
        perror("recv");
        exit(1);
    }
    buff[numbytes]='\0';
    printf("client : Recieved message : %s\n", buff);
    close(sockfd);

    return 0;
}
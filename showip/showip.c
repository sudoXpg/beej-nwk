#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include <sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>

int main(int argc, char *argv[]){
    
    int status;
    struct addrinfo hints,*res, *p;
    char ipstr[INET6_ADDRSTRLEN];           // max possible ip strlen

    if(argc!=2){
        fprintf(stderr,"usage : showip hostname\n");
        return 1;
    }

    memset(&hints,0,sizeof(hints));     // set memory as zero
    hints.ai_family = AF_UNSPEC;        // so it can be ipv4/v6
    hints.ai_socktype=SOCK_STREAM;      // TCP

    if((status=getaddrinfo(argv[1],NULL,&hints,&res))!=0){              // get info and store in res
        fprintf(stderr,"getaddrinfo error %s\n",gai_strerror(status));
        return 2;
    }
    printf("IP address for %s : \n\n",argv[1]);
    for(p=res;p!=NULL;p=p->ai_next){
        void *addr;
        char *ipver;

        if(p->ai_family==AF_INET){                                      // IPv4 
            struct sockaddr_in *ipv4=(struct sockaddr_in *)p->ai_addr;
            addr=&(ipv4->sin_addr);
            ipver="IPv4";
        }
        
        if(p->ai_family==AF_INET6){                                      // IPv6
            struct sockaddr_in6 *ipv6=(struct sockaddr_in6 *)p->ai_addr;
            addr=&(ipv6->sin6_addr);
            ipver="IPv6";
        }

        // convert to printable text
        inet_ntop(p->ai_family, addr,ipstr,sizeof(ipstr));
        printf(" %s : %s\n",ipver,ipstr);
    }

    


    freeaddrinfo(res);
    return 0;
}
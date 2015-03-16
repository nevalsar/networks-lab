/*
client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>

#define FIS_PORT "32000"
#define PEER_PORT "31000"  // the port the client will be connecting to

#define MAXDATASIZE 100  // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6
void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int connect_to_host(struct addrinfo *servinfo, int *sockfd) {

    struct addrinfo *p;
    char s[INET6_ADDRSTRLEN];

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr),
        s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);  // all done with this structure
}

void get_host_info(struct addrinfo **servinfo, std::string serverip){
    struct addrinfo hints;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(serverip.c_str(), PEER_PORT, &hints, servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
}

void receive_reply(std::string fis_ip) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXDATASIZE];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(fis_ip.c_str(), PEER_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(6);
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(5);
    }

    freeaddrinfo(servinfo);

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    close(sockfd);

    buf[numbytes] = '\0';
    printf("Received reply : %s\n", buf);
}

void send_query(std::string fis_ip, std::string query){

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(fis_ip.c_str(), FIS_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(5);
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        exit(5);
    }

    if ((numbytes = sendto(sockfd, query.c_str(), strlen(query.c_str()), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo);

    close(sockfd);

}

int main(int argc, char const *argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo *servinfo;
    std::string fis_ip;

    if (argc <2){
        printf("Usage : Peer_Client FIS_Server_IP\n");
        exit(6);
    } else {
        fis_ip = argv[1];
    }

    while (1) {
        int choice;
        printf("\n-- Peer Client --\n");
        printf("Enter 1 to query FIS Server\n");
        fflush(stdout);
        printf("Enter 2 to Download from peer\n");
        fflush(stdout);
        fscanf(stdin, "%3d", &choice);

        if (choice == 1){
            char query[100] = "0";
            printf("Enter filename to query peer IP: ");
            fflush(stdout);
            fscanf(stdin, "%100s", query+1);
            send_query(fis_ip, query);
            receive_reply(fis_ip);

        } else if (choice == 2) {
            char filename[50];
            char serverip[75];
            printf("Enter peer IP: ");
            fflush(stdout);
            fscanf(stdin, "%75s", serverip);
            printf("Enter filename: ");
            fflush(stdout);
            fscanf(stdin, "%50s", filename);


              if (filename[strlen(filename)-1] == '\n')
                filename[strlen(filename)-1] = '\0';
              if (filename[strlen(filename)-1] == '\r')
                filename[strlen(filename)-1] = '\0';

            if (serverip[strlen(serverip)-1] == '\n')
                serverip[strlen(serverip)-1] = '\0';
              if (serverip[strlen(serverip)-1] == '\r')
                serverip[strlen(serverip)-1] = '\0';

              get_host_info(&servinfo, serverip);
              connect_to_host(servinfo, &sockfd);

              if (send(sockfd, filename, strlen(filename), 0) == -1)
                  perror("send");

              char recvBuff[MAXDATASIZE];
              FILE *outputFile = fopen(filename, "wb");

              if (outputFile == NULL) {
                fprintf(stderr, "Cannot open output file");
                return 1;
              }

              int bytesReceived = recv(sockfd, recvBuff, MAXDATASIZE-1, 0);
              // write(1, recvBuff, bytesReceived);
              fwrite(recvBuff, 1, bytesReceived, outputFile);

              while ((bytesReceived = recv(sockfd, recvBuff, MAXDATASIZE-1, 0)) > 0) {
                  // you should add error checking here
                  // write(1, recvBuff, bytesReceived);
                  fwrite(recvBuff, 1, bytesReceived, outputFile);
              }

              fclose(outputFile);
              close(sockfd);
        } else {
            printf("Sorry, invalid choice\n");
            continue;
        }


    }

    return 0;
}

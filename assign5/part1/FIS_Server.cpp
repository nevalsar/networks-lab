/*
** FIS_Server.c -- a datagram sockets "server" demo
*/

#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define FIS_PORT "32000"    // the port users will be connecting to
#define PEER_PORT "31000"    // the port to reply to peer

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void send_reply(std::string ipstring, std::string reply){

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(ipstring.c_str(), PEER_PORT, &hints, &servinfo)) != 0) {
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

    if ((numbytes = sendto(sockfd, reply.c_str(), strlen(reply.c_str()), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("Peer_Server: sent reply \'%s\' to Peer_Client at %s\n", reply.c_str(), ipstring.c_str());
    close(sockfd);

}

int main(void) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;

    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;  // use my IP

    if ((rv = getaddrinfo(NULL, FIS_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("FIS_Server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("FIS_Server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "FIS_Server: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("FIS_Server is up.\nWaiting for incoming connections...\n");

    std::map <std::string, std::string> file_list;

    while (1) {
        char buf[MAXBUFLEN];
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        buf[numbytes] = '\0';

        std::string ipstring = inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr),
                s, sizeof s);

        printf("FIS_Server: got packet from %s\n\n", ipstring.c_str());

        if (buf[0] =='1') {
            // Receiving file list from Peer_Server
            char* file;
            file=strtok(buf+1, "\n");
            while (file != NULL) {
                if (file) {
                    file_list[file] = ipstring;
                    printf("Add : File \'%s\' from %s\n", file, ipstring.c_str());
                }
                file=strtok(NULL, "\n");
            }


        } else if (buf[0] == '0') {
            // IP Query received

            printf("Queried file : %s\n", buf+1);
            if (file_list.find(buf+1) != file_list.end()) {
                printf("Server IP = %s\n", file_list[buf+1].c_str());
                send_reply(ipstring, file_list[buf+1].c_str());
            } else {
                printf("File not found!\n");
                send_reply(ipstring, "Not found!");
            }
        }
        printf("\n");

    }

    close(sockfd);

    return 0;
}

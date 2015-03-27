/*
** FIS_Server.cpp
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
#include "Socket.cpp"

#define FIS_PORT "11000"  // FIS Server listening port
#define PEER_PORT "12000"  // the port to communicate to peer

#define MAXDATASIZE 100

void send_reply(std::string ipstring, std::string reply) {
    int sockfd, rv, numbytes;
    Socket new_connection;

    new_connection.ifUDP = true;
    new_connection.ifRemote = true;
    new_connection.ip.assign(ipstring.c_str());
    new_connection.port.assign(PEER_PORT);
    new_connection.getaddressinformation();
    sockfd = new_connection.getsocket(false, false);

    if ((numbytes = sendto(sockfd, reply.c_str(), strlen(reply.c_str()), 0,
             (new_connection.ptr_address)->ai_addr, (new_connection.ptr_address)->ai_addrlen)) == -1) {
        perror("FIS_Server UDP - sendto");
        exit(1);
    }
    new_connection.cleardata();

    printf("Peer_Server UDP : sent reply \'%s\' to Peer_Client at %s\n", reply.c_str(), ipstring.c_str());
    close(sockfd);
}

int main(void) {
    int sockfd;
    socklen_t addr_len;
    int numbytes;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr;

    // bind socket listening on my IP
    Socket new_connection;
    new_connection.ifUDP = true;
    new_connection.ifRemote = false;
    new_connection.port.assign(FIS_PORT);
    new_connection.getaddressinformation();
    sockfd = new_connection.getsocket(true, false);
    new_connection.cleardata();

    // filename, IP map
    std::map <std::string, std::string> file_list;

    while (1) {
        char buf[MAXDATASIZE];
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';

        std::string ipstring = inet_ntop(their_addr.ss_family,
                &(((struct sockaddr_in*)&their_addr)->sin_addr),
                s, sizeof s);
        printf("FIS_Server: got packet from %s\n\n", ipstring.c_str());

        if (buf[0] =='1') {
            // Receiving file list from Peer_Server
            char* file;
            file = strtok(buf + 1, "\n");
            while (file != NULL) {
                if (file) {
                    file_list[file] = ipstring;
                    printf("Add : File \'%s\' from %s\n", file, ipstring.c_str());
                }
                file = strtok(NULL, "\n");
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

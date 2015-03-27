#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <bits/stdc++.h>

class Socket{
    int returnval;
    int sockfd;
    int numbytes;
    char s[INET_ADDRSTRLEN];
    struct sockaddr;
    struct addrinfo hints, *servinfo;

 public:
    std::string ip;
    std::string port;
    bool ifUDP;
    bool ifRemote;
    struct addrinfo *ptr_address;

    int getaddressinformation();
    int getsocket(bool, bool);
    void cleardata();
};

int Socket::getaddressinformation() {
    // select socket type according to flag
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = (ifUDP)? SOCK_DGRAM: SOCK_STREAM;
    // use own IP for socket if none provided
    if (!ifRemote) {
        hints.ai_flags = AI_PASSIVE;
        returnval = getaddrinfo(NULL, port.c_str(), &hints, &servinfo);
    } else {
        returnval = getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo);
    }

    if (returnval != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(returnval));
        exit(1);
    }
}

int Socket::getsocket(bool ifBind, bool ifConnect) {
    int yes = 1;
    for (ptr_address = servinfo; ptr_address != NULL; ptr_address = ptr_address->ai_next) {
        if ((returnval = sockfd = socket(ptr_address->ai_family, ptr_address->ai_socktype,
                ptr_address->ai_protocol)) == -1) {
            perror("Socket");
            continue;
        }

        if (ifBind) {
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
                perror("setsockopt");
                exit(2);
            }

            if ((returnval = bind(sockfd, ptr_address->ai_addr, ptr_address->ai_addrlen)) == -1) {
                close(sockfd);
                perror("bind");
                continue;
            }
        }

        if (ifConnect) {
            if (connect(sockfd, ptr_address->ai_addr, ptr_address->ai_addrlen) == -1) {
                close(sockfd);
                perror("connect");
                continue;
            }
        }

        if (ifRemote) {
            inet_ntop(ptr_address->ai_family, &(((struct sockaddr_in *)ptr_address->ai_addr)->sin_addr),
                s, sizeof s);
            printf("Connecting to remote: %s\n", s);
        }

        break;
    }

    if (ptr_address == NULL) {
        fprintf(stderr, "Socket::getsocket() failed\n");
        exit(3);
    }
    return sockfd;
}

void Socket::cleardata() {
    ip.clear();
    port.clear();
    freeaddrinfo(servinfo);
};

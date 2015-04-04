/**********************************
Assignment 6
CS39006 Computer Networks Lab
Submitted by:
    NEVIN VALSARAJ 12CS10032
    PRANJAL PANDEY 12CS30026
**********************************/

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <bits/stdc++.h>

#ifndef SOCKET_H
#define SOCKET_H

#define MAXDATASIZE 500

void sigchild_handler(int s) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

class Socket{
    int returnval;
    int numbytes;
    char s[INET_ADDRSTRLEN];
    struct sockaddr;
    struct addrinfo hints, *servinfo;

 public:
    int sockfd;
    std::string ip;
    std::string port;
    bool ifUDP;
    bool ifRemote;
    struct addrinfo *ptr_address;

    int getaddressinformation();
    void makesocket(bool, bool);
    void cleardata();
    std::string get_ip();
};

int Socket::getaddressinformation() {
    // select socket type according to flag
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_protocol = 0;
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

void Socket::makesocket(bool ifBind, bool ifConnect) {
    int yes = 1;
    for (ptr_address = servinfo; ptr_address != NULL;
            ptr_address = ptr_address->ai_next) {
        if ((returnval = sockfd = socket(ptr_address->ai_family,
                ptr_address->ai_socktype, ptr_address->ai_protocol)) == -1) {
            perror("Socket");
            continue;
        }

        if (ifBind) {
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
                perror("setsockopt");
                exit(2);
            }

            if ((returnval = bind(sockfd, ptr_address->ai_addr,
                    ptr_address->ai_addrlen)) == -1) {
                close(sockfd);
                perror("bind");
                continue;
            }
        }

        if (ifConnect) {
            if (connect(sockfd, ptr_address->ai_addr,
                    ptr_address->ai_addrlen) == -1) {
                close(sockfd);
                perror("connect");
                continue;
            }
        }

        if (ifRemote) {
            inet_ntop(ptr_address->ai_family,
                &(((struct sockaddr_in *)ptr_address->ai_addr)->sin_addr),
                s, sizeof s);
            printf("Connecting to remote: %s\n", s);
        }

        break;
    }

    if (ptr_address == NULL) {
        fprintf(stderr, "Socket::getsocket() failed\n");
        exit(3);
    }
}

void Socket::cleardata() {
    ip.clear();
    port.clear();
    freeaddrinfo(servinfo);
}

std::string Socket::get_ip() {
    char ip4[INET_ADDRSTRLEN];
    std::string ip;
    struct sockaddr_in *temp = (sockaddr_in*)(
            ptr_address->ai_addr);
    inet_ntop(AF_INET, &(temp->sin_addr), ip4, INET_ADDRSTRLEN);
    ip.assign(ip4);
    return ip;
}

void send_udp_string(std::string ipstring, std::string portno, std::string reply) {
    int sockfd, numbytes;
    Socket new_connection;
    new_connection.ifUDP = true;
    new_connection.ifRemote = true;
    new_connection.ip.assign(ipstring.c_str());
    new_connection.port.assign(portno.c_str());
    new_connection.getaddressinformation();
    new_connection.makesocket(false, false);

    if ((numbytes = sendto(sockfd, reply.c_str(), strlen(reply.c_str()), 0,
             (new_connection.ptr_address)->ai_addr, (new_connection.ptr_address)->ai_addrlen)) == -1) {
        perror("FIS_Server UDP - sendto");
        exit(1);
    }

    new_connection.cleardata();
    close(sockfd);
}

std::string receive_udp_string(std::string ipstring, std::string portno) {
    Socket new_connection;
    int sockfd, numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXDATASIZE];
    char s[INET6_ADDRSTRLEN];
    std::string reply;

    std::cout<<"Waiting at " <<ipstring.c_str() <<":" <<portno.c_str()<<std::endl;

    new_connection.ifUDP = true;
    new_connection.ifRemote = true;
    new_connection.ip.assign(ipstring.c_str());
    new_connection.port.assign(portno.c_str());
    new_connection.getaddressinformation();
    new_connection.makesocket(true, false);
    new_connection.cleardata();

    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
        s, sizeof s);

    close(sockfd);
    buf[numbytes] = '\0';
    reply.assign(s);
    reply.push_back('\n');
    reply.assign(portno);
    reply.push_back('\n');
    reply.append(buf);
    std::cout <<"Received UDP string" <<std::endl;
    return reply;
}

#endif

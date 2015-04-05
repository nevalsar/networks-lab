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
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <bits/stdc++.h>

#include <string>

#ifndef SOCKET_H
#define SOCKET_H

#define MAXDATASIZE 500

using std::string;
using std::cout;
using std::endl;

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
    struct addrinfo hints, *servinfo;
    int sockfd;
    struct addrinfo *ptr_address;

    void getaddressinformation(string, string, bool, bool);
    void cleardata();
 public:
    string get_own_ip();
    void send_udp_string(string, string, string);
    void send_tcp_string(int, string);
    string receive_udp_string(string);
    string receive_tcp_string(int);
    int getsocket(string, string, bool, bool, bool, bool);
    void send_tcp_file(int, string);
    void receive_tcp_file(int, string);
};

void Socket::getaddressinformation(string ip, string port, bool isUDP,
        bool isRemote) {
    // select socket type according to flag
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_protocol = 0;
    hints.ai_socktype = (isUDP)? SOCK_DGRAM: SOCK_STREAM;
    // use own IP for socket if none provided
    if (!isRemote) {
        hints.ai_flags = AI_PASSIVE;
        returnval = getaddrinfo(NULL, port.c_str(), &hints, &servinfo);
    } else {
        returnval = getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo);
    }

    if (returnval != 0) {
        fprintf(stderr, "socket - getaddressinformation - getaddrinfo: %s\n",
            gai_strerror(returnval));
        exit(1);
    }
}

int Socket::getsocket(string ip, string port, bool isUDP, bool isRemote,
        bool ifBind, bool ifConnect) {
    getaddressinformation(ip, port, isUDP, isRemote);
    int yes = 1;
    for (ptr_address = servinfo; ptr_address != NULL;
            ptr_address = ptr_address->ai_next) {
        if ((returnval = sockfd = socket(ptr_address->ai_family,
                ptr_address->ai_socktype, ptr_address->ai_protocol)) == -1) {
            perror("socket - getsocket - Socket");
            continue;
        }

        if (ifBind) {
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
                perror("socket - getsocket - setsockopt");
                exit(2);
            }

            if ((returnval = bind(sockfd, ptr_address->ai_addr,
                    ptr_address->ai_addrlen)) == -1) {
                close(sockfd);
                perror("socket - getsocket - bind");
                continue;
            }
        }

        if (ifConnect) {
            if (connect(sockfd, ptr_address->ai_addr,
                    ptr_address->ai_addrlen) == -1) {
                close(sockfd);
                perror("socket - getsocket - connect");
                continue;
            }
        }

        if (isRemote) {
            inet_ntop(ptr_address->ai_family,
                &(((struct sockaddr_in *)ptr_address->ai_addr)->sin_addr),
                s, sizeof s);
            printf("socket - getsocket - Connecting to remote: %s\n", s);
        }

        break;
    }

    if (ptr_address == NULL) {
        fprintf(stderr, "socket - getsocket failed\n");
        exit(3);
    }
    cleardata();
    return sockfd;
}

void Socket::cleardata() {
    freeaddrinfo(servinfo);
}

std::string Socket::get_own_ip() {
    char ip4[INET_ADDRSTRLEN];
    int sockfd;
    void *addr;
    char *ipver;
    getaddressinformation("", "15000", true, false);
    for (ptr_address = servinfo; ptr_address != NULL;
            ptr_address = ptr_address->ai_next) {
        if (ptr_address->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 =
                (struct sockaddr_in*)ptr_address->ai_addr;
            addr = &(ipv4->sin_addr);
            break;
        }
    }
    // convert ip to string and return
    inet_ntop(AF_INET, addr, ip4, INET_ADDRSTRLEN);
    cleardata();
    return string(ip4);
}

void Socket::send_udp_string(std::string ip, std::string port,
        std::string reply) {
    int sockfd, numbytes;
    sockfd = getsocket(ip, port, true, true, false, false);
    if ((numbytes = sendto(sockfd, reply.c_str(), strlen(reply.c_str()), 0,
             (ptr_address)->ai_addr, (ptr_address)->ai_addrlen)) == -1) {
        perror("socket - send_udp_string");
        exit(1);
    }
    close(sockfd);
}

void Socket::send_tcp_string(int tcp_socket, std::string reply) {
    if (send(sockfd, reply.c_str(), strlen(reply.c_str()), 0) == -1) {
        perror("socket - send_tcp_string");
        exit(1);
    }
    close(sockfd);
}

std::string Socket::receive_udp_string(std::string port) {
    int sockfd, numbytes;
    sockfd = getsocket("", port, true, false, true, false);
    // Socket new_connection;
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXDATASIZE];
    char ipv4addr[INET_ADDRSTRLEN];
    std::string reply;

    std::cout <<"Waiting at port " <<port.c_str() <<std::endl;

    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("socket - receive_udp_string - recvfrom");
        exit(1);
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
        ipv4addr, sizeof ipv4addr);

    close(sockfd);
    buf[numbytes] = '\0';
    reply.assign(port + '\n' + buf);
    std::cout <<"socket - receive_udp_string - Received UDP string : " <<reply
        <<std::endl;
    return reply;
}

std::string Socket::receive_tcp_string(int tcp_socket) {
    int rc;
    char reply[MAXDATASIZE];
    rc = recv(tcp_socket, reply, MAXDATASIZE, 0);
    if (rc == -1) {
        fprintf(stderr, "socket - receive_tcp_string - recv failed: %s\n",
            strerror(errno));
        exit(1);
    }

    /* null terminate and strip any \r and \n from filename */
    reply[rc] = '\0';
    int size = strlen(reply);
    if (reply[size-1] == '\n' || reply[size-1] == '\r')
        reply[size-1] = '\0';
    return string(reply);
}

void Socket::send_tcp_file(int tcp_socket, string filename) {
    int file_desc, rc;
    struct stat stat_buf;
    off_t offset = 0;
    file_desc = open(filename.c_str(), O_RDONLY);
    if (file_desc == -1) {
        fprintf(stderr, "socket - send_tcp_file - unable to open '%s': %s\n",
            filename.c_str(), strerror(errno));
        exit(1);
    }

    /* get the size of the file to be sent */
    fstat(file_desc, &stat_buf);

    /* copy file using sendfile */
    offset = 0;
    rc = sendfile(tcp_socket, file_desc, &offset, stat_buf.st_size);
    if (rc == -1) {
        fprintf(stderr, "socket - send_tcp_file -error from sendfile: %s\n",
            strerror(errno));
        exit(1);
    }
    if (rc != stat_buf.st_size) {
        fprintf(stderr,"socket - send_tcp_file - incomplete transfer from \
            sendfile: %d of %d bytes\n", rc, (int)stat_buf.st_size);
        exit(1);
    }
    close(file_desc);
}

void Socket::receive_tcp_file(int tcp_socket, string filename) {
    int count, bytesReceived;
    char recvBuff[MAXDATASIZE];

    FILE *outputFile = fopen(filename.c_str(), "wb");
    if (outputFile == NULL) {
        fprintf(stderr, "Cannot open output file");
        exit(1);
    }

    count = 0;
    bytesReceived = recv(sockfd, recvBuff, MAXDATASIZE-1, 0);
    fwrite(recvBuff, 1, bytesReceived, outputFile);
    while ((bytesReceived = recv(sockfd, recvBuff, MAXDATASIZE-1, 0)) > 0) {
        if (count < 20) {
            printf(".");
            count++;
        } else {
            printf("\b \b");
            count++;
        }
        if (count == 40)
            count = 0;
        fwrite(recvBuff, 1, bytesReceived, outputFile);
    }

    printf("\nDone downloading file !\n");
    fclose(outputFile);
}

#endif

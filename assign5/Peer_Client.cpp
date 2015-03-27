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
#include "Socket.cpp"

#define FIS_PORT "11000"  // FIS Server listening port
#define PEER_PORT "12000"  // the port to communicate to peer

#define MAXDATASIZE 100  // max number of bytes we can get at once

void receive_reply(char* reply) {
    Socket new_connection;
    int sockfd, numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXDATASIZE];

    new_connection.ifUDP = true;
    new_connection.ifRemote = false;
    new_connection.port.assign(PEER_PORT);
    new_connection.getaddressinformation();
    sockfd = new_connection.getsocket(true, false);
    new_connection.cleardata();

    if ((numbytes = recvfrom(sockfd, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    close(sockfd);
    buf[numbytes] = '\0';
    strcpy(reply, buf);
}

void send_query(std::string fis_ip, std::string query) {
    Socket new_connection;
    new_connection.ifUDP = true;
    new_connection.ifRemote = true;
    new_connection.ip.assign(fis_ip.c_str());
    new_connection.port.assign(FIS_PORT);
    new_connection.getaddressinformation();
    int sockfd = new_connection.getsocket(false, false);

    if (sendto(sockfd, query.c_str(), strlen(query.c_str()), 0,
             (new_connection.ptr_address)->ai_addr, (new_connection.ptr_address)->ai_addrlen) == -1) {
        perror("sendto");
        exit(1);
    }

    new_connection.cleardata();
    close(sockfd);
}

int main(int argc, char const *argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo *servinfo;
    std::string fis_ip;

    if (argc <2) {
        printf("Usage : Peer_Client FIS_Server_IP\n");
        exit(6);
    } else {
        fis_ip.assign(argv[1]);
    }

    while (1) {
        printf("\n-- Peer Client --\n");
        char query[100] = "0";
        char reply[100] = "0";
        printf("Enter filename to query peer IP: ");
        fflush(stdout);
        fscanf(stdin, "%100s", query+1);
        send_query(fis_ip, query);
        receive_reply(reply);
        printf("Received reply : %s\n", reply);


        if (strstr(reply, "Not") != NULL)
            continue;

        int choice;
        printf("Enter 1 to download this file\n");
        fflush(stdout);
        printf("Enter 2 to enter another query\n");
        fflush(stdout);
        fscanf(stdin, "%3d", &choice);

        if (choice == 2) {
            continue;

        } else if (choice == 1) {
            char filename[50];
            char serverip[75];

            strcpy(serverip, reply);
            strcpy(filename, query+1);

            if (filename[strlen(filename)-1] == '\n')
                filename[strlen(filename)-1] = '\0';
            if (filename[strlen(filename)-1] == '\r')
                filename[strlen(filename)-1] = '\0';

            if (serverip[strlen(serverip)-1] == '\n')
                serverip[strlen(serverip)-1] = '\0';
            if (serverip[strlen(serverip)-1] == '\r')
                serverip[strlen(serverip)-1] = '\0';

            char recvBuff[MAXDATASIZE];
            FILE *outputFile = fopen(filename, "wb");

            if (outputFile == NULL) {
                fprintf(stderr, "Cannot open output file");
                return 1;
            }

            Socket new_connection;
            new_connection.ifUDP = false;
            new_connection.ifRemote = true;
            new_connection.ip.assign(serverip);
            new_connection.port.assign(PEER_PORT);
            new_connection.getaddressinformation();
            sockfd = new_connection.getsocket(false, true);
            new_connection.cleardata();

            if (send(sockfd, filename, strlen(filename), 0) == -1)
                perror("send");

            printf("Downloading %s from %s :\n", filename, serverip);

            char temp = '.';
            int count = 0;

            int bytesReceived = recv(sockfd, recvBuff, MAXDATASIZE-1, 0);
            fwrite(recvBuff, 1, bytesReceived, outputFile);
            while ((bytesReceived = recv(sockfd, recvBuff, MAXDATASIZE-1, 0)) > 0) {
                if (count < 20) {
                    printf("%c", temp);
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
            close(sockfd);

        } else {
            printf("Sorry, invalid choice\n");
            continue;
        }
    }
    return 0;
}

// Peer_Server.cpp

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
#include <wait.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include "dirent.h"
#include <bits/stdc++.h>

#define FIS_PORT "11000"  // FIS Server listening port
#define PEER_PORT "12000"  // the port to communicate to peer

#define BACKLOG 10  // how many pending connections queue will hold

#define TRUE 1
#define FALSE 0
#define EQUAL(X,Y) (strcmp(X, Y)==0?1:0)

using std::string;
void error(const char* msg) {
    perror(msg);
    exit(1);
}

std::vector<std::string> getfiles(std::string directory) {

    std::vector<std::string> v;
    DIR* dp;
    struct dirent* ep;
    dp=opendir(directory.c_str());

    if(dp!=NULL)
    {
        while((ep=readdir(dp))){
            if(!EQUAL(ep->d_name,"..") && !EQUAL(ep->d_name,".")&&ep->d_name[0]!='.')
                v.push_back(ep->d_name );
        }
        closedir(dp);
    }
    else
        error("could not open Directory");
    return v;
}

void sigchild_handler(int s) {
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

// get sockaddr, IPv4 or IPv6
void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void connect_to_host(struct addrinfo *servinfo, struct addrinfo **p, int *sockfd) {

    char s[INET6_ADDRSTRLEN];
    int yes = 1;

    // loop through all the results and connect to the first we can
    for (*p = servinfo; *p != NULL; *p = (*p)->ai_next) {
        if ((*sockfd = socket((*p)->ai_family, (*p)->ai_socktype,
                (*p)->ai_protocol)) == -1) {
            perror("Peer_Server: socket");
            continue;
        }

        if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(*sockfd, (*p)->ai_addr, (*p)->ai_addrlen) == -1) {
            close(*sockfd);
            perror("Peer_Server: bind");
            continue;
        }

        break;
    }

    if (*p == NULL) {
        fprintf(stderr, "Peer_Server: failed to connect\n");
        exit(2);
    }

    inet_ntop((*p)->ai_family, get_in_addr((struct sockaddr*)(*p)->ai_addr),
        s, sizeof s);
    printf("Peer_Server: connecting to %s\n", s);


}

void send_files(std::string serverip) {

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(serverip.c_str(), FIS_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(7);
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
        exit(8);
    }

    std::vector<std::string> files =  getfiles("./");
    std::string files_buff = "1";
    for (int i = 0; i < files.size(); ++i) {
        files_buff = files_buff+files[i] + "\n";
    }

    if ((numbytes = sendto(sockfd, files_buff.c_str(), strlen(files_buff.c_str()), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

    freeaddrinfo(servinfo);

    printf("Peer server : Sent files list to %s\n", serverip.c_str());
    close(sockfd);
}

void get_host_info(struct addrinfo **servinfo, int type, std::string port, std::string serverip){
    struct addrinfo hints;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    if (type = 0) {
        hints.ai_socktype = SOCK_DGRAM;
        if ((rv = getaddrinfo(serverip.c_str(), port.c_str(), &hints, servinfo)) != 0) {
            fprintf(stderr, "Peer_Server: getaddrinfo: %s\n", gai_strerror(rv));
            exit(1);
        }
    } else {
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;  // use my IP
        if ((rv = getaddrinfo(NULL, port.c_str(), &hints, servinfo)) != 0) {
            fprintf(stderr, "Peer_Server: getaddrinfo: %s\n", gai_strerror(rv));
            exit(1);
        }
    }


    // if ((rv = getaddrinfo(NULL, port.c_str(), &hints, servinfo)) != 0) {
    //     fprintf(stderr, "Peer_Server: getaddrinfo: %s\n", gai_strerror(rv));
    //     exit(1);
    // }
}

int main( int argc, char* argv[]) {
    int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
    struct addrinfo *servinfo, *p;
    struct sockaddr_storage their_addr;  // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    std::string serverip;

    if(argc < 2) {
        printf("Usage: Peer_Server FIS_Server_IP\n");
        exit(5);
    } else {
        serverip = argv[1];
    }

    send_files(serverip);

    get_host_info(&servinfo, 1, PEER_PORT, serverip);
    connect_to_host(servinfo, &p, &sockfd);
    freeaddrinfo(servinfo);  // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("Peer_Server: listen");
        exit(1);
    }

    sa.sa_handler = sigchild_handler;  // reap all the dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Peer_Server: sigaction");
        exit(1);
    }

    printf("Peer_Server: Waiting for connections...\n");

    while (1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr*)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("Peer_Server: accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr*)&their_addr),
            s, sizeof s);
        printf("Peer_Server: got connection from %s\n", s);

        if (!fork()) {  // this is the child process
            close(sockfd);  // child doesn't need the listener

            char filename[50];
            int rc;
            int file_desc;
            struct stat stat_buf;  // argument to fstat
            off_t offset = 0;  // file offset

            /* get the file name from the client */
            rc = recv(new_fd, filename, sizeof(filename), 0);
            if (rc == -1) {
              fprintf(stderr, "recv failed: %s\n", strerror(errno));
              exit(1);
            }

            /* null terminate and strip any \r and \n from filename */
            filename[rc] = '\0';
            if (filename[strlen(filename)-1] == '\n')
              filename[strlen(filename)-1] = '\0';
            if (filename[strlen(filename)-1] == '\r')
              filename[strlen(filename)-1] = '\0';

            /* open the file to be sent */
            file_desc = open(filename, O_RDONLY);
            if (file_desc == -1) {
              fprintf(stderr, "unable to open '%s': %s\n", filename, strerror(errno));
              exit(1);
            }

            /* get the size of the file to be sent */
            fstat(file_desc, &stat_buf);

            /* copy file using sendfile */
            offset = 0;
            rc = sendfile(new_fd, file_desc, &offset, stat_buf.st_size);
            if (rc == -1) {
              fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
              exit(1);
            }
            if (rc != stat_buf.st_size) {
              fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
                      rc,
                      (int)stat_buf.st_size);
              exit(1);
            }

            /* close descriptor for file that was sent */
            close(file_desc);

            /* close socket descriptor */
            close(new_fd);
            exit(0);  // let child exit
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}

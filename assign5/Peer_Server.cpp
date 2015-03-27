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
#include "Socket.cpp"

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

void send_files_list(std::string serverip) {

    int sockfd, numbytes;
    Socket new_connection;
    new_connection.ifUDP = true;
    new_connection.ifRemote = true;
    new_connection.ip.assign(serverip.c_str());
    new_connection.port.assign(FIS_PORT);
    new_connection.getaddressinformation();
    sockfd = new_connection.getsocket(false, false);

    std::vector<std::string> files =  getfiles("./");
    std::string files_buff = "1";
    for (int i = 0; i < files.size(); ++i) {
        files_buff = files_buff+files[i] + "\n";
    }

    if ((numbytes = sendto(sockfd, files_buff.c_str(), strlen(files_buff.c_str()), 0,
             (new_connection.ptr_address)->ai_addr, (new_connection.ptr_address)->ai_addrlen)) == -1) {
        perror("sendto");
        exit(1);
    }
    new_connection.cleardata();

    printf("Peer server : Sent files list to %s\n", serverip.c_str());
    close(sockfd);
}

int main( int argc, char* argv[]) {
    int sockfd, new_fd;  // listen on sockfd, new connection on new_fd
    struct sockaddr_storage their_addr;  // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    char s[INET6_ADDRSTRLEN];
    std::string serverip;

    if(argc < 2) {
        printf("Usage: Peer_Server FIS_Server_IP\n");
        exit(5);
    } else {
        serverip.assign(argv[1]);
    }

    send_files_list(serverip);

    Socket new_connection;
    new_connection.ifUDP = false;
    new_connection.ifRemote = false;
    new_connection.ip.assign(serverip.c_str());
    new_connection.port.assign(PEER_PORT);
    new_connection.getaddressinformation();
    sockfd = new_connection.getsocket(true, false);
    new_connection.cleardata();

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

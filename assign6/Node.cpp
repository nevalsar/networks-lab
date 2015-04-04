/**********************************
Assignment 6
CS39006 Computer Networks Lab
Submitted by:
    NEVIN VALSARAJ 12CS10032
    PRANJAL PANDEY 12CS30026
**********************************/

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
#include <dirent.h>
#include <bits/stdc++.h>

#include <algorithm>
#include <string>
#include <vector>
#include <tuple>
#include <utility>

#include "Socket.cpp"

#ifndef NODE_H
#define NODE_H

#define EQUAL(X, Y) (strcmp(X, Y) == 0?1:0)
#define ROOT_PORT 11000
#define BACKLOG 10

using std::string;
using std::cout;
using std::endl;

void error(const char* msg) {
    perror(msg);
    exit(1);
}

std::vector<string> getfiles(string);

class Node{
    // root variables
    std::vector<std::size_t> nodes;
    std::vector<std::size_t> keys_all;

    // keys - hash, ip, port
    std::vector<std::tuple<std::size_t, string, string> > keys;
    // self details - id, ip, port
    std::size_t id;
    string ip;
    string port;
    int node_count;

    // files - hash, filename
    std::vector<std::pair<std::size_t, string> > files;

    // sockets for listening
    int listen_udp_sock;
    int listen_tcp_sock;
    // read set for sockets
    fd_set readfds;
    int maxfd;

    std::hash<string> hash_fn;

 public:
    Node* successor;
    Node(int);
    ~Node();
    void rootlistener();
    void nodelistener();
    Node* getpointer_to_node(std::size_t, Node*);
    void send_files_list(std::vector<string>);
    void setup_sockets(bool);
    void send_requested_file();
    void process_new_node();
    void add_key(std::tuple<std::size_t, string, string>);
    std::vector<std::tuple<std::size_t, string, string> > retrieve_keys();
    void stabilize();
};

Node::Node(int n) {
    node_count = n;
    Socket sockobj;
    // get own ip and port, generate own id / hash value
    ip.assign(sockobj.get_own_ip());
    port.assign(std::to_string(ROOT_PORT + node_count));
    string temp(ip + ":" + port);
    cout <<"Start node : " <<temp <<endl;
    id = hash_fn(temp);
    successor = NULL;

    // get own file list, populate files vector
    string filename;
    filename.assign("files/");
    filename.append(std::to_string(node_count));
    filename.push_back('/');
    auto files_strings =  getfiles(filename);
    for (int i = 0; i < files_strings.size(); i++) {
        size_t hashval = hash_fn(files_strings[i]);
        files.push_back(make_pair(hashval, files_strings[i]));
        if (node_count == 0)
            keys_all.push_back(hashval);
    }

    // set up listener sockets
    if (node_count == 0)
        setup_sockets(true);
    else
        setup_sockets(false);

    // if root node
    if (node_count == 0) {
        // initialize successor value, nodes vector
        successor = this;
        nodes.push_back(id);
        for (int i = 0; i < files_strings.size(); ++i) {
            cout <<"Add : " <<files_strings[i] <<endl;
        }
        // listen for incoming connections
        while (true)
            rootlistener();

    } else {
        // send files to root
        send_files_list(files_strings);
        cout <<"Listening for file requests.." <<endl;
        // listen for incoming connections
        while (true)
            nodelistener();
    }
}

void Node::nodelistener() {
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    fd_set tempreadfds;
    while (true) {
        tempreadfds = readfds;
        // wait for incoming connection
        select(maxfd, &tempreadfds, NULL, NULL, &tv);
        // loop through to find which socket is ready
        for (int i = 0; i < maxfd; ++i) {
            if (FD_ISSET(i, &readfds) && i == listen_tcp_sock) {
                cout <<"Incoming file request" <<endl;
                // incoming TCP connection
                send_requested_file();
            }  // end if isset
        }  // end for
    }  // end listen loop
}

void Node::rootlistener() {
    // listen on UDP
    fd_set tempreadfds;
    while (true) {
        tempreadfds = readfds;
        // wait for incoming connection
        cout <<endl <<"Waiting for incoming connections..." <<endl <<endl;
        select(maxfd, &tempreadfds, NULL, NULL, NULL);
        // loop through to find which socket is ready
        for (int i = 0; i < maxfd; ++i) {
            if (FD_ISSET(i, &readfds)) {
                if (i == listen_udp_sock) {
                    cout <<"New node joined." <<endl;
                    // incoming UDP connection
                    process_new_node();
                } else if (i == listen_tcp_sock) {
                    cout <<"Incoming file request" <<endl;
                    // incoming TCP connection
                    send_requested_file();
                } else {
                    cout <<"I swear I heard some'in" <<endl;
                }
            }  // end if isset
        }  // end for
    }  // end listen loop
}

Node::~Node() {
    close(listen_tcp_sock);
    close(listen_udp_sock);
}

std::vector<string> getfiles(string directory) {
    std::vector<string> v;
    DIR* dp;
    struct dirent* ep;
    dp = opendir(directory.c_str());

    if (dp != NULL) {
        while ((ep = readdir(dp))) {
            if (!EQUAL(ep->d_name, "..") && !EQUAL(ep->d_name, ".") &&
                    ep->d_name[0] != '.')
                v.push_back(ep->d_name);
        }
        closedir(dp);
    } else {
        error("could not open Directory");
    }
    return v;
}

Node* Node::getpointer_to_node(std::size_t node_id, Node* startptr) {
    // base condition
    cout <<"Finding pointer" <<endl;
    if (this == startptr) {
        return startptr;
    } else if (this->id == node_id) {
        cout <<"2nd" <<endl;
        return this;
    } else {
        cout <<"3rd" <<endl;
        return successor->getpointer_to_node(node_count, startptr);
    }
}

// std::pair<std::string, std::string> Node::get_file_owner(std::size_t hashval,
//         Node* startptr) {
//     if (this == startptr)
//         return make_pair("Not", "found");
//     for (int i = 0; i < keys.size(); i++) {
//         if (std::get<0>(keys) == hashval)
//             return make_pair(std::get<1>(keys), std::get<2>(keys));
//     }
//     return make_pair("Not", "found");
// }

void Node::send_files_list(std::vector<string> files) {
    Socket sockobj;
    string ip("127.0.0.1");
    string port(std::to_string(ROOT_PORT));

    string files_buff = "";
    for (int i = 0; i < files.size(); ++i) {
        files_buff = files_buff+files[i] + "\n";
    }

    sockobj.send_udp_string(ip, port, files_buff);

    cout <<"Node : Sent files list to root node @ " <<ip <<":" <<port <<endl;
    cout <<files_buff <<endl;
}

void Node::setup_sockets(bool isRootNode) {
    struct addrinfo hints, *res;
    int sockfd;
    Socket sockobj;
    string port(std::to_string(ROOT_PORT + node_count));
    string ip = sockobj.get_own_ip();

    FD_ZERO(&readfds);

    if (isRootNode) {
        listen_udp_sock = sockobj.getsocket("", port, true, false, true, false);
        maxfd = listen_udp_sock;
        FD_SET(listen_udp_sock, &readfds);
        cout <<"Listening on " <<ip <<":" <<port <<endl;
    }

    // listen for incoming TCP download requests
    listen_tcp_sock = sockobj.getsocket("", port, false, false, true, false);
    if (isRootNode)
        maxfd = listen_tcp_sock;
    else
        maxfd = (listen_tcp_sock > maxfd)? listen_tcp_sock: maxfd;
    if (listen(listen_tcp_sock, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    FD_SET(listen_udp_sock, &readfds);
    cout <<"Listening on " <<ip <<":" <<port <<endl;

    struct sigaction sa;
    sa.sa_handler = sigchild_handler;  // reap all the dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

void Node::send_requested_file() {
    int rc;
    struct sockaddr_storage their_addr;  // connector's address information
    socklen_t sin_size = sizeof their_addr;
    char filename[50];
    char s[INET6_ADDRSTRLEN];

    int new_fd = accept(listen_tcp_sock, (struct sockaddr*)&their_addr,
        &sin_size);
    if (new_fd == -1) {
        perror("accept");
        return;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
        s, sizeof s);
    printf("Peer_Server: got connection from %s\n", s);

    if (!fork()) {  // this is the child process
        close(listen_tcp_sock);  // child doesn't need the listener


        /* get the file name from the client */
        Socket sockobj;
        string filename(sockobj.receive_tcp_string(new_fd));

        string file_path("files/" + std::to_string(node_count) + "/");
        file_path.append(filename);

        sockobj.send_tcp_file(new_fd, file_path);

        close(new_fd);
        exit(0);  // let child exit
    }
    close(new_fd);  // parent doesn't need this
}

void Node::process_new_node() {
    // receive udp string from listen_udp_sock
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXDATASIZE];
    char s[INET6_ADDRSTRLEN];
    int numbytes;

    if ((numbytes = recvfrom(listen_udp_sock, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("process_new_node - recvfrom");
        exit(1);
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
        s, sizeof s);
    buf[numbytes] = '\0';

    string filenames;
    filenames.assign(buf);

    // add files to all_file_keys
    // string filenames = receive_udp_string("127.0.0.1",
            // std::to_string(ROOT_PORT));

    std::size_t pos = filenames.find_first_of('\n');
    string ip, temp;
    ip.assign(filenames.substr(0, pos));
    temp = filenames.substr(pos+1);
    filenames.assign(temp);

    pos = filenames.find_first_of('\n');
    string portno;
    portno.assign(filenames.substr(0, pos));
    temp = filenames.substr(pos+1);
    filenames.assign(temp);

    while (filenames.size() > 0) {
        pos = filenames.find_first_of('\n');
        string file;
        file.assign(filenames.substr(0, pos));
        keys_all.push_back(hash_fn(file));
        cout <<"Add : " <<file <<endl;
        temp = filenames.substr(pos+1);
        filenames.assign(temp);
    }

    // add new hash to node_keys
    cout <<"Add new node to nodes list" <<endl;
    string new_node;
    new_node.assign(ip);
    new_node.push_back(':');
    new_node.append(portno);
    nodes.push_back(hash_fn(new_node));
    cout <<"Inserting : " <<hash_fn(new_node) <<endl;
    // sort ascending
    std::sort(nodes.begin(), nodes.end());
    cout <<"Sorted, size = " <<nodes.size() <<endl;
    cout <<"Nodes are:" <<endl;
    for (int i=0; i< nodes.size(); i++)
        cout <<nodes[i] <<endl;
    cout <<endl;

    for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i] == hash_fn(new_node)) {
            if (i-1 < 0) {
                cout <<nodes[nodes.size() - 1] <<" -> " <<nodes[i] <<endl;
                cout <<getpointer_to_node(nodes[nodes.size() - 1], this)->id <<":" <<getpointer_to_node(nodes[i], this)->id <<endl;

                getpointer_to_node(nodes[nodes.size() - 1], this)->successor =
                    getpointer_to_node(nodes[i], this);

                cout <<nodes[i] <<" -> " <<nodes[i + 1] <<endl;
                cout <<getpointer_to_node(nodes[i], this)->id <<":" <<getpointer_to_node(nodes[i+1], this)->id <<endl;

                getpointer_to_node(nodes[i], this)->successor =
                    getpointer_to_node(nodes[i + 1], this);
            } else if (i+1 == nodes.size()) {
                cout <<nodes[i] <<" -> " <<nodes[0] <<endl;
                cout <<getpointer_to_node(nodes[i], this)->id <<":" <<getpointer_to_node(nodes[0], this)->id <<endl;

                getpointer_to_node(nodes[i], this)->successor =
                    getpointer_to_node(nodes[0], this);

                cout <<nodes[i - 1] <<" -> " <<nodes[i] <<endl;
                cout <<getpointer_to_node(nodes[i-1], this)->id <<":" <<getpointer_to_node(nodes[i], this)->id <<endl;

                getpointer_to_node(nodes[i-1], this)->successor =
                    getpointer_to_node(nodes[i], this);
            } else {
                cout <<nodes[i] <<" -> " <<nodes[i + 1] <<endl;
                cout <<getpointer_to_node(nodes[i], this)->id <<":" <<getpointer_to_node(nodes[i+1], this)->id <<endl;

                getpointer_to_node(nodes[i], this)->successor =
                    getpointer_to_node(nodes[i+1], this);

                cout <<nodes[i - 1] <<" -> " <<nodes[i] <<endl;
                cout <<getpointer_to_node(nodes[i-1], this)->id <<":" <<getpointer_to_node(nodes[i], this)->id <<endl;

                getpointer_to_node(nodes[i-1], this)->successor =
                    getpointer_to_node(nodes[i], this);
            }
            cout <<"Stabilize " <<nodes[i] <<endl;
            (getpointer_to_node(nodes[i], this))->stabilize();
        }
    }
}

void Node::stabilize() {
    std::vector<std::tuple<std::size_t, string, string> >keys_temp =
        successor->retrieve_keys();
    for (int i = 0; i < keys_temp.size(); i++) {
        if (std::get<0>(keys_temp[i]) < id)
            add_key(keys_temp[i]);
        else
            successor->add_key(keys_temp[i]);
    }
}

std::vector<std::tuple<std::size_t, string, string> > Node::retrieve_keys() {
    std::vector<std::tuple<std::size_t, string, string> > keys_copy = keys;
    keys.clear();
    return keys_copy;
}

void Node::add_key(std::tuple<std::size_t, string, string> key) {
    keys.push_back(key);
}

#endif  // NODE_H

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
#define CLIENT_PORT 12000
#define BACKLOG 10

using std::string;
using std::cout;
using std::endl;

void error(const char* msg) {
    perror(msg);
    exit(1);
}

std::vector<string> getfiles(string);

bool compare(std::tuple<string, string, std::size_t> first,
    std::tuple<string, string, std::size_t> second) {
    return (std::get<2>(first) < std::get<2>(second));
}

bool compare2(std::tuple<std::size_t, string, string> first,
    std::tuple<std::size_t, string, string> second) {
    return (std::get<0>(first) < std::get<0>(second));
}

class Node{
    // root variables
    std::vector<std::tuple<string, string, std::size_t> > nodes;
    std::vector<std::tuple<std::size_t, string, string> > keys_all;

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
    // successor - tuple of ip, port, hash values
    std::tuple<string, string, std::size_t> successor;
    Node(int);
    ~Node();
    void setup_sockets();
    void send_files_list(std::vector<string>);
    void rootlistener();
    void process_root_udp_connection();
    void process_file_query(string, string);
    std::pair<string, string> get_file_owner(std::size_t);
    void process_new_node(string, string);
    void print_stats(bool, bool);
    Node* getpointer_to_node(std::size_t, Node*);
    void stabilize();
    void add_key(std::tuple<std::size_t, string, string>);
    std::vector<std::tuple<std::size_t, string, string> > clear_keys();
    void nodelistener();
    void send_requested_file();
    void update_node_data();
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
    successor = std::make_tuple("", "", 0);

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
            keys_all.push_back(make_tuple(hashval, ip, port));
    }

    // set up listener sockets
    if (node_count == 0)
        setup_sockets();
    else
        setup_sockets();

    // if root node
    if (node_count == 0) {
        // initialize successor value, nodes vector
        keys.clear();
        for (int i = 0; i < keys_all.size(); i++)
            keys.push_back(keys_all[i]);
        successor = make_tuple(ip, port, id);
        nodes.push_back(make_tuple(ip, port, id));
        cout <<"Adding files :" <<endl;
        for (int i = 0; i < files_strings.size(); ++i) {
            cout <<files_strings[i] <<endl;
        }
        cout <<"---------------" <<endl;
        // listen for incoming connections
        while (true)
            rootlistener();

    } else {
        // send files to root
        send_files_list(files_strings);
        // listen for incoming connections
        while (true)
            nodelistener();
    }
}

void Node::nodelistener() {
    fd_set tempreadfds;
    while (true) {
        tempreadfds = readfds;
        // wait for incoming connection
        cout <<"Listening for file requests.." <<endl;
        select(maxfd, &tempreadfds, NULL, NULL, NULL);
        // loop through to find which socket is ready
        for (int i = 0; i < maxfd; ++i) {
            if (FD_ISSET(i, &readfds)) {
                if (i == listen_udp_sock) {
                    cout <<"New udp connection" <<endl;
                    // incoming UDP connection
                    // TODO
                    update_node_data();
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

void Node::update_node_data() {
    // Socket sockobj;
    // string reply = sockobj.receive_udp_string(std::to_string(ROOT_PORT + node_count), listen_udp_sock);
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    char buf[MAXDATASIZE];
    char ipv4addr[INET_ADDRSTRLEN];
    std::string reply;

    std::cout <<"Waiting at port " <<port.c_str() <<std::endl;

    if ((numbytes = recvfrom(listen_udp_sock, buf, MAXDATASIZE-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("socket - receive_udp_string - recvfrom");
        exit(1);
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr),
        ipv4addr, sizeof ipv4addr);

    buf[numbytes] = '\0';
    reply.assign(buf);
    std::cout <<"socket - receive_udp_string - Received UDP string : " <<reply
        <<std::endl;
}

void Node::rootlistener() {
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
                    cout <<"New udp connection" <<endl;
                    // incoming UDP connection
                    process_root_udp_connection();
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
    // TODO
    // base condition
    // cout <<"Finding pointer" <<endl;
    // if (this == startptr) {
    //     return startptr;
    // } else if (this->id == node_id) {
    //     cout <<"2nd" <<endl;
    //     return this;
    // } else {
    //     cout <<"3rd" <<endl;
    //     return successor->getpointer_to_node(node_count, startptr);
    // }
}

std::pair<std::string, std::string> Node::get_file_owner(std::size_t hashval) {
    // if (this == startptr)
    //     return make_pair("Not", "found");
    // for (int i = 0; i < keys.size(); i++) {
    //     if (std::get<0>(keys) == hashval)
    //         return make_pair(std::get<1>(keys), std::get<2>(keys));
    // }
    // return make_pair("Not", "found");
}

void Node::send_files_list(std::vector<string> files) {
    Socket sockobj;
    string ip("127.0.0.1");
    string port(std::to_string(ROOT_PORT));

    string files_buff = "";
    for (int i = 0; i < files.size(); ++i) {
        files_buff = files_buff+files[i] + "\n";
    }

    sockobj.send_udp_string(ip, port, string(std::to_string(ROOT_PORT
        + node_count) + "\n" + files_buff));

    cout <<"Node : Sent files list to root node @ " <<ip <<":" <<port <<endl;
    cout <<files_buff <<endl;
}

void Node::setup_sockets() {
    // TODO - overwriting parameter values
    struct addrinfo hints, *res;
    int sockfd;
    Socket sockobj;
    string port(std::to_string(ROOT_PORT + node_count));
    string ip = sockobj.get_own_ip();

    FD_ZERO(&readfds);

    listen_udp_sock = sockobj.getsocket("", port, true, false, true, false);
    maxfd = listen_udp_sock;
    FD_SET(listen_udp_sock, &readfds);
    cout <<"Listening on " <<ip <<":" <<port <<endl;

    // listen for incoming TCP download requests
    listen_tcp_sock = sockobj.getsocket("", port, false, false, true, false);
    maxfd = (listen_tcp_sock > maxfd)? listen_tcp_sock: maxfd;
    if (listen(listen_tcp_sock, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    FD_SET(listen_tcp_sock, &readfds);
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

void Node::process_root_udp_connection() {
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

    string message(buf);
    std::size_t pos = message.find_first_of('/');
    if(pos == 0) {
        // new IP query
        process_file_query(message.substr(pos+1), string(s));
    } else {
        // a new node is joining
        process_new_node(message, string(s));
    }
}

void Node::process_file_query(string filename, string ip_client) {
    // TODO
    // search for filename in chord ring
    Socket sockobj;
    string reply;
    bool isFound;
    if (isFound) {
        // if filename is found
        // reply - ip \n port \n id
    } else {
        reply.assign("Not found");
    }
    sockobj.send_udp_string(ip_client, std::to_string(CLIENT_PORT), reply);
}

void Node::process_new_node(string filenames, string ip_node) {
    // add files to all_file_keys
    // parse received string and add files
    std::size_t pos;
    string port_node;
    std::size_t hash_node;
    string temp;

    pos = filenames.find_first_of('\n');
    port_node.assign(filenames.substr(0, pos));
    temp = filenames.substr(pos+1);
    filenames.assign(temp);

    cout <<"New node from " <<ip_node <<":" <<port_node <<endl;

    cout <<"Adding files :" <<endl;
    cout <<"------------------" <<endl;
    while (filenames.size() > 0) {
        pos = filenames.find_first_of('\n');
        string file;
        file.assign(filenames.substr(0, pos));
        keys_all.push_back(make_tuple(hash_fn(file), ip_node, port_node));
        cout <<file <<endl;
        temp = filenames.substr(pos+1);
        filenames.assign(temp);
    }
    cout <<"------------------" <<endl;

    // add new hash to node_keys
    hash_node = hash_fn(ip_node + ":" + port_node);
    nodes.push_back(make_tuple(ip_node, port_node, hash_node));
    cout <<"Inserting new node : " <<hash_fn(ip_node + ":" + port_node) <<endl;

    print_stats(true, false);

    // TODO
    // regenerate key list for each node
    std::sort(nodes.begin(), nodes.end(), compare);
    std::sort(keys_all.begin(), keys_all.end(), compare2);

    std::vector<std::vector<int> > map_file_to_node(nodes.size());
    for (int i = 0; i < keys_all.size(); i++) {
        for (int j = 0; j < nodes.size(); j++) {
            std::size_t file_id = std::get<0>(keys_all[i]);
            std::size_t node_id = std::get<2>(nodes[j]);
            if (file_id < node_id) {
                map_file_to_node[j].push_back(i);
                break;
            }
        }
        if(std::get<0>(keys_all[i]) > std::get<2>(nodes[nodes.size() - 1]))
            map_file_to_node[0].push_back(i);
    }

    cout <<"Node file map" <<endl;
    cout <<"-----------------------" <<endl;
    for (int i = 0; i < map_file_to_node.size(); i++) {
        cout <<std::get<2>(nodes[i]) <<" : " <<endl;
        for (int j = 0; j < map_file_to_node[i].size(); j++)
            cout <<std::get<0>(keys_all[map_file_to_node[i][j]]) <<endl;
        cout <<endl;
    }
    cout <<"-----------------------" <<endl;

    // update successor and keys list for each node
    for (int i = 0; i < map_file_to_node.size(); i++) {
        if (std::get<2>(nodes[i]) == id) {
            // if root node
            successor = nodes[(i+1)%(nodes.size())];
            keys.clear();
            for (int j = 0; j < map_file_to_node[i].size(); j++)
                keys.push_back(keys_all[map_file_to_node[i][j]]);

        } else {
            // if not root node
            string updated_data;
            // append successor value
            std::tuple<string, string, std::size_t> temp = nodes[(i+1)%(nodes.size())];
            updated_data.append(std::get<0>(temp));
            updated_data.push_back('\n');
            updated_data.append(std::get<1>(temp));
            updated_data.push_back('\n');
            updated_data.append(std::to_string(std::get<2>(temp)));
            // append updated keys list
            for (int j = 0; j < map_file_to_node[i].size(); j++) {
                updated_data.push_back('\n');
                updated_data.append(std::to_string(std::get<0>(keys_all[map_file_to_node[i][j]])));
                updated_data.push_back('\n');
                updated_data.append(std::get<1>(keys_all[map_file_to_node[i][j]]));
                updated_data.push_back('\n');
                updated_data.append(std::get<2>(keys_all[map_file_to_node[i][j]]));
            }
            // send new keys list to each node
            Socket sockobj;
            sockobj.send_udp_string(std::get<0>(nodes[i]), std::get<1>(nodes[i]), updated_data);
        }
    }
}

void Node::print_stats(bool printRootStats, bool printNodeStats) {
    if (printRootStats) {
        // print nodes
        std::sort(nodes.begin(), nodes.end(), compare);
        cout <<"Sorted node list:" <<endl;
        cout <<"------------------" <<endl;
        for (int i=0; i< nodes.size(); i++)
            cout <<i+1 <<") " <<std::get<2>(nodes[i]) <<endl;
        cout <<"------------------" <<endl;

        // print all keys
        std::sort(keys_all.begin(), keys_all.end(), compare2);
        cout<<"Sorted files list" <<endl;
        cout <<"------------------" <<endl;
        for (int i = 0; i< keys_all.size(); i++)
            cout <<i+1 <<" - " <<std::get<0>(keys_all[i]) <<"-" <<std::get<1>(keys_all[i])
                <<":" <<std::get<2>(keys_all[i]) <<endl;
        cout <<"------------------" <<endl;
    }
    if (printNodeStats) {
        // print all keys
        std::sort(keys.begin(), keys.end(), compare2);
        cout<<"Sorted files list" <<endl;
        cout <<"------------------" <<endl;
        for (int i = 0; i< keys.size(); i++)
            cout <<i+1 <<" - " <<std::get<0>(keys[i]) <<"-" <<std::get<1>(keys[i])
                <<":" <<std::get<2>(keys[i]) <<endl;
        cout <<"------------------" <<endl;
    }
}

void Node::stabilize() {
    // TODO
    // std::vector<std::tuple<std::size_t, string, string> >keys_temp =
    //     successor->clear_keys();
    // for (int i = 0; i < keys_temp.size(); i++) {
    //     if (std::get<0>(keys_temp[i]) < id)
    //         add_key(keys_temp[i]);
    //     else
    //         successor->add_key(keys_temp[i]);
    // }
}

std::vector<std::tuple<std::size_t, string, string> > Node::clear_keys() {
    std::vector<std::tuple<std::size_t, string, string> > keys_copy = keys;
    keys.clear();
    return keys_copy;
}

void Node::add_key(std::tuple<std::size_t, string, string> key) {
    keys.push_back(key);
}

#endif  // NODE_H

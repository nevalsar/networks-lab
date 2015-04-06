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
    void rootlistener();
    void send_files_list(std::vector<string>);
    void nodelistener();
    void process_udp_connection();
    void process_file_query(string, string);
    void process_new_node(string, string);
    void update_node_data(string);
    void send_requested_file();
    void print_stats(bool, bool);
    std::pair<string, string> get_file_owner(std::size_t);
    Node* getpointer_to_node(std::size_t, Node*);
    void process_root_file_query(string, string);
    void process_file_query(string);
    void process_query_result(string);
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
                    process_udp_connection();
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

void Node::update_node_data(string reply) {
    // Socket sockobj;
    // string reply = sockobj.receive_udp_string(std::to_string(ROOT_PORT + node_count), listen_udp_sock);

    std::cout <<"socket - receive_udp_string - Received UDP string : " <<reply
        <<std::endl;

    // parse and update node data
    std::size_t pos;
    string temp;

    pos = reply.find_first_of('\n');
    (std::get<0>(successor)).assign(reply.substr(0, pos));
    temp = reply.substr(pos+1);
    reply.assign(temp);

    pos = reply.find_first_of('\n');
    (std::get<1>(successor)).assign(reply.substr(0, pos));
    temp = reply.substr(pos+1);
    reply.assign(temp);

    pos = reply.find_first_of('\n');
    std::get<2>(successor) = std::atoll((reply.substr(0, pos)).c_str());
    temp = reply.substr(pos+1);
    reply.assign(temp);

    cout <<"Successor : " <<std::get<0>(successor) <<":"
        <<std::get<1>(successor) <<":" <<std::get<2>(successor) << endl;
    getchar();

    keys.clear();
    cout <<"New keys :" <<endl;
    cout <<"------------------" <<endl;
    while (reply.size() > 0) {
        std::tuple<std::size_t, string, string> temp_key;

        pos = reply.find_first_of('\n');
        std::get<0>(temp_key) = std::atoll((reply.substr(0, pos)).c_str());
        temp = reply.substr(pos+1);
        reply.assign(temp);

        pos = reply.find_first_of('\n');
        (std::get<1>(temp_key)).assign(reply.substr(0, pos));
        temp = reply.substr(pos+1);
        reply.assign(temp);

        pos = reply.find_first_of('\n');
        (std::get<2>(temp_key)).assign(reply.substr(0, pos));
        temp = reply.substr(pos+1);
        reply.assign(temp);

        keys.push_back(temp_key);
        cout <<std::get<0>(temp_key) <<":" <<std::get<1>(temp_key) <<":"
            <<std::get<2>(temp_key) <<endl;
    }
    cout <<"------------------" <<endl;
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
                    process_udp_connection();
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

void Node::process_udp_connection() {
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
    std::size_t pos;
    if ((pos = message.find_first_of('/')) == 0) {
        if (node_count == 0)
            // new IP query
            process_root_file_query(message.substr(pos+1), string(s));
        else
            // forwarded IP query
            process_file_query(message.substr(pos+1));
    } else if (((pos = message.find_first_of('|')) == 0)) {
        // handle forward query result
        process_query_result(message.substr(pos+1));
    } else {
        if (node_count == 0)
            // a new node is joining
            process_new_node(message, string(s));
        else
            // update node successor and keys
            update_node_data(message);
    }
}

void Node::process_root_file_query(string filename, string ip_client) {
    // TODO
    // search for filename in chord ring
    Socket sockobj;
    string reply;
    std::size_t hashval = hash_fn(filename);
    cout <<"New query for : " <<filename <<":" <<hashval <<endl;
    bool isFound = false;
    bool ifExist = false;
    for (int i = 0; i < keys_all.size(); i++)
        if (std::get<0>(keys_all[i]) == hashval) {
            ifExist = true;
            break;
        }
    if (ifExist) {
        // if filename exists
        // check local
        for (int i = 0; i < keys.size(); i++)
            if (std::get<0>(keys[i]) == hashval) {
                isFound = true;
                break;
            }
        if (isFound) {
            cout <<"Found here" <<endl;
            reply.assign(sockobj.get_own_ip() + '\n' + std::to_string(ROOT_PORT));
            sockobj.send_udp_string(ip_client, std::to_string(CLIENT_PORT), reply);
        } else {
            cout <<"Sent to successor" <<endl;
            sockobj.send_udp_string(std::get<0>(successor), std::get<1>(successor), '/' + filename + '\n' + ip_client);
        }
    } else {
        cout <<"Not found" <<endl;
        reply.assign("Not found");
        sockobj.send_udp_string(ip_client, std::to_string(CLIENT_PORT), reply);
    }
}

void Node::process_file_query(string filename) {
    // if not root node
    std::size_t pos = filename.find_first_of('\n');
    string temp = filename.substr(0, pos);
    string ip_client = filename.substr(pos+1);
    filename.assign(temp);

    std::size_t hashval = hash_fn(filename);
    cout <<"Forwarded query for : " <<filename <<":" <<hashval <<endl;
    bool isFound = false;
    for (int i = 0; i < keys.size(); i++)
        if (std::get<0>(keys[i]) == hashval) {
            isFound = true;
            break;
        }
    Socket sockobj;
    if (isFound) {
        cout <<"Found here" <<endl;
        string reply;
        reply.assign('|' + sockobj.get_own_ip() + '\n' + std::to_string(ROOT_PORT)
            + '\n' + ip_client);
        sockobj.send_udp_string(std::get<0>(successor),
            std::get<1>(successor), reply);
    } else {
        cout <<"Sent to successor" <<endl;
        sockobj.send_udp_string(std::get<0>(successor), std::get<1>(successor), '/' + filename);
    }
    // end of non-root node
}

void Node::process_query_result(string message) {
    cout <<"Process result - ";
    Socket sockobj;

    if (node_count == 0) {
        cout <<"root" <<endl;
        std::size_t pos = message.find_first_of('\n');
        string ip_res = message.substr(0, pos);
        string temp = message.substr(pos+1);
        message.assign(temp);

        pos = message.find_first_of('\n');
        string port_res = message.substr(0, pos);
        string ip_client = message.substr(pos+1);

        string reply(ip_res + '\n' + port_res);
        sockobj.send_udp_string(ip_client, std::to_string(CLIENT_PORT), reply);
    } else {
        cout <<"non-root" <<endl;
        sockobj.send_udp_string(std::get<0>(successor), std::get<1>(successor), '|' + message);
    }
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
            updated_data.push_back('\n');
            // append updated keys list
            for (int j = 0; j < map_file_to_node[i].size(); j++) {
                updated_data.append(std::to_string(std::get<0>(keys_all[map_file_to_node[i][j]])));
                updated_data.push_back('\n');
                updated_data.append(std::get<1>(keys_all[map_file_to_node[i][j]]));
                updated_data.push_back('\n');
                updated_data.append(std::get<2>(keys_all[map_file_to_node[i][j]]));
                updated_data.push_back('\n');
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

#endif  // NODE_H

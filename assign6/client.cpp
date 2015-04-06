/**********************************
Assignment 6
CS39006 Computer Networks Lab
Submitted by:
    NEVIN VALSARAJ 12CS10032
    PRANJAL PANDEY 12CS30026
**********************************/

#include <bits/stdc++.h>
#include <stdio.h>
#include <functional>
#include <iostream>
#include <string>

#include "Socket.cpp"

#define ROOT_PORT 11000
#define CLIENT_PORT 12000

using std::cout;
using std::endl;
using std::string;

int main(int argc, char const *argv[]) {
    string ip_root;
    bool ifQuit;
    Socket sockobj;
    int sockfd;
    string port_root(std::to_string(ROOT_PORT));

    if (argc <2) {
        printf("Usage : client ROOT_NODE_IP\n");
        exit(1);
    } else {
        ip_root.assign(argv[1]);
    }

    ifQuit = false;
    // loop until exit flag triggered
    while (!ifQuit) {
        // print prompt for user input
        cout <<endl <<"-- Client --" <<endl;
        cout <<"Enter filename to query or '/quit' to Quit" <<endl;
        string filename;
        cout <<">";
        getline(std::cin, filename);

        // parse input for exit condition
        if (filename.find("/") == 0) {
            if (filename.find("/quit") == 0)
                ifQuit = true;
            else
                cout <<"Invalid command" <<endl;
            continue;
        }

        // query root node for the entered filename
        // all query filenames must be preceded by '/'
        sockobj.send_udp_string(ip_root, port_root, string("/" + filename));
        string reply(sockobj.receive_udp_string(std::to_string(CLIENT_PORT)));
        cout <<"Received reply : " <<reply <<endl;

        // parse reply to check if file found
        // returns 'Not found' if file not found
        if (reply.find("Not") != string::npos) {
            continue;
        } else {
            // if found, parse reply to get ip and port
            string temp;
            string ip_res, port_res;
            std::size_t pos;

            pos = reply.find_first_of('\n');
            ip_res.assign(reply.substr(0, pos));
            port_res.assign(reply.substr(pos+1));

            // connect and send filename
            sockfd = sockobj.getsocket(ip_res, port_res, false, true, false,
                    true);
            sockobj.send_tcp_string(sockfd, filename);
            // download file
            sockobj.receive_tcp_file(sockfd, filename);
            close(sockfd);
        }
    }

    return 0;
}

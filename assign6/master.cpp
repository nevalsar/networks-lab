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

#include "Node.cpp"

int main(int argc, char const *argv[]) {
    // int number_of_nodes;
    // std::cout <<"Enter node count: ";
    // std::cin>> number_of_nodes;
    // std::cout <<"Init " <<number_of_nodes <<" nodes" <<std::endl;
    if (argc < 2) {
        std::cout <<"Usage: master node_count" <<std::endl;
        exit(1);
    }
    // size_t pid = getpid();
    // for (int i = 0; i < atoi(argv[1]); i++)
    //     if(pid != 0){
    //         size_t tempid = fork();
    //         char arg[11][11];
    //         strcpy(arg[0], "./client.o");
    //         strcpy(arg[1], std::to_string(i));
    //         execvp("./client.o", arg);
    //     }
    Node noobj(std::atoi(argv[1]));
    return 0;
}

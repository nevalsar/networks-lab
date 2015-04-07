/**********************************
Assignment 7
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
    if (argc < 3) {
        std::cout <<"Usage: master root_ip node_count" <<std::endl;
        exit(1);
    }
    Node node_obj(argv[1], std::atoi(argv[2]));
    return 0;
}

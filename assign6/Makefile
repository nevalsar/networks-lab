run: master client
	./master 127.0.0.1 0

master:	master.cpp Node.o Socket.o
	g++ -std=c++11 -o master master.cpp

client: client.cpp
	g++ -std=c++11 -o client client.cpp

Node.o:	Node.cpp
	g++ -std=c++11 -c Node.cpp

Socket.o:	Socket.cpp
	g++ -c Socket.cpp

clean:
	rm -f master client Node.o Socket.o

#include <string>
#include <iostream>
#include "../include/ConnectionHandler.h"
#include "../include/Frame.h"
#include "../include/StompProtocol.h"
#include <thread>


using namespace std;
int main(int argc, char *argv[]) {
    
	StompProtocol* stomp_protocol = new StompProtocol();
    thread t(&StompProtocol::server_input, stomp_protocol);
    cout<<"login {host:port} {username} {password}"<<endl;
    while (1) {
        //commands from terminal
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);

		std::string line(buf);
        string word;
        stringstream stream(line);
        vector<string> words({});
        while(stream >> word)
            words.push_back(word);
        
        if(words.size() > 0)
		    stomp_protocol->terminal_input(words);

    }

	delete(stomp_protocol);
	cout<<"Disconnected"<<endl;
	return 0;
}

#include <stdlib.h>
#include "../include/connectionHandler.h"
#include <boost/thread.hpp>

/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/

class ReadFromKeyboard{

private:
    ConnectionHandler &connectionHandler;
public:

    ReadFromKeyboard(ConnectionHandler &handler): connectionHandler(handler){}

    void operator()() {
        while (true) {
            const short bufsize = 1024;
            char buf[bufsize];
            std::cin.getline(buf, bufsize);
            std::string line(buf);
            if (!connectionHandler.sendLine(line)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            // connectionHandler.sendLine(line) appends '\n' to the message. Therefor we send len+1 bytes.
            //std::cout << "Sent " << len+1 << " bytes to server" << std::endl;
        }
    }

};

class ReadFromSocket{

private:
    ConnectionHandler &connectionHandler;
public:

    ReadFromSocket(ConnectionHandler &handler): connectionHandler(handler){}

    void operator()() {
        while (true) {
            // We can use one of three options to read data from the server:
            // 1. Read a fixed number of characters
            // 2. Read a line (up to the newline character using the getline() buffered reader
            // 3. Read up to the null character
            std::string answer;
            // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
            // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
            if (!connectionHandler.getLine(answer)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }

            int len=answer.length();
            // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
            // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
            answer.resize(len-1);
            std::cout << answer << std::endl;
            if (answer == "ACK signout succeeded") { 
                break;
            }
        }
        }
};


int main (int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);
    
    ConnectionHandler connectionHandler(host, port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    ReadFromKeyboard ReadFromKeyboard(connectionHandler);
    ReadFromSocket ReadFromSocket(connectionHandler);

    boost::thread th1(ReadFromKeyboard);
    boost::thread th2(ReadFromSocket);
    th2.join();

    return 0;
}

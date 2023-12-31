#ifndef SOCKETS_CLIENT_MY_SOCKET_H
#define SOCKETS_CLIENT_MY_SOCKET_H

#include <winsock2.h>
#include <string>

class MySocket {
public:
    // vytvorenie pripojenia vďaka meno servera a kam sa idem pripojiť - číslo portu
    static MySocket* createConnection(std::string hostName, short port);
    // vráti dynamicky vytvorený socket

    // deštruktor
    ~MySocket();
    // pošle na server reťazec dát, NIKDY NEPOSIELAME integer -> ak chcem poslať číslo 123, poslem ho ako reťazec !!!
    void sendData(const std::string& data);
    void sendEndMessage();
protected:
    MySocket(SOCKET socket);
private:
    static const char * endMessage;
    // socket pomocou neho komunikjujem
    SOCKET connectSocket;
};

#endif //SOCKETS_CLIENT_MY_SOCKET_H

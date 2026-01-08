#include<iostream>
#include<winsock2.h> //core API for network comms on windows to send data across networks
#include<ws2tcpip.h>
#include<sstream> //for the istringstream thingy below
#include<string>
#include<fstream> //file handling

#pragma comment(lib, "ws2_32.lib") //link the program with ws2 library

using namespace std;

int main(){
    //1. start winsock
    WSADATA wsaData; //struct to store info from wsastartup
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //requesting winsock 2.2 (POWER ON NETWORKING!)

    if (wsaResult != 0){ 
        cerr<<"WSAStartup Failed: " <<wsaResult <<"\n";
        return 1;
    }

    cout<<"Winsock initialized successfully\n";

    //2. creating a tcp socket
    //AF_INET for ipv4 addressing
    //SOCK_STREAM for tcp
    //IPPROTO_TCP explicitly specifying tcp
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET){
        cerr<<"Socket creation failed\n";
        WSACleanup(); //cleanup winsocket
        return 1;
    }

    cout<<"Socket created successfully\n";
    //at this point, we have winsock initialized and a tcp socket exists (it is not bound tho!)

    //3. bind the socket to localhost:8080
    sockaddr_in serverAddr; //sockaddr_in is a struct that describes the IPv4 address 
    //(where this socket will be living on the network)
    serverAddr.sin_family = AF_INET; //address family is af_inet(i.e. IPv4)
    serverAddr.sin_port = htons(8080); //port number 8080. htons -> host to network short.
    //converts host byte order (8080) to network byte order 0x1F90. Computers use little endian whereas networks use
    //big endian
    serverAddr.sin_addr.s_addr = INADDR_ANY; //IP Address. Accepts connections on all network interfaces.
    //euivalent to (0.0.0.0). If u want only localhost use inet_addr(127.0.0.1). Currently the server accepts
    //from localhosts, lan ip, wifi, eth or any valid interface.

    //we are telling bind to associate this socket with this IP + Port.
    //this reserves port 8080 and no other process can use it. Incoming packets for this port comes to our socket
    //it takes generic sockaddr but since we have ipv4 one, we casted it
    //the third parameter is here to tell the bind to read how many bytes, which is obviously equal to the length of serverAddr
    if(bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR){
        cerr<<"Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout<<"Socket bound to port 8080\n";

    //4. listen for incoming conns
    //TCP state becomes to listen, OS creates a connection queue and SYN packets are accepted
    //SOMAXCONN tells to use the system default max queue length
    if(listen(serverSocket, SOMAXCONN) == SOCKET_ERROR){
        cerr<<"Listen failed!\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout<<"Server is listening on port 8080\n";

    //5. accept a client conn
    //whenever a client connects, a new socket is given to it
    //serverSocket is used to only listen
    //clientSocket kicks in when talking with client
    sockaddr_in clientAddr;
    int clientSize = sizeof(clientAddr);

    //accept takes the listening socket (clientSocket) and keeps blocking until a conn arrives.
    //it then gives the new connected socket for a client
    while(true){ //keeps it running!
    SOCKET clientSocket = accept(
        serverSocket, //listening socket
        (sockaddr*)&clientAddr, //client address (output) (where to store client info)
        &clientSize //size of address (in/out) (size of that buffer)
    );

    if(clientSocket == INVALID_SOCKET){
        cerr<<"Accept failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout<<"Client Connected!\n";

    //6. receive data from clinet (tested on chrome)
    char buffer[4096]; //4096 bytes (4KB) storage to copy data to user space
    int bytesRecieved = recv(clientSocket, buffer, sizeof(buffer) - 1, 0); //connected socket, buffer to store data,
    //max bytes to read, flags

        if (bytesRecieved > 0){
            buffer[bytesRecieved] = '\0'; //we already reserved 1 byte for \0. \0 is used to mark the end of the string
            cout<<"-----HTTP REQUEST-----\n";
            cout<<buffer<<"\n";
            cout<<"----------------------\n";
        }

        //7. sending response (not used any longer)

        //---------BASIC RESPONSE----------
        //const char* body = "<html><h1>Hello from C++ Server</h1></html>";
        
        //string response = 
        //"HTTP/1.1 200 OK\r\n"
        //"Content-Type: text/html\r\n"
        //"Content-Length: " + to_string(strlen(body)) + "\r\n"
        //"\r\n"
        //+ body;

        //send(clientSocket, response.c_str(), response.length(), 0); //pretty self-explanatory
        //---------------------------------

        //7. parse request line
        istringstream requestStream(buffer); //istringstream is used to read from a string
        string method, path, version; 

        requestStream >> method >> path >> version; //basically reads the string (stored in buffer) for cin for vars

        cout<<"Method: " << method << " | Path: " << path <<"\n";

        //8. sending response
        //route handling
        string filePath;

        if (path == "/"){
            filePath = "index.html";
        } else {
            filePath = path.substr(1); //remove /
            filePath += ".html";
        }

        ifstream file(filePath, ios::binary); //html files are required to be opened in binary mode
        string body;
        string statusLine;

        if(file){
            body.assign(
                (istreambuf_iterator<char>(file)), //copy the entire content of the file to the body
                istreambuf_iterator<char>()
            );
            statusLine = "HTTP/1.1 200 OK\r\n";
        } else {
            ifstream errorFile("404.html", ios::binary);

            body.assign(
                (istreambuf_iterator<char>(errorFile)),
                istreambuf_iterator<char>()
            );
            statusLine = "HTTP/1.1 404 Not Found\r\n";
        }

        string response = 
            statusLine +
            "Content-Type: text/html\r\n"
            "Content-Length: " + to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + 
            body;

        send(clientSocket, response.c_str(), response.size(), 0);
    }
    //cleanup
    closesocket(serverSocket); 
    WSACleanup(); //shut down winsock (POWER OFF NETWORKING!)

    return 0;
}

//g++ main.cpp -o server.exe in terminal to make a server.exe executable file (for normal files exec).
//g++ main.cpp -o server.exe -lws2_32 (use this for the socket thingy) works only with mingw.
//use .\server.exe to run
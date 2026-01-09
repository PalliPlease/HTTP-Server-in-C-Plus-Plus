# A Mini HTTP Server Written in C++
A simple HTTP/1.1 web server written in C++ using Windows Socket API (Winsock). This project shows how a basic TCP server works under the hood.

## Features
1. Listens on port 8080
2. Parses basic HTTP request lines
3. Supports simple routing
4. Serves static .html files

## Requirements
1. Windows OS
2. MinGW (g++)
3. Basic understanding of TCP/IP, HTTP and C++

## Working
1. Initialize winsock
   -> Calls WSAStartup() to enable networking
2. Create a TCP Socket
   -> Uses AF_NET (IPv4), SOCK_STREAM and IPPROTO_TCP
3. Bind Socket
   -> Binds to localhost:8080
4. Listen for Connections
   -> OS queues incoming connections
5. Accept Clients
6. Recieve HTTP Request
   -> Read raw HTTP data from browser
7. Parse Request Line
8. Route & Serve File
9. Send Response
   -> Uses send() to return HTML content

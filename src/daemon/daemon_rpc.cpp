#include "daemon_rpc.hpp"

#include <chrono>
#include <iostream>

#include "../sock_addr.hpp"

#ifdef _WIN32
#define SOCK_ERR (WSAGetLastError())
#else
#define SOCK_ERR (errno)
#endif

using namespace std;

DaemonRpc::DaemonRpc(std::string hostHeader, std::string authHeader)
    : host_header(hostHeader), auth_header(authHeader)
{
    SockAddr sock_addr(hostHeader);

    rpc_addr.sin_family = AF_INET;
    rpc_addr.sin_addr.s_addr = sock_addr.ip;
    rpc_addr.sin_port = sock_addr.port;
}

int DaemonRpc::SendRequest(std::vector<char>& result, int id, const char* method,
                             std::string& params)
{
    int bodySize, sendSize, recvSize, errCode, resCode;
    int sent, contentLength, contentReceived, headerLength;

    // initialize the socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd <= 0 /*|| sockfd == INVALID_SOCKET*/)
    {
        errCode = SOCK_ERR;

        // std::cout << "failed to create socket, error code: " << errCode
        //           << std::endl;
        return -1;
    }

    if (connect(sockfd, (const sockaddr*)&rpc_addr, sizeof(rpc_addr)) < 0)
    {
        errCode = SOCK_ERR;

        std::cerr << "failed to connect to rpc socket, error code: " << errCode
                  << std::endl;
        return -1;
    }

    // generate the http request

    const int bodyLen = params.size() + 64;
    char* body = new char[bodyLen];

    bodySize = snprintf(body, bodyLen,
                        "{\"id\":%d,\"method\":\"%s\",\"params\":[%s]}", id,
                        method, params.c_str());

    const int sendBufferLen = bodySize + 256;
    char* sendBuffer = new char[bodySize + 256];
    sendSize =
        snprintf(sendBuffer, sendBufferLen,
                 "POST / HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Authorization: Basic %s\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %d\r\n\r\n"
                 "%s\r\n\r\n",
                 host_header.c_str(), auth_header.c_str(), bodySize, body);

    sent = send(sockfd, sendBuffer, sendSize, 0);
    delete[] sendBuffer;
    delete[] body;

    if (sent < 0) return -1;

    char* headerBuff = new char[HEADER_SIZE + 1];

    int headerRecv = 0;
    char* endOfHeader = 0;
    // receive http header (and potentially part or the whole body)
    do
    {
        int recvRes = recv(sockfd, headerBuff + headerRecv, HEADER_SIZE - headerRecv, 0);
        if (recvRes <= 0)
        {
            std::cerr << "rpc socket receive error code: " << errno
                      << std::endl;
            delete[] headerBuff;
            return -1;
        }
        headerRecv += recvRes;
        headerBuff[headerRecv -1] = 0;
    } while ((endOfHeader = std::strstr(headerBuff, "\r\n\r\n")) == NULL);

    endOfHeader += 4;
    headerBuff[headerRecv - 1] = '\0';

    resCode = std::atoi(headerBuff + std::strlen("HTTP/1.1 "));

    contentLength = std::atoi(std::strstr(headerBuff, "Content-Length: ") +
                              std::strlen("Content-Length: "));
    contentReceived = std::strlen(endOfHeader);
    headerLength = HEADER_SIZE - contentReceived - 1;

    // std::cout << "HTTP CODE: " << resCode << std::endl;
    // std::cout << "CONTENT LENGTH: " << contentLength << std::endl;
    // std::cout << "CONTENT RECEIVED: " << contentReceived << std::endl;
    // std::cout << "TOTAL RECEIVED: " << totalRecv << std::endl;

    result.resize(contentLength + 1);
    memcpy(result.data(), headerBuff + headerLength, contentReceived);
    delete[] headerBuff;
    // receive http body if it wasn't already
    while (contentReceived < contentLength)
    {
        int recvRes = recv(sockfd, result.data() + contentReceived,
                           contentLength - contentReceived, 0);
        if (recvRes <= 0)
        {
            std::cerr << "rpc socket receive error code: " << errno
                      << std::endl;
            return -1;
        }
        contentReceived += recvRes;
    }

    result[contentReceived - 1] = 0;

    close(sockfd);
    return resCode;
}
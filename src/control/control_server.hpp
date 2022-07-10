#ifndef MANAGE_SERVER_HPP_
#define MANAGE_SERVER_HPP_

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

enum class ControlCommands
{
    NONE = 0,
    UPDATE_BLOCK = 1,
};

class ControlServer
{
   public:
    void Start(unsigned short port)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        int optval = 1;

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                       sizeof(optval)) != 0)
        {
            throw std::runtime_error("Failed to set control server options");
        }
        
        if (bind(sockfd, (const sockaddr*)&addr, sizeof(addr)) != 0)
        {
            throw std::runtime_error("Control server failed to bind to port " +
                                     std::to_string(port));
        }

        if (listen(sockfd, 4) != 0)
        {
            throw std::runtime_error(
                "Control server failed to enter listenning state.");
        }
    }

    ControlCommands GetNextCommand() const
    {
        // one command per connection
        // we don't care about the address, as it must be local (is it
        // secure?)
        int connfd = accept(sockfd, nullptr, nullptr);

        char buf[128];
        if (recv(connfd, buf, sizeof(buf), 0) <= 0)
        {
            // disconnected
            return ControlCommands::NONE;
        }

        auto cmd = static_cast<ControlCommands>(buf[0] - '0');
        close(connfd);

        Logger::Log(LogType::Info, LogField::ControlServer,
                    "Received command: {}", (int)cmd);

        return cmd;
    }

   private:
    int sockfd;
};

#endif
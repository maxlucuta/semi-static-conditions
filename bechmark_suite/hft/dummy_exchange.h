#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


class Exchange
{
    private:
        int socketFD;
        struct sockaddr_in serverAddress;
        const char* buy = "BUY";
        const char* sell = "SELL";

    public:
        Exchange(int port = 8000, const char* ip = "69.69.69.69")
        {   
            serverAddress = {0};
            socketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(port);
            serverAddress.sin_addr.s_addr = inet_addr(ip);
        }

        ~Exchange()
        {
            close(socketFD);
        }

        void sendBuyOrder(int amount)
        {
            const char* amountStr = std::to_string(amount).c_str();
            char orderDesc[strlen(amountStr) + strlen(buy) + 1];
            strcpy(orderDesc, amountStr);
            strcat(orderDesc, buy);
            sendto(
                socketFD, 
                (const char *)orderDesc, 
                strlen(orderDesc),
                MSG_CONFIRM, 
                (const struct sockaddr*) &serverAddress, 
                sizeof(serverAddress)
            );
        }

        void sendSellOrder(int amount)
        {
            const char* amountStr = std::to_string(amount).c_str();
            char orderDesc[strlen(amountStr) + strlen(sell) + 1];
            strcpy(orderDesc, amountStr);
            strcat(orderDesc, sell);
            sendto(
                socketFD, 
                (const char *)orderDesc, 
                strlen(orderDesc),
                MSG_CONFIRM, 
                (const struct sockaddr*) &serverAddress, 
                sizeof(serverAddress)
            );
        }
};
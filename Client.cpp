#include "Client.hpp"

static void usage(void)
{
    std::cout << "\nUsage:\n"
              << "For setting up connection:\n"
                 "\t\tclient [-h] <IP address>\n"
                 "Description:\n"
                 "      -h            Get the help information.\n"
                 "  <IP address>      Server IP address.\n"
                 "For query reachability:"
                 "\t\t<source>  <destination>\n"
                 "Description:\n"
                 "    <source>        Source, the beginning.\n"
                 "   <destination>>   Destination, the end.\n"
              << std::endl;
}

int main(int argc, char **argv)
{
    // IP address is needed.
    if (argc != 2 || strcmp(argv[1], "-h") == 0)
    {
        usage();
        exit(0);
    }

    // define socekt file descriptor for IPv4, use AF_INET6 for IPv6 instead.
    int sock_cli = ::socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;  // protocol family
    server_addr.sin_addr.s_addr = ::inet_addr(argv[1]);
    server_addr.sin_port = htons(PORT);  // the port the server is listening to

    if (::connect(sock_cli, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Connect failed." << std::endl;
        exit(-1);
    }
    std::cout << "Connecting to server(IP:" << argv[1] << ")." << std::endl;

    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    std::cout << "Please input query or type \"exit\" to quit:" << std::endl;
    while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        ::send(sock_cli, sendbuf, strlen(sendbuf), 0); // send argv
        std::cout << "Data sent." << std::endl;
        if (strcmp(sendbuf, "over\n") == 0)
        {
            std::cout << "Client received data:" << std::endl;
            memset(recvbuf, 0, sizeof(recvbuf));
            while (::recv(sock_cli, recvbuf, sizeof(recvbuf), 0)) // receive data
            {
                if (strcmp(recvbuf, "over") != 0)
                {
                    std::cout << recvbuf << std::endl;   // print the received data
                    memset(recvbuf, 0, sizeof(recvbuf)); // clear receive buffer
                }
                else 
                {
                    break;
                }
            }                                        
        }
        if (strcmp(sendbuf, "exit\n") == 0)
        {
            std::cout << "Client exited." << std::endl;
            break;
        }
        memset(sendbuf, 0, sizeof(sendbuf));  // clear sending buffer
        std::cout << "Please input query(\"over\" to finish sending queries) or \"exit\" to quit:" << std::endl;
    }
    ::close(sock_cli);  // close client socket
    return 0;
}

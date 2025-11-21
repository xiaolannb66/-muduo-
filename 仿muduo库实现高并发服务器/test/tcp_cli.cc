#include "../source/server.hpp"

int main()
{
    Socket cli_sock;
    cli_sock.CreateClient(8500, "127.0.0.1");
    for (int i = 0; i < 5; i++) {
        std::string str = "hello bitejiuyeke!";
        cli_sock.Send(str.c_str(), str.size());
        char buf[1024] = {0};
        cli_sock.Recv(buf, 1023);
        DBG_LOG("%s", buf);
        sleep(1);
    }
    while(1) sleep(1);
    return 0;
}
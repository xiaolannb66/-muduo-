#include "../source/server.hpp"

void OnConnected(const PtrConnection &conn) {
    DBG_LOG("NEW CONNECTION:%p", conn.get());
}
void OnClosed(const PtrConnection &conn) {
    DBG_LOG("CLOSE CONNECTION:%p", conn.get());
}
void OnMessage(const PtrConnection &conn, Buffer *buf) {
    DBG_LOG("%s", buf->ReadPosition());
    buf->MoveReadOffset(buf->ReadAbleSize());
    std::string str = "Hello World";
    conn->Send(str.c_str(), str.size());
}
int main()
{
    TcpServer server(8500);
    server.SetThreadCount(2);
    //server.EnableInactiveRelease(10);
    server.SetClosedCallback(OnClosed);
    server.SetConnectedCallback(OnConnected);
    server.SetMessageCallback(OnMessage);
    server.Start();
    return 0;
}
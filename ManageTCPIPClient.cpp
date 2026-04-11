// ManageTCPIPClient.cpp : 定义静态库的函数。
//

#include "pch.h"//编译时第一行，先加载了pch.h,MangeTCPIPClient.h 时已经知晓库
//#include "framework.h"// 已经在 pch.h 中包含了
#include "MangeTCPIPClient.h"


static bool g_wsaInit = false;

static void InitWinsock()
{
    if (!g_wsaInit)
    {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        g_wsaInit = true;
    }
}

TCPClient::TCPClient()
    : m_socket(INVALID_SOCKET)
    , m_connected(false)
{
    InitWinsock();
}

TCPClient::~TCPClient()
{
    Disconnect();
}

bool TCPClient::Connect(const std::string& ip, int port)
{
    Disconnect();

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    if (connect(s, (sockaddr*)&addr, sizeof(addr)) < 0)
    {
        closesocket(s);
        return false;
    }

    m_socket = s;
    m_connected = true;
    return true;
}

void TCPClient::Disconnect()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    m_connected = false;
}

int TCPClient::Send(const char* data, int len)
{
    if (!m_connected || m_socket == INVALID_SOCKET) return -1;
    return send(m_socket, data, len, 0);
}

int TCPClient::Receive(char* buf, int bufLen)
{
    if (!m_connected || m_socket == INVALID_SOCKET) return -1;
    return recv(m_socket, buf, bufLen, 0);
}

bool TCPClient::IsConnected() const
{
    return m_connected;
}

ClientManager::~ClientManager()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_clients.clear();
}

std::string ClientManager::CreateClient()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    static int idx = 0;
    std::string id = "cli_" + std::to_string(idx++);
    m_clients[id] = std::make_unique<TCPClient>();
    return id;
}

bool ClientManager::DestroyClient(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_clients.find(id);
    if (it == m_clients.end()) return false;
    m_clients.erase(it);
    return true;
}

TCPClient* ClientManager::GetClient(const std::string& id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_clients.find(id);
    if (it == m_clients.end()) return nullptr;
    return it->second.get();
}

bool ClientManager::ConnectClient(const std::string& id, const std::string& ip, int port)
{
    auto* c = GetClient(id);
    if (!c) return false;
    return c->Connect(ip, port);
}

// 在 ClientManager 类中添加/修改以下方法

bool ClientManager::SendData(const std::string& id, const char* data, int len, int& bytesSent)
{
    auto* c = GetClient(id);
    if (!c || !c->IsConnected()) return false;
    bytesSent = c->Send(data, len);
    return bytesSent > 0;
}

int ClientManager::RecvData(const std::string& id, char* buf, int bufLen)
{
    auto* c = GetClient(id);
    if (!c || !c->IsConnected()) return -1;
    return c->Receive(buf, bufLen);
}

// 补充缺失的导出函数
extern "C" {

    EXPORT_API bool CALL_CONV DestroyClient(void* manager, const char* clientId)
    {
        auto* mgr = static_cast<ClientManager*>(manager);
        return mgr->DestroyClient(clientId);
    }

    EXPORT_API bool CALL_CONV IsClientConnected(void* manager, const char* clientId)
    {
        auto* mgr = static_cast<ClientManager*>(manager);
        auto* client = mgr->GetClient(clientId);
        return client ? client->IsConnected() : false;
    }

    EXPORT_API void CALL_CONV DisconnectClient(void* manager, const char* clientId)
    {
        auto* mgr = static_cast<ClientManager*>(manager);
        auto* client = mgr->GetClient(clientId);
        if (client) client->Disconnect();
    }
}
#pragma once

//#define CLIENT_MANGE_DLL//编译DLL时定义(可以zaiC/C++预处理器->预处理器定义中添加)，使用时注释掉
#ifdef CLIENT_MANGE_DLL
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
#endif

#define CALL_CONV __stdcall

#pragma region//头文件依赖

#pragma endregion

#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

class TCPClient
{
public:
    TCPClient();
    ~TCPClient();
    bool Connect(const std::string& ip, int port);
    void Disconnect();
    int Send(const char* data, int len);
    int Receive(char* buf, int bufLen);
    bool IsConnected() const;
private:
    SOCKET m_socket;
    bool m_connected;
};

class ClientManager
{
public:
    ClientManager() = default;
    ~ClientManager();
    std::string CreateClient();
    bool DestroyClient(const std::string& clientId);
    TCPClient* GetClient(const std::string& clientId);
    bool ConnectClient(const std::string& clientId, const std::string& ip, int port);
    bool SendData(const std::string& clientId, const char* data, int len, int& bytesSent);
    int RecvData(const std::string& clientId, char* buf, int bufLen);
private:
    std::unordered_map<std::string, std::unique_ptr<TCPClient>> m_clients;
    std::mutex m_mutex;
};

//  纯 C 接口导出（C# 和 C++ 都能用）
extern "C" {//返回bool,C++为1字节，C#4字节，可以返回int，0失败，1成功或特征[return: MarshalAs(UnmanagedType.I1)](强制把这个布尔值当作 1 个字节的整数来处理)
    // 管理器
    EXPORT_API void* CALL_CONV CreateManager();
    EXPORT_API void CALL_CONV DestroyManager(void* manager);

    // 客户端操作（全部使用 void* 或 char*，不暴露 std::string）
    EXPORT_API const char* CALL_CONV CreateClient(void* manager);
    EXPORT_API bool CALL_CONV DestroyClient(void* manager, const char* clientId);

    // 注意：这里不要暴露 TCPClient*，而是用 void* (C用此)或者句柄
    EXPORT_API bool CALL_CONV ConnectClient(void* manager, const char* clientId, const char* ip, int port);
    EXPORT_API bool CALL_CONV IsClientConnected(void* manager, const char* clientId);
    EXPORT_API void CALL_CONV DisconnectClient(void* manager, const char* clientId);

    // 数据收发
    EXPORT_API bool CALL_CONV SendData(void* manager, const char* clientId, const char* data, int len);
    EXPORT_API int CALL_CONV RecvData(void* manager, const char* clientId, char* buf, int bufLen);

    // 内存释放
    EXPORT_API void CALL_CONV FreeString(const char* str);
}
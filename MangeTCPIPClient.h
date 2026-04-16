#pragma once

//#define CLIENT_MANGE_DLL//编译DLL时定义(可以zaiC/C++预处理器->预处理器定义中添加)，使用时注释掉
#ifdef CLIENT_MANGE_DLL
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __declspec(dllimport)
#endif

#define CALL_CONV __stdcall//标准


//  纯 C 接口导出（C# 和 C++ 都能用）
extern "C" {//返回bool,C++为1字节，C#4字节，可以返回int，0失败，1成功或特征[return: MarshalAs(UnmanagedType.I1)](强制把这个布尔值当作 1 个字节的整数来处理)
    //后续bool还是改int
    typedef void* TCPIP_CLIENT_MANAGER_HANDLE;//只暴露客户端管理句柄
    // 管理器
    //创建句柄对象后调用此分配内存
    EXPORT_API void* CALL_CONV CreateManager();
    //销毁分配的内存
    EXPORT_API void CALL_CONV DestroyManager(void* manager);

    // 客户端操作（全部使用 void* 或 char*，不暴露 std::string）
    EXPORT_API bool CALL_CONV CreateClient(void* manager, const char* clientId);
    EXPORT_API bool CALL_CONV DestroyClient(void* manager, const char* clientId);

    // 用 void* (C用此)或者句柄
    EXPORT_API bool CALL_CONV ConnectClient(void* manager, const char* clientId, const char* ip, int port);
    EXPORT_API bool CALL_CONV IsClientConnected(void* manager, const char* clientId);
    EXPORT_API void CALL_CONV DisconnectClient(void* manager, const char* clientId);

    // 数据收发
    EXPORT_API bool CALL_CONV SendData(void* manager, const char* clientId, const char* data, int len);
    //单收
    EXPORT_API int CALL_CONV RecvData(void* manager, const char* clientId, char* buf, int bufLen);
    //设置接收等待时间,0成功，-1失败,-2套接字未实例化
    EXPORT_API int CALL_CONV SetReceiveTimeout(void* manager, const char* clientId, int timeout);

    // 内存释放
    //EXPORT_API void CALL_CONV FreeString(const char* str);
}
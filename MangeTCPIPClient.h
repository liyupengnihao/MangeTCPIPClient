#pragma once

//#define CLIENT_MANGE_DLL//编译DLL时定义(可以zaiC/C++预处理器->预处理器定义中添加)，使用时注释掉
#ifdef CLIENT_MANGE_DLL
#define ManageTCPIPClient_API __declspec(dllexport)
#else
#define ManageTCPIPClient_API __declspec(dllimport)
#endif

#define CALL_CONV __stdcall//标准C#与C++都可用

#define FUNC_OK     0//对外导出函数0代表正常


//  纯 C 接口导出（C# 和 C++ 都能用）//所有bool改int，0成功其他值代表其他
extern "C" {//返回bool,C++为1字节，C#4字节，可以返回int，或特征[return: MarshalAs(UnmanagedType.I1)](强制把这个布尔值当作 1 个字节的整数来处理)
    //后续bool还是改int
    typedef void* TCPIP_CLIENT_MANAGER_HANDLE;//只暴露客户端管理句柄
    // 管理器
    //创建句柄对象后调用此分配内存
    ManageTCPIPClient_API void* CALL_CONV CreateManager();
    //销毁分配的内存
    ManageTCPIPClient_API void CALL_CONV DestroyManager(void* manager);

    // 客户端操作（全部使用 void* 或 char*，不暴露 std::string），0成功-1失败
    ManageTCPIPClient_API int CALL_CONV CreateClient(void* manager, const char* clientId);
    // 断开连接并销毁，0成功，-1失败
    ManageTCPIPClient_API int CALL_CONV DestroyClient(void* manager, const char* clientId);

    // 连接客户端，0成功，-1失败
    ManageTCPIPClient_API int CALL_CONV ConnectClient(void* manager, const char* clientId, const char* ip, int port);
    // 是否是连接状态,0连接状态，-1非连接状态，-2ID下无套接字对象
    ManageTCPIPClient_API int CALL_CONV IsClientConnected(void* manager, const char* clientId);
    // 断开连接
    ManageTCPIPClient_API void CALL_CONV DisconnectClient(void* manager, const char* clientId);

    // 数据收发,返回发送字节数
    ManageTCPIPClient_API int CALL_CONV SendData(void* manager, const char* clientId, const char* data, int len);
    //单次读取,返回读取的字节数
    ManageTCPIPClient_API int CALL_CONV RecvData(void* manager, const char* clientId, char* buf, int bufLen);
    //设置接收等待时间,0成功，-1失败,-2套接字未实例化
    ManageTCPIPClient_API int CALL_CONV SetReceiveTimeout(void* manager, const char* clientId, int timeout);

    // 内存释放
    //EXPORT_API void CALL_CONV FreeString(const char* str);
}
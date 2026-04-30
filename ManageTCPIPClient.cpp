// ManageTCPIPClient.cpp : 定义静态库的函数。
//

#include "pch.h"//编译时第一行，先加载了pch.h,MangeTCPIPClient.h 时已经知晓库
//#include "framework.h"// 已经在 pch.h 中包含了
#include "MangeTCPIPClient.h"

using std::string;

static bool g_wsaInit = false;

static void InitWinsock()
{//初始化网络环境
	if (!g_wsaInit)
	{
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
		g_wsaInit = true;
	}
}

/// <summary>
/// TCPIP客户端
/// </summary>
class TCPClientBase
{//基类只管基础功能
public:
	TCPClientBase();
	~TCPClientBase();
	/// <summary>
	/// 连接服务器
	/// </summary>
	/// <param name="ip"></param>
	/// <param name="port"></param>
	/// <returns>0成功，-1无效套接字，-2连接失败</returns>
	int Connect(const string& ip, int port);
	/// <summary>
	/// 断开连接
	/// </summary>
	/// <returns>0成功</returns>
	int Disconnect();
	/// <summary>
	/// 设置接收等待时间，单位毫秒
	/// </summary>
	/// <param name="timeout"></param>
	/// <returns></returns>
	int SetReceiveTimeout(int timeout);
	/// <summary>
	/// 发送字节
	/// </summary>
	/// <param name="data">发送数据</param>
	/// <param name="len">数据长度</param>
	/// <returns>发送的字节数，-1表示发送失败</returns>
	int Send(const char* data, int len);
	/// <summary>
	/// 单读取只读取缓冲区中全部值
	/// </summary>
	/// <param name="buf">接收缓冲区</param>
	/// <param name="bufLen">缓冲区长度</param>
	/// <returns>接收到的字节数，0等待超时(设置等待时间的话)-1表示接收失败</returns>
	int Receive(char* buf, int bufLen);
	/// <summary>
	/// 读取标志位状态
	/// </summary>
	/// <returns>0连接，-1连接失败</returns>
	int IsConnected() const;//不修改任何成员变量
protected:
	virtual bool OnConnect(const string& ip, int port);//导出动态库，诺导出有虚函数就必须导出整个类。不导出虚函数必须有实现。纯虚不能new TCPClientBase。还是空实现吧
	virtual void OnDisconnect();
	virtual int OnSetReceiveTimeout(int timeout);
	virtual int OnSend(const char* data, int len);
	virtual int OnReceive(char* buf, int bufLen);
	virtual bool OnIsConnected() const;
	//ip
	string m_ip;
	//端口
	int m_port;
	//套接字对象
	SOCKET m_socket;
	//连接状态
	bool m_connected;
};

/// <summary>
/// TCPIP客户端管理
/// </summary>
class ClientManagerBase
{
public:
	ClientManagerBase() = default;//default默认构造函数，不进行其他操作
	~ClientManagerBase();
	bool CreateClient(string idx);
	bool DestroyClient(const string& clientId);
	
	bool ConnectClient(const string& clientId, const string& ip, int port);
	bool SendData(const string& clientId, const char* data, int len, int& bytesSent);
	int RecvData(const string& clientId, char* buf, int bufLen);//单搜索，无如何处理
	TCPClientBase* GetClient(const string& clientId);//辅助函数，
protected://子类可访问
	//供子类重写
	virtual bool OnCreateClient(string idx);
	virtual bool OnDestroyClient(const string& clientId);

	virtual bool OnConnectClient(const string& clientId, const string& ip, int port);
	virtual bool OnSendData(const string& clientId, const char* data, int len, int& bytesSent);
	virtual int OnRecvData(const string& clientId, char* buf, int bufLen);


	std::unordered_map<string, std::unique_ptr<TCPClientBase>> m_clients;
	//哈希表(基于数组实现)(客户端标识字符串，unique_ptr(对应客户端，独占所有权的智能指针(只有一个unique_ptr指针指向对象，自动释放)))
	
	std::mutex m_mutex;//互斥锁，对m_clients的操作加锁
	//锁可让调用方传入，内部不假定一点多线程调用库(C++高版本可以，第版本应该不行，C#也不行)

private://子类也无法访问
	
};



TCPClientBase::TCPClientBase()
	: m_socket(INVALID_SOCKET)
	, m_connected(false)
{
	InitWinsock();
}

TCPClientBase::~TCPClientBase()
{
	Disconnect();
}

int TCPClientBase::Connect(const string& ip, int port)
{
	Disconnect();

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) return -1;

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	m_ip = ip;
	m_port = port;

	if (connect(s, (sockaddr*)&addr, sizeof(addr)) < 0)
	{
		closesocket(s);
		return -2;
	}

	//开启保活，内核检测是否在线
	int val = 1; // 1 代表 true
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*)&val, sizeof(val));

	m_socket = s;
	m_connected = true;
	return 0;
}

int TCPClientBase::Disconnect()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
	m_connected = false;
	return 0;
}

int TCPClientBase::SetReceiveTimeout(int timeout)
{
	if (!m_connected || m_socket == INVALID_SOCKET) return -1;
	return setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

int TCPClientBase::Send(const char* data, int len)
{
	if (!m_connected || m_socket == INVALID_SOCKET) return -1;
	return send(m_socket, data, len, 0);
}

int TCPClientBase::Receive(char* buf, int bufLen)
{
	if (!m_connected || m_socket == INVALID_SOCKET) return -1;
	return recv(m_socket, buf, bufLen, 0);
}

int TCPClientBase::IsConnected() const
{
	return m_connected ? 0 : -1;
}

#pragma region TCPClientBase虚函数默认实现
bool TCPClientBase::OnConnect(const string& ip, int port)
{
	return false;
}

void TCPClientBase::OnDisconnect()
{

}

int TCPClientBase::OnSetReceiveTimeout(int timeout)
{
	return -1;
}

int TCPClientBase::OnSend(const char* data, int len)
{
	return -1;
}

int TCPClientBase::OnReceive(char* buf, int bufLen)
{
	return -1;
}
bool TCPClientBase::OnIsConnected() const
{
	return m_connected;
}
#pragma endregion

ClientManagerBase::~ClientManagerBase()
{//释放m_clients资源
	std::lock_guard<std::mutex> lock(m_mutex);
	m_clients.clear();//调用每个unique_ptr的析构释放内存
}

bool ClientManagerBase::CreateClient(string idx)
{//哈希键由外部传入
	std::lock_guard<std::mutex> lock(m_mutex);//锁的是所在作用域从此开始到作用域结束
	m_clients[idx] = std::make_unique<TCPClientBase>();
	return true;
}

bool ClientManagerBase::DestroyClient(const string& id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_clients.find(id);
	if (it == m_clients.end()) return false;
	m_clients.erase(it);//删除哈希表键值
	return true;
}

bool ClientManagerBase::ConnectClient(const string& id, const string& ip, int port)
{//连接键对应对象
	auto* c = GetClient(id);
	if (!c) return false;
	return c->Connect(ip, port);
}

// 在 ClientManager 类中添加/修改以下方法

bool ClientManagerBase::SendData(const string& id, const char* data, int len, int& bytesSent)
{//键对应对象发送数据
	auto* c = GetClient(id);
	if (!c || !c->IsConnected()) return false;
	bytesSent = c->Send(data, len);
	return bytesSent > 0;
}

int ClientManagerBase::RecvData(const string& id, char* buf, int bufLen)
{//得到键对应对象的数据
	auto* c = GetClient(id);
	if (!c || !c->IsConnected()) return -1;
	return c->Receive(buf, bufLen);
}

TCPClientBase* ClientManagerBase::GetClient(const string& id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_clients.find(id);
	if (it == m_clients.end()) return nullptr;
	return it->second.get();//得到值，TCPClient对象
}

#pragma region ClientManagerBase虚函数默认实现
bool ClientManagerBase::OnCreateClient(string idx)
{
	return false;
}

bool ClientManagerBase::OnDestroyClient(const string& clientId)
{
	return false;
}

bool ClientManagerBase::OnConnectClient(const string& clientId, const string& ip, int port)
{
	return false;
}

bool ClientManagerBase::OnSendData(const string& clientId, const char* data, int len, int& bytesSent)
{
	return false;
}

int ClientManagerBase::OnRecvData(const string& clientId, char* buf, int bufLen)
{
	return -1;
}
#pragma endregion


//#pragma region 纯C导出函数实现，给VC++和C#调用的

ManageTCPIPClient_API void* CALL_CONV CreateManager()
{
	return new ClientManagerBase();
}

ManageTCPIPClient_API void CALL_CONV DestroyManager(void* manager)
{
	ClientManagerBase* mgr = static_cast<ClientManagerBase*>(manager);
	mgr->~ClientManagerBase();
}

ManageTCPIPClient_API int CALL_CONV CreateClient(void* manager, const char* clientId)
{//外部传入键
	auto* mgr = static_cast<ClientManagerBase*>(manager);
	return mgr->CreateClient(clientId) ? 0 : -1;//char*->std::string可以隐式，反向不行
}

ManageTCPIPClient_API int CALL_CONV DestroyClient(void* manager, const char* clientId)
{
	auto* mgr = static_cast<ClientManagerBase*>(manager);//static_cast编译判断manager是ClientManager或ClientManager的父类
	return mgr->DestroyClient(clientId) ? 0 : -1;
}

ManageTCPIPClient_API int CALL_CONV ConnectClient(void* manager, const char* clientId, const char* ip, int port)
{
	auto* mgr = static_cast<ClientManagerBase*>(manager);
	auto* client = mgr->GetClient(clientId);

	if (client) return client->Connect(ip, port) ? 0 : -1;
	return -1;
}

ManageTCPIPClient_API int CALL_CONV IsClientConnected(void* manager, const char* clientId)
{
	auto* mgr = static_cast<ClientManagerBase*>(manager);
	auto* client = mgr->GetClient(clientId);
	return client ? (client->IsConnected() ? 0 : -1 ): -2;//加不加括号都是一个意思
}

ManageTCPIPClient_API void CALL_CONV DisconnectClient(void* manager, const char* clientId)
{
	auto* mgr = static_cast<ClientManagerBase*>(manager);
	auto* client = mgr->GetClient(clientId);
	if (client) client->Disconnect();
}

ManageTCPIPClient_API int CALL_CONV SendData(void* manager, const char* clientId, const char* data, int len)
{
	auto* mgr = static_cast<ClientManagerBase*>(manager);
	auto* client = mgr->GetClient(clientId);
	int rel = 0;
	if (client)
	{
		rel = client->Send(data, len);
	}
	return rel;

}

ManageTCPIPClient_API int CALL_CONV RecvData(void* manager, const char* clientId, char* buf, int bufLen)
{
	auto* mgr = static_cast<ClientManagerBase*>(manager);
	auto* client = mgr->GetClient(clientId);
	if (client)
	{
		client->Receive(buf, bufLen);
	}
}

ManageTCPIPClient_API int CALL_CONV SetReceiveTimeout(void* manager, const char* clientId, int timeout)
{
	auto* mgr = static_cast<ClientManagerBase*>(manager);
	auto* client = mgr->GetClient(clientId);
	if (client) return client->SetReceiveTimeout(timeout);
	return -2;
}
//#pragma endregion
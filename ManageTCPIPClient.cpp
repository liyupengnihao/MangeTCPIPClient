// ManageTCPIPClient.cpp : 定义静态库的函数。
//

#include "pch.h"//编译时第一行，先加载了pch.h,MangeTCPIPClient.h 时已经知晓库
//#include "framework.h"// 已经在 pch.h 中包含了
#include "MangeTCPIPClient.h"

using std::string;

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


//不让外部看见
class TCPClient//可做接口ClientManager管理接口
{
public:
	TCPClient();
	~TCPClient();
	/// <summary>
	/// 创建套接字对象并连接
	/// </summary>
	/// <param name="ip"></param>
	/// <param name="port"></param>
	/// <returns></returns>
	bool Connect(const string& ip, int port);
	void Disconnect();
	int SetReceiveTimeout(int timeout);
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
	ClientManager() = default;//default默认构造函数，不进行其他操作
	~ClientManager();
	bool CreateClient(string idx);
	bool DestroyClient(const string& clientId);
	TCPClient* GetClient(const string& clientId);
	bool ConnectClient(const string& clientId, const string& ip, int port);
	bool SendData(const string& clientId, const char* data, int len, int& bytesSent);
	int RecvData(const string& clientId, char* buf, int bufLen);
private:
	std::unordered_map<string, std::unique_ptr<TCPClient>> m_clients;//哈希表O(1)查找效率(客户端标识字符串，unique_ptr(对应客户端，独占所有权的智能指针(只有一个unique_ptr指针指向对象，自动释放)))
	std::mutex m_mutex;//互斥锁，对m_clients的操作加锁
	//锁可让调用方传入，内部不假定一点多线程调用库
};



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

bool TCPClient::Connect(const string& ip, int port)
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

	//开启保活，内核检测是否在线
	int val = 1; // 1 代表 true
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*)&val, sizeof(val));

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

int TCPClient::SetReceiveTimeout(int timeout)
{
	if (!m_connected || m_socket == INVALID_SOCKET) return -1;
	return setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

ClientManager::~ClientManager()
{//释放m_clients资源
	std::lock_guard<std::mutex> lock(m_mutex);
	m_clients.clear();//调用每个unique_ptr的析构释放内存
}

bool ClientManager::CreateClient(string idx)
{//哈希键由外部传入
	std::lock_guard<std::mutex> lock(m_mutex);//锁的是所在作用域从此开始到作用域结束
	m_clients[idx] = std::make_unique<TCPClient>();
	return true;
}

bool ClientManager::DestroyClient(const string& id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_clients.find(id);
	if (it == m_clients.end()) return false;
	m_clients.erase(it);//删除哈希表键值
	return true;
}

TCPClient* ClientManager::GetClient(const string& id)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_clients.find(id);
	if (it == m_clients.end()) return nullptr;
	return it->second.get();//得到值，TCPClient对象
}

bool ClientManager::ConnectClient(const string& id, const string& ip, int port)
{//连接键对应对象
	auto* c = GetClient(id);
	if (!c) return false;
	return c->Connect(ip, port);
}

// 在 ClientManager 类中添加/修改以下方法

bool ClientManager::SendData(const string& id, const char* data, int len, int& bytesSent)
{//键对应对象发送数据
	auto* c = GetClient(id);
	if (!c || !c->IsConnected()) return false;
	bytesSent = c->Send(data, len);
	return bytesSent > 0;
}

int ClientManager::RecvData(const string& id, char* buf, int bufLen)
{//得到键对应对象的数据
	auto* c = GetClient(id);
	if (!c || !c->IsConnected()) return -1;
	return c->Receive(buf, bufLen);
}


#pragma region 纯C导出函数实现

EXPORT_API void* CALL_CONV CreateManager()
{
	return new ClientManager();
}

EXPORT_API void CALL_CONV DestroyManager(void* manager)
{
	ClientManager* mgr = static_cast<ClientManager*>(manager);
	mgr->~ClientManager();
}

EXPORT_API bool CALL_CONV CreateClient(void* manager, const char* clientId)
{//外部传入键
	auto* mgr = static_cast<ClientManager*>(manager);
	return mgr->CreateClient(clientId);//char*->std::string可以隐式，反向不行
}

EXPORT_API bool CALL_CONV DestroyClient(void* manager, const char* clientId)
{
	auto* mgr = static_cast<ClientManager*>(manager);//static_cast编译判断manager是ClientManager或ClientManager的父类
	return mgr->DestroyClient(clientId);
}

EXPORT_API bool CALL_CONV ConnectClient(void* manager, const char* clientId, const char* ip, int port)
{
	auto* mgr = static_cast<ClientManager*>(manager);
	auto* client = mgr->GetClient(clientId);

	if (client) return client->Connect(ip, port);
	return false;
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

EXPORT_API bool CALL_CONV SendData(void* manager, const char* clientId, const char* data, int len)
{
	auto* mgr = static_cast<ClientManager*>(manager);
	auto* client = mgr->GetClient(clientId);
	int rel = 0;
	if (client)
	{
		rel = client->Send(data, len);
	}
	return rel ? true : false;
		
}

EXPORT_API int CALL_CONV RecvData(void* manager, const char* clientId, char* buf, int bufLen)
{
	auto* mgr = static_cast<ClientManager*>(manager);
	auto* client = mgr->GetClient(clientId);
	if (client)
	{
		client->Receive(buf, bufLen);
	}
}

EXPORT_API int CALL_CONV SetReceiveTimeout(void* manager, const char* clientId, int timeout)
{
	auto* mgr = static_cast<ClientManager*>(manager);
	auto* client = mgr->GetClient(clientId);
	if (client) return client->SetReceiveTimeout(timeout);
	return -2;
}
#pragma endregion
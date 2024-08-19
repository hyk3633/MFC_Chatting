#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <memory>
#include "DataType.h"
#include "pch.h"

class CMFCServerDlg;
class Database;

const unsigned int MSG_CLIENT_LOGIN = WM_USER + 1;
const unsigned int MSG_REMOVE_ID	= WM_USER + 2;

struct PacketInfo
{
	std::vector<char> tempBuf;
	size_t totalSize = 0, dataSize = 0, pos = 0;
	int recvSize = 0;
	DataType type = DataType::NONE;
};

struct SocketInfo
{
	WSAOVERLAPPED overlapped;	// overlapped 구조체
	WSABUF wsaBuf;				// io 작업버퍼
	SOCKET socket;
	char msgBuf[PACKET_SIZE];
	int number = -1;
	PacketInfo packetInfo;
};

class Server
{
public:

	Server(CMFCServerDlg* dlg);

	virtual ~Server();

	bool InitializeServer();

	void StartServer();

	void WorkerThread();

	void AddSocketInfo(SocketInfo* socketInfo);

	inline bool IsAcceptable() const { return bAccept; }

protected:

	bool CreateWorkerThreads();

	static void Recv(SocketInfo* socketInfo);

	void ProcessPacket(SocketInfo* socketInfo, const PacketInfo& packetInfo);

	void SignUpToDB(SocketInfo* socketInfo, const char* buf, const size_t& size);

	void Login(SocketInfo* socketInfo, const char* buf, const size_t& size);

	std::wstring MultiByteToUnicode(const char* buf, const int& size);

	std::string UnicodeToMultiByte(const std::wstring& wStr);

	void BroadcastText(SocketInfo* socketInfo, const char* buf, const size_t& size);

	void SendIdToOtherClients(const std::wstring& id, const int clientNumber);

	void SendAllClientsId(SocketInfo* socketInfo);

	void Send(SocketInfo* socketInfo, const char* buf, const size_t& size);

	void Broadcast(const char* buf, const size_t& size, const int skipNumber = -1);

	void SaveImageExt(const int clientNumber, const char* buf, const size_t& size);

	void RecvImage(const int clientNumber, const char* buf, const size_t& size);

	void BroadcastImage(const int clientNumber, const char* buf, const size_t& size);

	void FindImageAndSend(SocketInfo* socketInfo, const char* buf, const size_t& size);

	void ShutdownServer();

	void RemoveDisconnectedClient(SocketInfo* socketInfo);

	void InitializePacketInfo(PacketInfo& packetInfo);

	void ImageToBytes(std::vector<char>& buf, const std::wstring& filePath, const std::wstring& fileExt, size_t& size);

	void WriteStringToBuffer(const std::wstring& wStr, const DataType type, std::vector<char>& buf, size_t& packetSize);

protected:

	CMFCServerDlg* ownerDlg;

	CRITICAL_SECTION critsecWork;

	SOCKET listenSocket;

	HANDLE iocpHandle;

	std::thread* accepterThread;

	std::vector<HANDLE*> workerThreads;

	int threadCount = 0;

	int clientCount = 0;

	std::unordered_map<int, SocketInfo*> clientsSocketMap;

	std::unordered_map<int, std::wstring> idMap;

	std::unordered_map<int, std::wstring> profileImageExtMap;

	std::unique_ptr<Database> dbPtr;

	bool bAccept = false;

	bool bRecv = false;

};
#include "pch.h"
#include "Server.h"
#include "Database.h"
#include "MFC_ServerDlg.h"
#include <fstream>

using namespace std;

unsigned int AccepterThread(LPVOID p, SOCKET& listenSocket, HANDLE& iocpHandle);

unsigned int WINAPI WorkerThreadStart(LPVOID param)
{
	Server* iocpEvent = reinterpret_cast<Server*>(param);
	iocpEvent->WorkerThread();
	return 0;
}

Server::Server(CMFCServerDlg* dlg) : ownerDlg(dlg)
{
}

Server::~Server()
{
	ShutdownServer();
}

bool Server::InitializeServer()
{
	if (dbPtr == nullptr)
	{
		dbPtr = make_unique<Database>();
		if (dbPtr->Initialize())
		{
			if (dbPtr->Connect() == false)
				return false;
		}
		else
			return false;
	}

	// WinSock 초기화
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		return false;
	}

	// 리스닝 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}

	// 소켓에 IP 주소 및 포트 번호를 바인딩 하기 위한 IPv4 패킷용 주소 구조체
	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 바인딩
	result = bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
	if (result != 0)
	{
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 소켓 리스닝 시작
	result = listen(listenSocket, 5);
	if (result != 0)
	{
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	return true;
}

void Server::StartServer()
{
	// Completion port 객체 생성
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (iocpHandle == NULL)
	{
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	InitializeCriticalSection(&critsecWork);

	bRecv = true;

	// Worker thread 생성
	if (!CreateWorkerThreads())
	{
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	bAccept = true;

	accepterThread = new thread(&AccepterThread, this, std::ref(listenSocket), std::ref(iocpHandle));
}

bool Server::CreateWorkerThreads()
{
	unsigned int threadId;
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	threadCount = sysInfo.dwNumberOfProcessors * 2;

	for (int i = 0; i < threadCount; i++)
	{
		workerThreads.push_back((HANDLE*)_beginthreadex(NULL, 0, &WorkerThreadStart, this, CREATE_SUSPENDED, &threadId));
		if (workerThreads[i] == NULL)
		{
			return false;
		}
		ResumeThread(workerThreads[i]);
	}

	return true;
}

void Server::WorkerThread()
{
	int result;
	DWORD recvBytes;
	SocketInfo* completionKey;
	SocketInfo* recvSocketInfo;

	while (bRecv)
	{
		EnterCriticalSection(&critsecWork);
		result = GetQueuedCompletionStatus(iocpHandle, &recvBytes, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&recvSocketInfo, INFINITE);
		LeaveCriticalSection(&critsecWork);
		if (!result || !recvBytes)
		{
			RemoveDisconnectedClient(recvSocketInfo);
			continue;
		}

		if (recvSocketInfo == nullptr)
			continue;

		recvSocketInfo->wsaBuf.len = recvBytes;

		char* recvBuf = recvSocketInfo->wsaBuf.buf;
		PacketInfo& packetInfo = recvSocketInfo->packetInfo;

		if (packetInfo.type == DataType::NONE || packetInfo.totalSize < packetInfo.dataSize + 1 + sizeof(size_t))
		{
			packetInfo.totalSize += recvBytes;
			if (packetInfo.type == DataType::NONE)
			{
				packetInfo.type = static_cast<DataType>(recvBuf[0]);
				CopyMemory(&packetInfo.dataSize, &recvBuf[1], sizeof(size_t));

				if (packetInfo.tempBuf.size() == 0)
				{
					size_t packetSize = 1 + sizeof(size_t) + packetInfo.dataSize;
					packetInfo.tempBuf.reserve(packetSize);
					packetInfo.tempBuf.insert(packetInfo.tempBuf.begin(), packetSize, 0);
				}
			}
			CopyMemory(&packetInfo.tempBuf[packetInfo.pos], recvBuf, recvBytes);
			packetInfo.pos += recvBytes;
		}

		if (packetInfo.totalSize == packetInfo.dataSize + 1 + sizeof(size_t))
		{
			ProcessPacket(recvSocketInfo, packetInfo);
			InitializePacketInfo(packetInfo);
		}

		Recv(recvSocketInfo);
	}
}

void Server::AddSocketInfo(SocketInfo* socketInfo)
{
	socketInfo->number = clientCount++;
	clientsSocketMap[socketInfo->number] = socketInfo;
}

unsigned int AccepterThread(LPVOID p, SOCKET& listenSocket, HANDLE& iocpHandle)
{
	Server* server = reinterpret_cast<Server*>(p);

	int result;
	SOCKET clientSocket;
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	DWORD recvBytes = 0;
	DWORD flags = 0;

	while (server->IsAcceptable())
	{
		clientSocket = WSAAccept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen, NULL, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			closesocket(clientSocket);
			return 0;
		}

		SocketInfo* socketInfo = new SocketInfo();
		socketInfo->socket = clientSocket;
		socketInfo->wsaBuf.len = PACKET_SIZE;
		socketInfo->wsaBuf.buf = socketInfo->msgBuf;

		server->AddSocketInfo(socketInfo);

		iocpHandle = CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (DWORD)socketInfo, 0);

		result = WSARecv(socketInfo->socket, &socketInfo->wsaBuf, 1, &recvBytes, &flags, &(socketInfo->overlapped), NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			return 0;
		}
	}

	return 0;
}

void Server::Recv(SocketInfo* socketInfo)
{
	DWORD flags = 0;

	ZeroMemory(&(socketInfo->overlapped), sizeof(OVERLAPPED));
	ZeroMemory(socketInfo->msgBuf, PACKET_SIZE);
	socketInfo->wsaBuf.len = PACKET_SIZE;
	socketInfo->wsaBuf.buf = socketInfo->msgBuf;

	int result = WSARecv(socketInfo->socket, &(socketInfo->wsaBuf), 1, (LPDWORD)&socketInfo, &flags, (LPWSAOVERLAPPED) & (socketInfo->overlapped), NULL);
	if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
	{
		cout << "[Error] : Failed to receive packet!\n";
	}
}

void Server::ProcessPacket(SocketInfo* socketInfo, const PacketInfo& packetInfo)
{
	switch (packetInfo.type)
	{
	case DataType::SIGNUP_INFO:
		SignUpToDB(socketInfo, &packetInfo.tempBuf[1 + sizeof(size_t)], packetInfo.dataSize);
		break;
	case DataType::LOGIN_INFO:
		Login(socketInfo, &packetInfo.tempBuf[1 + sizeof(size_t)], packetInfo.dataSize);
		break;
	case DataType::TEXT:
		BroadcastText(socketInfo, &packetInfo.tempBuf[1 + sizeof(size_t)], packetInfo.dataSize);
		break;
	case DataType::IMAGE_EXT:
		SaveImageExt(socketInfo->number, &packetInfo.tempBuf[1 + sizeof(size_t)], packetInfo.dataSize);
		break;
	case DataType::IMAGE:
		RecvImage(socketInfo->number, &packetInfo.tempBuf[0], packetInfo.totalSize);
		break;
	case DataType::IMAGE_REQUEST:
		FindImageAndSend(socketInfo, &packetInfo.tempBuf[1 + sizeof(size_t)], packetInfo.dataSize);
		break;
	}
}

void Server::SignUpToDB(SocketInfo* socketInfo, const char* buf, const size_t& size)
{
	wstring wStr = MultiByteToUnicode(buf, size);

	wstringstream ss(wStr);
	wstring id, pw;

	ss >> id >> pw;

	bool result = dbPtr->SignUp(id, pw);
	vector<char> sendBuf(2, 0);
	sendBuf[0] = static_cast<int>(DataType::SIGNUP_INFO);
	sendBuf[1] = result;
	Send(socketInfo, &sendBuf[0], 2);
}

void Server::Login(SocketInfo* socketInfo, const char* buf, const size_t& size)
{
	wstring wStr = MultiByteToUnicode(buf, size);

	wstringstream ss(wStr);
	wstring id, pw;

	ss >> id >> pw;

	bool result = dbPtr->Login(id, pw);
	vector<char> sendBuf(2, 0);
	sendBuf[0] = static_cast<int>(DataType::LOGIN_INFO);
	sendBuf[1] = result;
	Send(socketInfo, &sendBuf[0], 2);

	if (result)
	{
		// 리스트 추가
		ownerDlg->SendMessage(MSG_CLIENT_LOGIN, (WPARAM)&buf[0], id.length());
		idMap[socketInfo->number] = id;
		
		// 방금 접속한 클라의 id를 다른 클라들에 브로드캐스트
		SendIdToOtherClients(id, socketInfo->number);

		// 접속 중인 클라들의 id들을 방금 접속한 클라에게 
		SendAllClientsId(socketInfo);
	}
}

std::wstring Server::MultiByteToUnicode(const char* buf, const int& size)
{
	if (size <= 0)
		return wstring().c_str();

	int strSize = MultiByteToWideChar(CP_UTF8, 0, buf, size, NULL, 0);
	wstring wStr(strSize, 0);
	strSize = MultiByteToWideChar(CP_UTF8, 0, buf, size, &wStr[0], size);
	return wStr.c_str();
}

std::string Server::UnicodeToMultiByte(const std::wstring& wStr)
{
	if (wStr.empty())
		return string().c_str();

	int size = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), (int)wStr.size(), NULL, 0, NULL, NULL);
	string str(size, 0);
	size = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), (int)wStr.size(), &str[0], size, NULL, NULL);
	return str;
}

void Server::BroadcastText(SocketInfo* socketInfo, const char* buf, const size_t& size)
{
	wstring id = L"[" + idMap[socketInfo->number] + L"] : ";
	wstring text = MultiByteToUnicode(buf, size);
	vector<char> sendBuf;
	size_t packetSize = 0;
	WriteStringToBuffer(id + text, DataType::TEXT, sendBuf, packetSize);

	Broadcast(&sendBuf[0], packetSize, socketInfo->number);
}

void Server::SendIdToOtherClients(const std::wstring& id, const int clientNumber)
{
	string idStr = UnicodeToMultiByte(id);

	size_t idSize = idStr.length();
	size_t dataSize = 1 + sizeof(size_t) + idSize;
	size_t packetSize = 1 + sizeof(size_t) + dataSize;
	vector<char> sendBuf(packetSize, 0);

	sendBuf[0] = static_cast<int>(DataType::ID);
	CopyMemory(&sendBuf[1], &dataSize, sizeof(size_t));
	sendBuf[1 + sizeof(size_t)] = 1;
	CopyMemory(&sendBuf[2 + sizeof(size_t)], &idSize, sizeof(size_t));
	CopyMemory(&sendBuf[2 + sizeof(size_t) + sizeof(size_t)], &idStr[0], idSize);

	Broadcast(&sendBuf[0], packetSize, clientNumber);
}

void Server::SendAllClientsId(SocketInfo* socketInfo)
{
	if (idMap.size() == 1)
		return;

	size_t dataSize = 1;
	for (auto& pair : idMap)
	{
		if (pair.first == socketInfo->number)
			continue;
		dataSize += sizeof(size_t) + pair.second.length();
	}

	size_t packetSize = 1 + sizeof(size_t) + dataSize;
	vector<char> sendBuf(packetSize, 0);
	sendBuf[0] = static_cast<int>(DataType::ID);
	CopyMemory(&sendBuf[1], &dataSize, sizeof(size_t));
	sendBuf[1 + sizeof(size_t)] = idMap.size() - 1;
	size_t pos = 2 + sizeof(size_t);

	for (auto& pair : idMap)
	{
		if (pair.first == socketInfo->number)
			continue;

		string idStr = UnicodeToMultiByte(pair.second);
		size_t idSize = idStr.length();

		CopyMemory(&sendBuf[pos], &idSize, sizeof(size_t));
		CopyMemory(&sendBuf[pos + sizeof(size_t)], &idStr[0], idSize);

		pos += sizeof(size_t) + idSize;
	}

	Send(socketInfo, &sendBuf[0], packetSize);
}

void Server::Send(SocketInfo* socketInfo, const char* buf, const size_t& size)
{
	if (size <= PACKET_SIZE)
	{
		int sentSize = 0;
		while (sentSize < size)
		{
			sentSize += send(socketInfo->socket, &buf[sentSize], size, 0);
		}
	}
	else
	{
		size_t totalSize = size, pos = 0;
		int sendSize = 0, sentSize = 0;

		while (totalSize > 0)
		{
			if (totalSize > PACKET_SIZE)
				sendSize = PACKET_SIZE;
			else
				sendSize = totalSize;

			sentSize = send(socketInfo->socket, &buf[pos], sendSize, 0);

			totalSize -= sentSize;
			pos += sentSize;
		}
	}
}

void Server::Broadcast(const char* buf, const size_t& size, const int skipNumber)
{
	for (auto& pair : clientsSocketMap)
	{
		if (pair.first == skipNumber)
			continue;

		Send(pair.second, buf, size);
	}
}

void Server::SaveImageExt(const int clientNumber, const char* buf, const size_t& size)
{
	wstring wStr = MultiByteToUnicode(buf, size);
	profileImageExtMap[clientNumber] = wStr;
}

void Server::RecvImage(const int clientNumber, const char* buf, const size_t& size)
{
	// 파일 저장
	wstring imageName = idMap[clientNumber] + L"." + profileImageExtMap[clientNumber];
	ofstream file(imageName, ios::binary | ios::out);
	file.write(&buf[1 + sizeof(size_t)], size);
	file.close();

	BroadcastImage(clientNumber, buf, size);
}

void Server::BroadcastImage(const int clientNumber, const char* buf, const size_t& size)
{
	if (clientsSocketMap.size() == 1)
		return;

	wstring wStr = idMap[clientNumber] + L" " + profileImageExtMap[clientNumber];
	string str = UnicodeToMultiByte(wStr);

	size_t dataSize = str.length();
	size_t packetSize = 1 + sizeof(size_t) + dataSize;

	vector<char> sendBuf(packetSize, 0);

	sendBuf[0] = static_cast<int>(DataType::IMAGE_EXT);
	CopyMemory(&sendBuf[1], &dataSize, sizeof(size_t));
	CopyMemory(&sendBuf[1 + sizeof(size_t)], &str[0], str.length());

	Broadcast(&sendBuf[0], packetSize, clientNumber);

	std::this_thread::sleep_for(100ms);

	int a = buf[0];
	Broadcast(buf, size, clientNumber);
}

void Server::FindImageAndSend(SocketInfo* socketInfo, const char* buf, const size_t& size)
{
	wstring id = MultiByteToUnicode(buf, size);
	wstring imageExt;
	for (auto& pair : idMap)
	{
		if (pair.second == id)
		{
			if (profileImageExtMap.find(pair.first) == profileImageExtMap.end())
				return;
			else
			{
				imageExt = profileImageExtMap[pair.first];
				break;
			}
		}
	}

	wstring imageName = id + L"." + imageExt;
	ifstream file(imageName, ios::binary | ios::in);
	if (file.is_open())
	{
		wstring wStr = id + L" " + imageExt;
		vector<char> sendBuf;
		size_t packetSize = 0;
		WriteStringToBuffer(wStr, DataType::IMAGE_EXT, sendBuf, packetSize);

		Send(socketInfo, &sendBuf[0], packetSize);

		std::this_thread::sleep_for(100ms);

		vector<char> fileBuf;
		size_t sendSize = 0;
		ImageToBytes(fileBuf, imageName, imageExt, sendSize);

		Send(socketInfo, &fileBuf[0], sendSize);
	}
}

void Server::ShutdownServer()
{
	bRecv = bAccept = false;

	WSACleanup();

	DeleteCriticalSection(&critsecWork);

	for (auto& pair : clientsSocketMap)
	{
		SocketInfo* socketInfo = pair.second;
		if (socketInfo->socket != INVALID_SOCKET)
		{
			closesocket(socketInfo->socket);
		}
		if (socketInfo)
		{
			delete socketInfo;
			socketInfo = nullptr;
		}
	}
	clientsSocketMap.clear();

	closesocket(listenSocket);
	if (accepterThread)
	{
		if (accepterThread->joinable())
			accepterThread->join();
	
		delete accepterThread;
		accepterThread = nullptr;
	}

	for (HANDLE* workerThread : workerThreads)
	{
		if (workerThread)
		{
			CloseHandle(workerThread);
			workerThread = nullptr;
		}
	}
	workerThreads.clear();

	dbPtr->Close();
}

void Server::ImageToBytes(std::vector<char>& buf, const std::wstring& filePath, const std::wstring& fileExt, size_t& size)
{
	ifstream file(filePath, ios::binary | ios::in);

	if (file.is_open())
	{
		file.seekg(0, ios::end);
		size_t fileSize = file.tellg();

		file.clear();
		file.seekg(0, ios::beg);

		size_t packetSize = 1 + fileSize + sizeof(size_t);
		size = packetSize;

		buf = vector<char>(packetSize);

		buf[0] = static_cast<int>(DataType::IMAGE);
		CopyMemory(&buf[1], &fileSize, sizeof(size_t));

		file.read(&buf[1 + sizeof(size_t)], size);
		file.close();
	}
}

void Server::WriteStringToBuffer(const wstring& wStr, const DataType type, std::vector<char>& buf, size_t& packetSize)
{
	string str = UnicodeToMultiByte(wStr);
	size_t dataSize = str.length();
	packetSize = 1 + sizeof(size_t) + dataSize;
	buf = vector<char>(packetSize, 0);
	buf[0] = static_cast<int>(type);
	CopyMemory(&buf[1], &dataSize, sizeof(size_t));
	CopyMemory(&buf[1 + sizeof(size_t)], &str[0], dataSize);
}

void Server::InitializePacketInfo(PacketInfo& packetInfo)
{
	packetInfo.dataSize = 0;
	packetInfo.totalSize = 0;
	packetInfo.recvSize = 0;
	packetInfo.pos = 0;
	packetInfo.type = DataType::NONE;
	packetInfo.tempBuf.clear();
}

void Server::RemoveDisconnectedClient(SocketInfo* socketInfo)
{
	EnterCriticalSection(&critsecWork);

	if (idMap.find(socketInfo->number) != idMap.end())
	{
		const wstring id = idMap[socketInfo->number];
		vector<char> sendBuf;
		size_t packetSize = 0;
		WriteStringToBuffer(id, DataType::DISCONNECTED_ID, sendBuf, packetSize);

		Broadcast(&sendBuf[0], packetSize, socketInfo->number);

		const wchar_t* wchPtr = &id[0];
		ownerDlg->SendMessage(MSG_REMOVE_ID, (WPARAM)&wchPtr[0], (LPARAM)id.length());

		idMap.erase(socketInfo->number);
	}

	if (clientsSocketMap.find(socketInfo->number) != clientsSocketMap.end())
	{
		if (socketInfo->socket != INVALID_SOCKET)
		{
			closesocket(socketInfo->socket);
		}
		clientsSocketMap.erase(socketInfo->number);
		if (socketInfo)
		{
			delete socketInfo;
			socketInfo = nullptr;
		}
	}

	LeaveCriticalSection(&critsecWork);
}

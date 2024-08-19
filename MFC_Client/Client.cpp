#include "pch.h"
#include "Client.h"
#include "MFC_ClientDlg.h"
#include <fstream>

using namespace std;

unsigned int ThreadRecv(LPVOID p, SOCKET& sock);
unsigned int ThreadSend(LPVOID p, SOCKET& sock, const DataType& type, const wstring& path = L"", const wstring& ext = L"BMP");

void Client::SetDlg(CMFCClientDlg* dlg)
{
	ownerDlg = dlg;
}

Client::~Client()
{
	DisconnectToServer();
}

bool Client::InitSocket()
{
	WSADATA wsaData;
	int result;
	// WinSock 초기화
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		return false;
	}

	// 소켓 생성
	mySocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (mySocket == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}

	return true;
}

bool Client::StartSocket()
{
	// 접속하고자 하는 서버에 대한 주소 세팅
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(9999);
	inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr.s_addr));

	// 서버에 연결 요청
	if (connect(mySocket, (sockaddr*)&addr, sizeof(addr)) != 0)
	{
		return false;
	}

	if (recvThread == nullptr)
	{
		isAbleRecv = true;
		recvThread = new thread(&ThreadRecv, this, std::ref(mySocket));
	}

	pathName = wstring(MAX_PATH, 0);
	HRESULT result = SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, &pathName[0]);

	if (SUCCEEDED(result))
	{
		pathName.erase(remove(pathName.begin(), pathName.end(), L'\0'), pathName.end());
		pathName += L"\\MFC_Chatting";
		if (GetFileAttributes(pathName.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory(pathName.c_str(), NULL);
		}
		pathName += L"\\Client";
		if (GetFileAttributes(pathName.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory(pathName.c_str(), NULL);
		}
	}
	else
	{
		pathName = L"";
	}

	return true;
}

void Client::ProcessSignUpResult(const char* buf)
{
	ownerDlg->SendMessage(MSG_SIGNUP_RESULT, (WPARAM)&buf[1], 1);
}

void Client::ProcessLoginResult(const char* buf)
{
	ownerDlg->SendMessage(MSG_LOGIN_RESULT, (WPARAM)&buf[1], 1);
}

unsigned int ThreadRecv(LPVOID p, SOCKET& sock)
{
	Client* client = reinterpret_cast<Client*>(p);
	if (client == nullptr)
		return 0;

	bool bDisconnected = false;
	char buf[PACKET_SIZE];
	vector<char> recvBuf;

	while (client->GetIsAbleRecv())
	{
		ZeroMemory(buf, PACKET_SIZE);

		size_t totalSize = 0, dataSize = 0, pos = 0;
		int recvSize = 0;
		DataType type = DataType::NONE;
		
		while (type == DataType::NONE || totalSize < 1 + sizeof(size_t) + dataSize)
		{
			recvSize = recv(sock, buf, PACKET_SIZE, 0);

			if (recvSize < 0)
			{
				bDisconnected = true;
				break;
			}
			else
			{
				totalSize += recvSize;
				if (type == DataType::NONE)
				{
					type = static_cast<DataType>(buf[0]);

					if (static_cast<int>(type) <= 1)
					{
						bool result = buf[1];
						if (type == DataType::SIGNUP_INFO)
							client->ProcessSignUpResult(buf);
						else
							client->ProcessLoginResult(buf);
						break;
					}
					else
					{
						CopyMemory(&dataSize, &buf[1], sizeof(size_t));
						if (recvBuf.size() == 0)
						{
							size_t packetSize = 1 + sizeof(size_t) + dataSize;
							recvBuf.reserve(packetSize);
							recvBuf.insert(recvBuf.begin(), packetSize, 0);
						}
					}
				}
				CopyMemory(&recvBuf[pos], buf, recvSize);
				pos += recvSize;
			}
		}

		if (bDisconnected)
			break;

		if (totalSize == 1 + sizeof(size_t) + dataSize)
		{
			client->ProcessPacket(&recvBuf[0], dataSize, type);
			recvBuf.clear();
		}
	}

	client->SendDisconnectedMessage();

	return 0;
}

unsigned int ThreadSend(LPVOID p, SOCKET& sock, const DataType& type, const wstring& path, const wstring& ext)
{
	Client* client = reinterpret_cast<Client*>(p);
	if (client == nullptr)
		return 0;

	if (type == DataType::IMAGE)
	{
		client->SendText(DataType::IMAGE_EXT, ext);
		std::this_thread::sleep_for(100ms);

		size_t packetSize = 0;
		vector<char> buf;
		client->ImageToBytes(buf, path, ext, packetSize);
		client->Send(&buf[0], packetSize);
	}

	return 0;
}

void Client::SendText(const DataType type, const std::wstring& wStr)
{
	string text = UnicodeToMultyByte(wStr);

	size_t textSize = text.length();
	size_t packetSize = 1 + sizeof(size_t) + textSize;

	vector<char> buf(packetSize, 0);
	buf[0] = static_cast<int>(type);
	CopyMemory(&buf[1], &textSize, sizeof(size_t));
	CopyMemory(&buf[1 + sizeof(size_t)], &text[0], textSize);

	const bool result = Send(&buf[0], packetSize);
}

std::string Client::UnicodeToMultyByte(const std::wstring& wStr)
{
	if (!wStr.length())
		return string();

	int size = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), (int)wStr.size(), NULL, 0, NULL, NULL);
	string str(size, 0);
	size = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), (int)wStr.size(), &str[0], size, NULL, NULL);
	return str;
}

bool Client::Send(const char* buf, const size_t& size)
{
	if (mySocket != INVALID_SOCKET)
	{
		if (size <= PACKET_SIZE)
		{
			int sentSize = 0;
			while (sentSize < size)
			{
				sentSize += send(mySocket, &buf[sentSize], (int)size, 0);
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

				sentSize = send(mySocket, &buf[pos], sendSize, 0);

				totalSize -= sentSize;
				pos += sentSize;
			}
		}
		return true;
	}
	else
		return false;
}

void Client::ProcessPacket(const char* buf, const size_t& size, const DataType type)
{
	switch (type)
	{
	case DataType::ID:
	{
		ownerDlg->SendMessage(MSG_UPDATE_ID_LIST, (WPARAM)&buf[1 + sizeof(size_t)], size);
		break;
	}
	case DataType::TEXT:
	{
		ownerDlg->SendMessage(MSG_RECV_TEXT, (WPARAM)&buf[1 + sizeof(size_t)], size);
		break;
	}
	case DataType::IMAGE_EXT:
	{
		SaveImageName(buf, size);
		break;
	}
	case DataType::IMAGE:
	{
		SaveImage(buf, size);
		break;
	}
	case DataType::IMAGE_REQUEST:
	{
		SendImageRequestMessage(buf, size);
		break;
	}
	case DataType::DISCONNECTED_ID:
	{
		RemoveDisconnectedId(buf, size);
		break;
	}
	}
}

void Client::SaveImageName(const char* buf, const size_t& size)
{
	wstring wStr = MultiByteToUnicode(&buf[1 + sizeof(size_t)], size);
	wstringstream wss(wStr);
	wstring id, ext;
	wss >> id >> ext;
	idExtMap[id] = ext;
	lastImageName = id + L"." + ext;
}

void Client::SendImageRequestMessage(const char* buf, const size_t& size)
{
	SaveImage(buf, size);
	wstring imageName = lastImageName;
	const wchar_t* wStrPtr = &imageName[0];
	ownerDlg->SendMessage(MSG_RECV_TEXT, (WPARAM)&wStrPtr, (LPARAM)imageName.length());
}

void Client::RemoveDisconnectedId(const char* buf, const size_t& size)
{
	wstring id = MultiByteToUnicode(&buf[1 + sizeof(size_t)], size);
	const wchar_t* wchPtr = &id[0];
	ownerDlg->SendMessage(MSG_REMOVE_ID, (WPARAM)&wchPtr[0], (LPARAM)id.length());
	idExtMap.erase(id);
}

std::wstring Client::MultiByteToUnicode(const char* buf, const int& size)
{
	if (size <= 0)
		return wstring().c_str();

	int strSize = MultiByteToWideChar(CP_UTF8, 0, buf, size, NULL, 0);
	wstring wStr(strSize, 0);
	strSize = MultiByteToWideChar(CP_UTF8, 0, buf, size, &wStr[0], size);

	return wStr.c_str();
}

void Client::SaveImage(const char* buf, const size_t& size)
{
	if (lastImageName.length() == 0)
		return;

	wstring filePath = pathName + L"\\" + myId;

	if (GetFileAttributes(filePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		if (!CreateDirectory(filePath.c_str(), NULL))
			return;
	}

	ofstream file(filePath + L"\\" + lastImageName, ios::binary | ios::out);
	file.write(&buf[1 + sizeof(size_t)], size);
	file.close();
}

void Client::SendImage(const std::wstring& filePath, const std::wstring& fileExt, const DataType type)
{
	if (sendThread == nullptr)
	{
		sendThread = new thread(&ThreadSend, this, std::ref(mySocket), type, filePath, fileExt);

		if (sendThread->joinable())
			sendThread->join();

		delete sendThread;
		sendThread = nullptr;
	}
}

void Client::ImageToBytes(std::vector<char>& buf, const std::wstring& filePath, const std::wstring& fileExt, size_t& size)
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

std::wstring Client::GetImageExt(const std::wstring& id)
{
	if (idExtMap.find(id) != idExtMap.end())
		return idExtMap[id];
	else
		return wstring();
}

std::wstring Client::GetImagePath() const
{
	return pathName;
}

void Client::RequestImageToServer(const std::wstring& wStr)
{
	string str = UnicodeToMultyByte(wStr);
	size_t dataSize = str.length();
	size_t packetSize = 1 + sizeof(size_t) + dataSize;
	vector<char> buf(packetSize, 0);
	buf[0] = static_cast<int>(DataType::IMAGE_REQUEST);
	CopyMemory(&buf[1], &dataSize, sizeof(size_t));
	CopyMemory(&buf[1 + sizeof(size_t)], &str[0], dataSize);
	Send(&buf[0], packetSize);
}

void Client::DisconnectToServer()
{
	if (mySocket != INVALID_SOCKET)
	{
		closesocket(mySocket);
		mySocket = INVALID_SOCKET;
	}

	if (recvThread)
	{
		if (recvThread->joinable())
			recvThread->join();

		delete recvThread;
		recvThread = nullptr;
	}
}

void Client::SendDisconnectedMessage()
{
	if (ownerDlg && ::IsWindow(ownerDlg->m_hWnd))
	{
		ownerDlg->SendMessage(MSG_DISCONNECTED, 0, 0);
		ownerDlg = nullptr;
	}
}
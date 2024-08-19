#pragma once
#pragma comment(lib, "ws2_32.lib")
#include "Enum/DataType.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <thread>
#include <string>
#include <vector>
#include <unordered_map>
#include "pch.h"

class CMFCClientDlg;

const unsigned int MSG_SIGNUP_RESULT	= WM_USER + 1;
const unsigned int MSG_LOGIN_RESULT		= WM_USER + 2;
const unsigned int MSG_UPDATE_ID_LIST	= WM_USER + 3;
const unsigned int MSG_RECV_TEXT		= WM_USER + 4;
const unsigned int MSG_SEND_TEXT		= WM_USER + 5;
const unsigned int MSG_DISCONNECTED		= WM_USER + 6;
const unsigned int MSG_REMOVE_ID		= WM_USER + 7;

class Client
{
public:

	Client() = default;

	void SetDlg(CMFCClientDlg* dlg);

	virtual ~Client();

	bool InitSocket();

	bool StartSocket();

	void ProcessSignUpResult(const char* buf);

	void ProcessLoginResult(const char* buf);

	void SendText(const DataType type, const std::wstring& wStr);

	std::string UnicodeToMultyByte(const std::wstring& wStr);

	bool Send(const char* buf, const size_t& size);

	inline bool GetIsAbleRecv() const { return isAbleRecv; }

	void ProcessPacket(const char* buf, const size_t& size, const DataType type);

	static std::wstring MultiByteToUnicode(const char* buf, const int& size);

	void SetMyId(const std::wstring& id) { myId = id; }

	inline std::wstring GetMyId() const { return myId; }

	void SendImage(const std::wstring& filePath, const std::wstring& fileExt, const DataType type);

	void ImageToBytes(std::vector<char>& buf, const std::wstring& filePath, const std::wstring& fileExt, size_t& size);

	std::wstring GetImageExt(const std::wstring& id);

	std::wstring GetImagePath() const;

	void RequestImageToServer(const std::wstring& wStr);

	void DisconnectToServer();

	void SendDisconnectedMessage();

private:

	std::thread* recvThread;

	std::thread* sendThread = nullptr;

	SOCKET mySocket;

	char recvBuf[PACKET_SIZE];

	bool bRun;

	CMFCClientDlg* ownerDlg;

	CWnd* m_pParent;

	bool isAbleRecv;

	std::wstring myId;

	std::wstring lastImageName;

	std::wstring pathName;

	std::unordered_map<std::wstring, std::wstring> idExtMap;

};

// MFC_ClientDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFC_Client.h"
#include "MFC_ClientDlg.h"
#include "ClientDlg_LoginTab.h"
#include "ClientDlg_SignUpTab.h"
#include "ClientDlg_Main.h"
#include "afxdialogex.h"
#include "Client.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMFCClientDlg::CMFCClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFC_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, tabCtrl);
}

BEGIN_MESSAGE_MAP(CMFCClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CMFCClientDlg::OnSelchangeTab)
	ON_MESSAGE(MSG_SIGNUP_RESULT, &CMFCClientDlg::OnMsgSignupResult)
	ON_MESSAGE(MSG_LOGIN_RESULT, &CMFCClientDlg::OnMsgLoginResult)
	ON_WM_SIZE()
	ON_MESSAGE(MSG_UPDATE_ID_LIST, &CMFCClientDlg::OnUpdateIdList)
	ON_MESSAGE(MSG_RECV_TEXT, &CMFCClientDlg::OnRecvText)
	ON_COMMAND(ID_MENU_FILE_IMPORT, &CMFCClientDlg::OnMenuFileImport)
	ON_COMMAND(ID_MENU_FILE_EXPORT, &CMFCClientDlg::OnMenuFileExport)
	ON_MESSAGE(MSG_SEND_TEXT, &CMFCClientDlg::OnMsgSendText)
	ON_COMMAND(ID_MENU_SETTING_PROFILE, &CMFCClientDlg::OnMenuSetProfileImage)
	ON_MESSAGE(MSG_DISCONNECTED, &CMFCClientDlg::OnMsgDisconnected)
	ON_MESSAGE(MSG_REMOVE_ID, &CMFCClientDlg::OnMsgRemoveId)
	ON_WM_GETMINMAXINFO()
	ON_MESSAGE(MSG_SHOW_IMAGE, &CMFCClientDlg::OnMsgShowImage)
END_MESSAGE_MAP()


// CMFCClientDlg 메시지 처리기

BOOL CMFCClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);

	DeactivateMenu();

	bool result = false;
	clientPtr = std::make_shared<Client>();
	if (clientPtr)
	{
		if (clientPtr->InitSocket())
		{
			clientPtr->SetDlg(this);
			result = clientPtr->StartSocket();
			if (!result) AfxMessageBox(L"실패");
		}
	}

	CRect rect;
	tabCtrl.GetWindowRect(rect);
	tabCtrl.InsertItem(0, L"로그인");
	tabCtrl.InsertItem(1, L"회원가입");
	tabCtrl.SetCurSel(0);

	loginTab = new ClientDlg_LoginTab(this);
	loginTab->Create(IDD_LOGIN, &tabCtrl);
	loginTab->SetClient(clientPtr);
	loginTab->MoveWindow(0, 20, rect.Width(), rect.Height() - 20);
	loginTab->ShowWindow(SW_SHOW);

	signUpTab = new ClientDlg_SignUpTab(this);
	signUpTab->Create(IDD_SIGNUP, &tabCtrl);
	signUpTab->SetClient(clientPtr);
	signUpTab->MoveWindow(0, 20, rect.Width(), rect.Height() - 20);
	signUpTab->ShowWindow(SW_HIDE);

	mainTab = new ClientDlg_Main(this);
	mainTab->Create(IDD_CLIENT_MAIN, &tabCtrl);
	mainTab->SetClient(clientPtr);

	return result;
}

void CMFCClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CMFCClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCClientDlg::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (pNMHDR->idFrom == IDC_TAB)
	{
		if (tabCtrl.GetCurSel() == 0)
		{
			loginTab->ShowWindow(SW_SHOW);
			signUpTab->ShowWindow(SW_HIDE);
		}
		else
		{
			loginTab->ShowWindow(SW_HIDE);
			signUpTab->ShowWindow(SW_SHOW);
		}
	}
}

afx_msg LRESULT CMFCClientDlg::OnMsgSignupResult(WPARAM wParam, LPARAM lParam)
{
	const char* buf = (char*)wParam;
	buf[0] ? AfxMessageBox(L"가입 성공") : AfxMessageBox(L"가입 실패");
	return 0;
}

afx_msg LRESULT CMFCClientDlg::OnMsgLoginResult(WPARAM wParam, LPARAM lParam)
{
	const char* buf = (char*)wParam;
	const bool result = buf[0];

	if (result)
	{
		AfxMessageBox(L"로그인 성공");
		std::wstring id = clientPtr->GetMyId() + L" (나)";
		mainTab->AddId(id);
		CreateMainWindow();
		ActivateMenu();
	}
	else
	{
		AfxMessageBox(L"로그인 실패");
	}

	return 0;
}

void CMFCClientDlg::CreateMainWindow()
{
	bMain = true;

	loginTab->ShowWindow(SW_HIDE);
	loginTab->DestroyWindow();
	signUpTab->ShowWindow(SW_HIDE);
	signUpTab->DestroyWindow();

	CRect wndRect;
	GetWindowRect(wndRect);
	MoveWindow(wndRect.left, wndRect.top, MAIN_X, MAIN_Y);

	CRect tabRect;
	tabCtrl.DeleteAllItems();
	tabCtrl.GetWindowRect(tabRect);
	tabCtrl.InsertItem(0, L"채팅");
	tabCtrl.SetCurSel(0);

	CRect mainTabRect;
	mainTab->GetWindowRect(mainTabRect);
	mainTab->ShowWindow(SW_SHOW);
	mainTab->MoveWindow(0, 20, mainTabRect.Width(), mainTabRect.Height());
}

void CMFCClientDlg::ActivateMenu()
{
	CMenu* menu = GetMenu();
	if (menu)
	{
		menu->EnableMenuItem(ID_MENU_SETTING_PROFILE, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_MENU_FILE_EXPORT, MF_BYCOMMAND | MF_ENABLED);
		menu->EnableMenuItem(ID_MENU_FILE_IMPORT, MF_BYCOMMAND | MF_ENABLED);
	}
}

void CMFCClientDlg::DeactivateMenu()
{
	CMenu* menu = GetMenu();
	if (menu)
	{
		menu->EnableMenuItem(ID_MENU_SETTING_PROFILE, MF_BYCOMMAND | MF_DISABLED);
		menu->EnableMenuItem(ID_MENU_FILE_EXPORT, MF_BYCOMMAND | MF_DISABLED);
		menu->EnableMenuItem(ID_MENU_FILE_IMPORT, MF_BYCOMMAND | MF_DISABLED);
	}
}

void CMFCClientDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (GetDlgItem(IDC_TAB) != NULL)
	{
		tabCtrl.SetWindowPos(NULL, 0, 0, cx - 100, cy - 100, SWP_NOMOVE);
		if (GetDlgItem(IDD_CLIENT_MAIN) != NULL)
		{
			mainTab->SetWindowPos(NULL, 0, 0, cx - 100, cy - 100, SWP_NOMOVE);
		}
		else if(GetDlgItem(IDD_LOGIN) != NULL && GetDlgItem(IDD_SIGNUP) != NULL)
		{
			loginTab->SetWindowPos(NULL, 0, 0, cx - 100, cy - 100, SWP_NOMOVE);
			signUpTab->SetWindowPos(NULL, 0, 0, cx - 100, cy - 100, SWP_NOMOVE);
		}
	}
}

afx_msg LRESULT CMFCClientDlg::OnUpdateIdList(WPARAM wParam, LPARAM lParam)
{
	const char* buf = (char*)wParam;
	const int quantity = buf[0];

	size_t pos = 1;
	
	for (int i = 0; i < quantity; i++)
	{
		size_t size = 0;
		CopyMemory(&size, &buf[pos], sizeof(size_t));
		std::wstring id = Client::MultiByteToUnicode(&buf[pos + sizeof(size_t)], size);
		pos += sizeof(size_t) + size;

		mainTab->AddId(id);
	}

	return 0;
}


afx_msg LRESULT CMFCClientDlg::OnRecvText(WPARAM wParam, LPARAM lParam)
{
	const char* buf = (char*)wParam;
	int size = (int)lParam;

	std::wstring wStr = Client::MultiByteToUnicode(&buf[0], size);

	mainTab->AddTextToListBox(wStr.c_str());
	chatLog += wStr + L"\n";

	return 0;
}


void CMFCClientDlg::OnMenuFileImport()
{
	CString fileFilter = _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, fileFilter);
	if (IDOK == dlg.DoModal())
	{
		CString pathName = dlg.GetPathName();

		std::wifstream file(pathName);
		file.imbue(std::locale("ko_KR.utf8"));

		if (file.is_open()) {
			std::wstring line;
			while (std::getline(file, line))
			{
				mainTab->AddTextToListBox(line.c_str());
				chatLog += line;
			}

			file.close();
		}
		else
		{
			AfxMessageBox(_T("파일 열기에 실패하였습니다."));
		}
	}
	else
	{
		AfxMessageBox(_T("파일 열기에 실패하였습니다."));
	}
}


void CMFCClientDlg::OnMenuFileExport()
{
	CString fileFilter = _T("텍스트 (*.txt)");
	CString defaultFilter = _T("txt");
	//세번째 매개변수 자동으로 지정될 이름 : 오늘 날짜 + 시간으로 설정되게
	CFileDialog dlg(FALSE, defaultFilter, NULL, OFN_OVERWRITEPROMPT, fileFilter);
	if (IDOK == dlg.DoModal())
	{
		CString pathName = dlg.GetPathName();

		std::wofstream file(pathName, std::ios::out | std::ios::binary);
		if (file.is_open()) {
			
			file.imbue(std::locale("ko_KR.utf8"));
			file << chatLog << std::endl;
			file.close();
		}
	}
} 


afx_msg LRESULT CMFCClientDlg::OnMsgSendText(WPARAM wParam, LPARAM lParam)
{
	const wchar_t* wchPtr = (wchar_t*)wParam;
	std::wstring wStr(wchPtr);

	chatLog += L"[" + clientPtr->GetMyId() + L"] : " + wStr + L"\n";
	return 0;
}


void CMFCClientDlg::OnMenuSetProfileImage()
{
	CString fileFilter = _T(" Image Files (*.bmp;*.jpg;*.png) |*.bmp;*.jpg;*.png| All Files(*.*)|*.*||");

	CFileDialog dlg(true, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, fileFilter, this);
	if (dlg.DoModal() == IDOK)
	{
		CString file_path = dlg.GetPathName();
		CString file_ext = dlg.GetFileExt();
		file_ext = file_ext.MakeUpper();

		CImage image;
		image.Load(file_path);
		mainTab->SetProfileImage(image);

		clientPtr->SendImage(file_path.operator LPCWSTR(), file_ext.operator LPCWSTR(), DataType::IMAGE);
	}
}


afx_msg LRESULT CMFCClientDlg::OnMsgDisconnected(WPARAM wParam, LPARAM lParam)
{
	AfxMessageBox(L"서버와 연결이 종료되었습니다.");
	clientPtr.reset();
	SendMessage(WM_CLOSE);
	return 0;
}


afx_msg LRESULT CMFCClientDlg::OnMsgRemoveId(WPARAM wParam, LPARAM lParam)
{
	const wchar_t* wchPtr = (wchar_t*)wParam;
	std::wstring idToRemove(wchPtr);

	mainTab->RemoveId(idToRemove);

	return 0;
}


void CMFCClientDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (!bMain)
	{
		lpMMI->ptMinTrackSize = CPoint(LOGIN_SIZE, LOGIN_SIZE);
		lpMMI->ptMaxTrackSize = CPoint(LOGIN_SIZE, LOGIN_SIZE);
	}
	else
	{
		lpMMI->ptMinTrackSize = CPoint(MAIN_X, MAIN_Y);
		lpMMI->ptMaxTrackSize = CPoint(MAIN_X, MAIN_Y);
	}

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


afx_msg LRESULT CMFCClientDlg::OnMsgShowImage(WPARAM wParam, LPARAM lParam)
{
	const wchar_t* wChar = (wchar_t*)wParam;
	std::wstring imageName(wChar);

	std::wstring filePath = clientPtr->GetImagePath() + L"\\" + clientPtr->GetMyId() + L"\\" + imageName;
	mainTab->ShowImage(filePath);

	return 0;
}

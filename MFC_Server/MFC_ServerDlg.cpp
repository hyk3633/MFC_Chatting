
// MFC_ServerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFC_Server.h"
#include "MFC_ServerDlg.h"
#include "afxdialogex.h"
#include "Server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCServerDlg 대화 상자



CMFCServerDlg::CMFCServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFC_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CLIENTS, listCtrl_Clients);
}

BEGIN_MESSAGE_MAP(CMFCServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MSG_CLIENT_LOGIN, &CMFCServerDlg::OnMsgClientLogin)
	ON_MESSAGE(MSG_REMOVE_ID, &CMFCServerDlg::OnMsgRemoveId)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CMFCServerDlg 메시지 처리기

BOOL CMFCServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	CRect rect;
	listCtrl_Clients.GetClientRect(&rect);
	listCtrl_Clients.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	listCtrl_Clients.InsertColumn(0, _T("아이디"), LVCFMT_LEFT, rect.Width() * 0.5);

	serverPtr = std::make_shared<Server>(this);
	if (serverPtr->InitializeServer())
	{
		serverPtr->StartServer();
	}

	return TRUE;
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMFCServerDlg::OnPaint()
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

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

afx_msg LRESULT CMFCServerDlg::OnMsgClientLogin(WPARAM wParam, LPARAM lParam)
{
	std::string id("", lParam);
	CopyMemory(&id[0], (char*)wParam, lParam);

	CString cStr(id.c_str());

	listCtrl_Clients.InsertItem(listCtrl_Clients.GetItemCount(), cStr);

	return LRESULT();
}


afx_msg LRESULT CMFCServerDlg::OnMsgRemoveId(WPARAM wParam, LPARAM lParam)
{
	const wchar_t* wchPtr = (wchar_t*)wParam;
	std::wstring idToRemove(wchPtr);

	for (int i = 0; i < listCtrl_Clients.GetItemCount(); i++)
	{
		CString id = listCtrl_Clients.GetItemText(i, 0);

		if (idToRemove == id.operator LPCWSTR())
			listCtrl_Clients.DeleteItem(i);
	}

	return 0;
}


void CMFCServerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize = CPoint(581, 434);
	lpMMI->ptMaxTrackSize = CPoint(581, 434);
	CDialogEx::OnGetMinMaxInfo(lpMMI);
}

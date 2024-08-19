// ClientDlg_LoginTab.cpp: 구현 파일
//

#include "pch.h"
#include "MFC_Client.h"
#include "afxdialogex.h"
#include "ClientDlg_LoginTab.h"
#include "Client.h"

// ClientDlg_LoginTab 대화 상자

IMPLEMENT_DYNAMIC(ClientDlg_LoginTab, CDialogEx)

ClientDlg_LoginTab::ClientDlg_LoginTab(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN, pParent)
{

}

ClientDlg_LoginTab::~ClientDlg_LoginTab()
{
}

void ClientDlg_LoginTab::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ClientDlg_LoginTab, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &ClientDlg_LoginTab::OnBnClickedButtonLogin)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// ClientDlg_LoginTab 메시지 처리기


BOOL ClientDlg_LoginTab::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}


void ClientDlg_LoginTab::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize = CPoint(400, 400);
	lpMMI->ptMaxTrackSize = CPoint(400, 400);

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void ClientDlg_LoginTab::OnBnClickedButtonLogin()
{
	CString id, pw;
	GetDlgItemText(IDC_EDIT_LOGIN_ID, id);
	GetDlgItemText(IDC_EDIT_LOGIN_PW, pw);

	if (auto tempClient = clientPtr.lock())
	{
		std::wstring wStr = std::wstring(id.operator LPCWSTR()) + L" " + std::wstring(pw.operator LPCWSTR());
		tempClient->SetMyId(id.operator LPCWSTR());
		tempClient->SendText(DataType::LOGIN_INFO, wStr);
	}
}

void ClientDlg_LoginTab::SetClient(std::weak_ptr<Client> client)
{
	clientPtr = client;
}


void ClientDlg_LoginTab::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

}

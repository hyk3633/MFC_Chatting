// ClientDlg_SignUpTab.cpp: 구현 파일
//

#include "pch.h"
#include "MFC_Client.h"
#include "afxdialogex.h"
#include "ClientDlg_SignUpTab.h"
#include "Client.h"

// ClientDlg_SignUpTab 대화 상자

IMPLEMENT_DYNAMIC(ClientDlg_SignUpTab, CDialogEx)

ClientDlg_SignUpTab::ClientDlg_SignUpTab(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SIGNUP, pParent)
{

}

ClientDlg_SignUpTab::~ClientDlg_SignUpTab()
{
}

void ClientDlg_SignUpTab::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ClientDlg_SignUpTab, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_BUTTON_SIGNUP, &ClientDlg_SignUpTab::OnBnClickedButtonSignup)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// ClientDlg_SignUpTab 메시지 처리기


void ClientDlg_SignUpTab::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize = CPoint(360, 400);
	lpMMI->ptMaxTrackSize = CPoint(360, 400);

	CDialogEx::OnGetMinMaxInfo(lpMMI);
}


void ClientDlg_SignUpTab::OnBnClickedButtonSignup()
{
	CString id, pw;
	GetDlgItemText(IDC_EDIT_SIGNUP_ID, id);
	GetDlgItemText(IDC_EDIT_SIGNUP_PW, pw);
	
	if (auto tempClient = clientPtr.lock())
	{
		std::wstring wStr = std::wstring(id.operator LPCWSTR()) + L" " + std::wstring(pw.operator LPCWSTR());
		tempClient->SendText(DataType::SIGNUP_INFO, wStr);
	}
}

void ClientDlg_SignUpTab::SetClient(std::weak_ptr<Client> client)
{
	clientPtr = client;
}


void ClientDlg_SignUpTab::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
}

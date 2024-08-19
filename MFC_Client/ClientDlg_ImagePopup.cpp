// ClientDlg_ImagePopup.cpp: 구현 파일
//

#include "pch.h"
#include "MFC_Client.h"
#include "afxdialogex.h"
#include "ClientDlg_ImagePopup.h"


// ClientDlg_ImagePopup 대화 상자

IMPLEMENT_DYNAMIC(ClientDlg_ImagePopup, CDialogEx)

ClientDlg_ImagePopup::ClientDlg_ImagePopup() : CDialogEx(IDD_CLIENT_IMGPOPUP)
{

}

ClientDlg_ImagePopup::~ClientDlg_ImagePopup()
{
}

void ClientDlg_ImagePopup::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PICTURE, pictureCtrl_Profile);
}

BEGIN_MESSAGE_MAP(ClientDlg_ImagePopup, CDialogEx)
END_MESSAGE_MAP()


// ClientDlg_ImagePopup 메시지 처리기


BOOL ClientDlg_ImagePopup::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}

void ClientDlg_ImagePopup::SetImage(const std::wstring& filePath)
{
	if (filePath.length() == 0)
		return;

	CImage image;
	image.Load(filePath.c_str());

	CRect rect;
	pictureCtrl_Profile.GetWindowRect(rect);

	CDC* dc;
	dc = pictureCtrl_Profile.GetDC();

	image.StretchBlt(dc->m_hDC, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
}

// ClientDlg_Main.cpp: 구현 파일
//

#include "pch.h"
#include "MFC_Client.h"
#include "afxdialogex.h"
#include "ClientDlg_Main.h"
#include "ClientDlg_ImagePopup.h"
#include "Client.h"
#include <vector>
#include <algorithm>

// ClientDlg_Main 대화 상자

IMPLEMENT_DYNAMIC(ClientDlg_Main, CDialogEx)

ClientDlg_Main::ClientDlg_Main(CWnd* pParent /*=nullptr*/)
	: parentDlg(pParent), CDialogEx(IDD_CLIENT_MAIN, pParent)
	, textBoxText(_T(""))
{

}

ClientDlg_Main::~ClientDlg_Main()
{
}

void ClientDlg_Main::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CLIENTS, listCtrl_Clients);
	DDX_Control(pDX, IDC_LIST_CHAT, listBox_Chat);
	DDX_Control(pDX, IDC_EDIT_TEXT, editCtrl_Text);
	DDX_Control(pDX, IDC_BUTTON_SEND, btn_Send);
	DDX_Text(pDX, IDC_EDIT_TEXT, textBoxText);
	DDX_Control(pDX, IDC_PROFILE_IMAGE, pictureCtrl_Profile);
}

void ClientDlg_Main::SetClient(std::weak_ptr<Client> client)
{
	clientPtr = std::weak_ptr<Client>(client);
}

void ClientDlg_Main::AddId(const std::wstring& id)
{
	CString cStr(id.c_str());
	listCtrl_Clients.InsertItem(listCtrl_Clients.GetItemCount(), cStr);
}

void ClientDlg_Main::AddTextToListBox(const CString& text)
{
	listBox_Chat.InsertString(-1, text);
}

BEGIN_MESSAGE_MAP(ClientDlg_Main, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &ClientDlg_Main::OnBnClickedButtonSend)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CLIENTS, &ClientDlg_Main::OnClickListClients)
END_MESSAGE_MAP()

// ClientDlg_Main 메시지 처리기

BOOL ClientDlg_Main::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rect;
	listCtrl_Clients.GetClientRect(&rect);
	listCtrl_Clients.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	listCtrl_Clients.InsertColumn(0, _T("아이디"), LVCFMT_LEFT, rect.Width() * 0.5);

	return TRUE;
}

void ClientDlg_Main::SetProfileImage(CImage& image)
{
	CRect rect;
	pictureCtrl_Profile.GetWindowRect(rect);

	CDC* dc;
	dc = pictureCtrl_Profile.GetDC();

	image.StretchBlt(dc->m_hDC, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
}

void ClientDlg_Main::RemoveId(std::wstring& idToRemove)
{
	for (int i = 0; i < listCtrl_Clients.GetItemCount(); i++)
	{
		CString id = listCtrl_Clients.GetItemText(i, 0);

		if (idToRemove == id.operator LPCWSTR())
			listCtrl_Clients.DeleteItem(i);
	}
}


void ClientDlg_Main::OnBnClickedButtonSend()
{
	if (auto shPtr = clientPtr.lock())
	{
		UpdateData(true);
		if (textBoxText.GetLength() > 0)
		{
			shPtr->SendText(DataType::TEXT, textBoxText.operator LPCWSTR());

			std::wstring wStr = textBoxText.operator LPCWSTR();
			const wchar_t* wchPtr = &wStr[0];
			parentDlg->SendMessage(MSG_SEND_TEXT, (WPARAM)&wchPtr[0], (LPARAM)wStr.length());

			CString text = L"[나] : " + textBoxText;
			listBox_Chat.InsertString(-1, text);
			textBoxText = L"";

			UpdateData(false);
		}
	}
}


void ClientDlg_Main::OnClickListClients(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	int idx = pNMListView->iItem;
	
	CString cStr = listCtrl_Clients.GetItemText(idx, 0);
	std::wstring id = cStr.operator LPCWSTR();

	if (id.find(L"나") != std::wstring::npos)
		return;

	if (auto shPtr = clientPtr.lock())
	{
		std::wstring imageExt = shPtr->GetImageExt(id);
		if (imageExt.length() == 0)
		{
			shPtr->RequestImageToServer(id);
		}
		else
		{
			std::wstring filePath = shPtr->GetImagePath() + L"\\" + shPtr->GetMyId() + L"\\" + id + L"." + imageExt;
			ShowImage(filePath);
		}
	}
}

void ClientDlg_Main::ShowImage(const std::wstring& filePath)
{
	ClientDlg_ImagePopup* imagePopup = new ClientDlg_ImagePopup();
	imagePopup->Create(IDD_CLIENT_IMGPOPUP);
	imagePopup->ShowWindow(SW_SHOW);
	imagePopup->SetImage(filePath);
}

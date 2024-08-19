#pragma once
#include "afxdialogex.h"
#include <memory>
#include <string>

class Client;

// ClientDlg_Main 대화 상자

class ClientDlg_Main : public CDialogEx
{
	DECLARE_DYNAMIC(ClientDlg_Main)

public:
	ClientDlg_Main(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~ClientDlg_Main();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_MAIN };
#endif

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:

	void SetClient(std::weak_ptr<Client> client);

	void AddId(const std::wstring& id);

	void AddTextToListBox(const CString& text);

	afx_msg void OnBnClickedButtonSend();

	virtual BOOL OnInitDialog();

	void SetProfileImage(CImage& image);

	void RemoveId(std::wstring& idToRemove);

private:

	std::weak_ptr<Client> clientPtr;

	CWnd* parentDlg;

	CListCtrl listCtrl_Clients;

	CListBox listBox_Chat;
	CEdit editCtrl_Text;
	CButton btn_Send;

	CString textBoxText;
	
	CStatic pictureCtrl_Profile;

public:

	afx_msg void OnClickListClients(NMHDR* pNMHDR, LRESULT* pResult);
};

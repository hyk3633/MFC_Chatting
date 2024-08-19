#pragma once
#include "afxdialogex.h"
#include <memory>

class Client;

// ClientDlg_LoginTab 대화 상자

class ClientDlg_LoginTab : public CDialogEx
{
	DECLARE_DYNAMIC(ClientDlg_LoginTab)

public:
	ClientDlg_LoginTab(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~ClientDlg_LoginTab();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedButtonLogin();

	void SetClient(std::weak_ptr<Client> client);

private:

	std::weak_ptr<Client> clientPtr;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

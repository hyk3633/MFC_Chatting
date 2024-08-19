#pragma once
#include "afxdialogex.h"
#include <memory>

class Client;

// ClientDlg_SignUpTab 대화 상자

class ClientDlg_SignUpTab : public CDialogEx
{
	DECLARE_DYNAMIC(ClientDlg_SignUpTab)

public:
	ClientDlg_SignUpTab(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~ClientDlg_SignUpTab();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIGNUP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedButtonSignup();

	void SetClient(std::weak_ptr<Client> client);

private:

	std::weak_ptr<Client> clientPtr;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


// MFC_ClientDlg.h: 헤더 파일
//

#pragma once
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <codecvt>
#include <locale>

class Client;
class ClientDlg_LoginTab;
class ClientDlg_SignUpTab;
class ClientDlg_Main;

#define LOGIN_SIZE 522

#define MAIN_X 785
#define MAIN_Y 600

// CMFCClientDlg 대화 상자
class CMFCClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CMFCClientDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:

	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMenuFileImport();
	afx_msg void OnMenuFileExport();
	afx_msg void OnMenuSetProfileImage();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

protected:

	afx_msg LRESULT OnMsgSignupResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgLoginResult(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateIdList(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRecvText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgSendText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgDisconnected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgRemoveId(WPARAM wParam, LPARAM lParam);

	void CreateMainWindow();

	void ActivateMenu();

	void DeactivateMenu();

private:

	std::shared_ptr<Client> clientPtr;

	std::wstring chatLog;

	CTabCtrl tabCtrl;

	ClientDlg_LoginTab* loginTab;
	ClientDlg_SignUpTab* signUpTab;
	ClientDlg_Main* mainTab;

	bool bMain = false;
	
};

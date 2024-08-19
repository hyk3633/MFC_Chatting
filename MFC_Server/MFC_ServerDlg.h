
// MFC_ServerDlg.h: 헤더 파일
//

#pragma once
#include <memory>

class Server;

// CMFCServerDlg 대화 상자
class CMFCServerDlg : public CDialogEx
{
// 생성입니다.
public:
	CMFCServerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:

	afx_msg LRESULT OnMsgClientLogin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgRemoveId(WPARAM wParam, LPARAM lParam);

public:

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

protected:

	HICON m_hIcon;

private:

	CListCtrl listCtrl_Clients;

	std::shared_ptr<Server> serverPtr;

};

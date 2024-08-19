
// MFC_Client.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "MFC_Client.h"
#include "MFC_ClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFCClientApp

BEGIN_MESSAGE_MAP(CMFCClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMFCClientApp 생성

CMFCClientApp::CMFCClientApp()
{
	// 다시 시작 관리자 지원
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CMFCClientApp 개체입니다.

CMFCClientApp theApp;


// CMFCClientApp 초기화

BOOL CMFCClientApp::InitInstance()
{
	CWinApp::InitInstance();

	CMFCClientDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}


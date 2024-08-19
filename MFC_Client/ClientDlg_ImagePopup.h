#pragma once
#include "afxdialogex.h"
#include <string>

// ClientDlg_ImagePopup 대화 상자

class ClientDlg_ImagePopup : public CDialogEx
{
	DECLARE_DYNAMIC(ClientDlg_ImagePopup)

public:

	ClientDlg_ImagePopup();

	virtual ~ClientDlg_ImagePopup();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_IMGPOPUP };
#endif

	virtual BOOL OnInitDialog();

	void SetImage(const std::wstring& filePath);

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

private:

	CStatic pictureCtrl_Profile;

	std::wstring pathName;
	
};

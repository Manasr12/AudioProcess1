#pragma once
#include "afxdialogex.h"


// textdialog dialog

class textdialog : public CDialogEx
{
	DECLARE_DYNAMIC(textdialog)

public:
	textdialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~textdialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEXTFILTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

#pragma once
#include "afxdialogex.h"


// filterdialog dialog

class filterdialog : public CDialogEx
{
	DECLARE_DYNAMIC(filterdialog)

public:
	filterdialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~filterdialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double m_frequency = 0;
	double m_bandwidth = 0;
};

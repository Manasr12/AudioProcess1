// filterdialog.cpp : implementation file
//

#include "pch.h"
#include "AudioProcess.h"
#include "afxdialogex.h"
#include "filterdialog.h"


// filterdialog dialog

IMPLEMENT_DYNAMIC(filterdialog, CDialogEx)

filterdialog::filterdialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_frequency(0)
	, m_bandwidth(0)
{

}

filterdialog::~filterdialog()
{
}

void filterdialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FREQUENCY, m_frequency);
	DDX_Text(pDX, IDC_BANDWIDTH, m_bandwidth);
}


BEGIN_MESSAGE_MAP(filterdialog, CDialogEx)
END_MESSAGE_MAP()


// filterdialog message handlers

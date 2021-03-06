// CMyNewDlg.cpp: 实现文件
//
#include "stdafx.h"
#include "AbusLabeler.h"
#include "TumorInfoDlg.h"
#include "afxdialogex.h"


// CMyNewDlg 对话框

IMPLEMENT_DYNAMIC(CTumorInfoDlg, CDialog)

CTumorInfoDlg::CTumorInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_TUMOR_INFO_DLG, pParent)
{

}

CTumorInfoDlg::~CTumorInfoDlg()
{
}

void CTumorInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_RADIO1, m_radiobtgroup1);
	DDX_Radio(pDX, IDC_RADIOB1, m_info.birads);
	DDX_Radio(pDX, IDC_RADIOM1, m_info.morph);
	DDX_Radio(pDX, IDC_RADIODE1, m_info.derection);
	DDX_Radio(pDX, IDC_RADIO12, m_info.inner_echo);
	DDX_Radio(pDX, IDC_RADIO18, m_info.back_echo);
	DDX_Check(pDX, IDC_CHECK6, m_info.boder_type[0]);
	DDX_Check(pDX, IDC_CHECK2, m_info.boder_type[1]);
	DDX_Check(pDX, IDC_CHECK3, m_info.boder_type[2]);
	DDX_Check(pDX, IDC_CHECK4, m_info.boder_type[3]);
	DDX_Check(pDX, IDC_CHECK5, m_info.boder_type[4]);
	DDX_Check(pDX, IDC_CHECK23, m_info.boder_type[5]);
	DDX_Check(pDX, IDC_CHECK7, m_info.cacif_type[0]);
	DDX_Check(pDX, IDC_CHECK8, m_info.cacif_type[1]);
	DDX_Check(pDX, IDC_CHECK9, m_info.cacif_type[2]);
	DDX_Check(pDX, IDC_CHECK10, m_info.con_phen.type[0]);
	DDX_Check(pDX, IDC_CHECK11, m_info.con_phen.type[1]);
	DDX_Check(pDX, IDC_CHECK12, m_info.con_phen.type[2]);
	DDX_Check(pDX, IDC_CHECK13, m_info.con_phen.type[3]);
	DDX_Check(pDX, IDC_CHECK14, m_info.con_phen.type[4]);
	DDX_Radio(pDX, IDC_RADIO25, m_info.con_phen.boold_type);
	DDX_Check(pDX, IDC_CHECK15, m_info.other_case.othercase[0]);
	DDX_Check(pDX, IDC_CHECK16, m_info.other_case.othercase[1]);
	DDX_Check(pDX, IDC_CHECK17, m_info.other_case.othercase[2]);
	DDX_Check(pDX, IDC_CHECK24, m_info.other_case.othercase[3]);
	DDX_Check(pDX, IDC_CHECK25, m_info.other_case.othercase[4]);
	DDX_Check(pDX, IDC_CHECK26, m_info.other_case.othercase[5]);
	DDX_Check(pDX, IDC_CHECK27, m_info.other_case.othercase[6]);
	DDX_Check(pDX, IDC_CHECK28, m_info.other_case.othercase[7]);
	DDX_Check(pDX, IDC_CHECK29, m_info.other_case.othercase[8]);
	DDX_Radio(pDX, IDC_RADIO33, m_info.other_case.type);
	

}


BEGIN_MESSAGE_MAP(CTumorInfoDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTumorInfoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIOB1, &CTumorInfoDlg::OnBnClickedRadioB1)
	ON_BN_CLICKED(IDC_RADIOB2, &CTumorInfoDlg::OnBnClickedRadioB1)
	ON_BN_CLICKED(IDC_RADIOB3, &CTumorInfoDlg::OnBnClickedRadioB1)
	ON_BN_CLICKED(IDC_RADIOB4, &CTumorInfoDlg::OnBnClickedRadioB1)
	ON_BN_CLICKED(IDC_RADIOB5, &CTumorInfoDlg::OnBnClickedRadioB1)
	ON_BN_CLICKED(IDC_RADIOB6, &CTumorInfoDlg::OnBnClickedRadioB1)
	ON_BN_CLICKED(IDC_RADIOB7, &CTumorInfoDlg::OnBnClickedRadioB1)

	ON_BN_CLICKED(IDC_RADIOM1, &CTumorInfoDlg::OnBnClickedRadiom1)
	ON_BN_CLICKED(IDC_RADIOM2, &CTumorInfoDlg::OnBnClickedRadiom1)
	ON_BN_CLICKED(IDC_RADIOM3, &CTumorInfoDlg::OnBnClickedRadiom1)
END_MESSAGE_MAP()

void CTumorInfoDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	CDialog::OnOK();
}

void CTumorInfoDlg::OnBnClickedRadioB1()
{
	// TODO: 在此添加控件通知处理程序代码
	
	//int index = m_birads;
}


void CTumorInfoDlg::OnBnClickedRadiom1()
{
	// TODO: 在此添加控件通知处理程序代码
	
	//int index = m_morph;
}



BOOL CTumorInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


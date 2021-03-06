
// AbusLabelerDlg.cpp: 实现文件
//
#include "stdafx.h"
#include "AbusLabeler.h"
#include "AbusLabelerDlg.h"
#include "afxdialogex.h"
#include <opencv2/opencv.hpp>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkNIFTIImageWriter.h>
#include "ABUSAlg.h"
#include"omp.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static UINT BASED_CODE indicators[] = {
	ID_INDICATOR_TIME,
	ID_INDICATOR_POS,
	ID_INDICATOR_PIXEL,
	100001,
};

// CAbusLabelerDlg 对话框
UINT CalcDSCThreadFunc(LPVOID pParm);
UINT ReDSCLabelAndSaveThreadFunc(LPVOID pParm);

CAbusLabelerDlg::CAbusLabelerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ABUSLABELER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAbusLabelerDlg::InitStausBar()
{
	m_wndStatusBar.Create(this);
	m_wndStatusBar.SetIndicators(indicators, 4);
	CRect rect;
	GetClientRect(&rect);
	m_wndStatusBar.SetPaneInfo(0, ID_INDICATOR_TIME, SBPS_STRETCH, 0);
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_STRETCH, 0);
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_PIXEL, SBPS_STRETCH, 0);
	m_wndStatusBar.SetPaneInfo(3, 100001, SBPS_STRETCH, 0);
	m_wndStatusBar.GetStatusBarCtrl().SetBkColor(RGB(250, 180, 56));
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, AFX_IDW_CONTROLBAR_FIRST);
	RECT rect1;
	m_wndStatusBar.GetItemRect(3, &rect1);			//获取状态栏所需要区的窗口矩形
	m_ProgressState.SetParent(&m_wndStatusBar);		//设置状态栏为父
	m_ProgressState.MoveWindow(&rect1);
	m_ProgressState.ShowWindow(SW_SHOW);
	m_ProgressState.SetRange(0, 100);
	m_ProgressState.SetPos(0);
}

void CAbusLabelerDlg::InitTimer()
{
	SetTimer(1, 1000, NULL);
}

void CAbusLabelerDlg::Init3View()
{
	m_AxialView.CreatMemDC();
	m_AxialView.SetViewID(1);
	m_AxialView.m_alpha = 0;
	m_SagittalView.CreatMemDC();
	m_SagittalView.SetViewID(2);
	m_SagittalView.m_alpha = 0;
	m_CoronalView.CreatMemDC();
	m_CoronalView.SetViewID(3);
	m_CoronalView.m_alpha = 0;
	m_view_index[0] = 0;
	m_view_index[1] = 0;
	m_view_index[2] = 0;
	/*
	CRect rect;
	GetDlgItem(IDC_STATIC_VIEW1)->GetClientRect(&rect);
	cv::namedWindow("view1", cv::WINDOW_AUTOSIZE);
	cv::resizeWindow("view1", cv::Size(rect.Width(), rect.Height()));
	HWND hWnd1 = (HWND)cvGetWindowHandle("view1");
	HWND hParent1 = ::GetParent(hWnd1);
	::SetParent(hWnd1, GetDlgItem(IDC_STATIC_VIEW1)->m_hWnd);
	::ShowWindow(hParent1, SW_HIDE);
	*/
}

void CAbusLabelerDlg::InitVolumeInfoList()
{
	CRect rect;
	int nCols = 2;
	m_volumeInfoList.GetClientRect(&rect);
	m_volumeInfoList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_volumeInfoList.InsertColumn(0, _T("文件信息"), LVCFMT_CENTER, rect.Width() / nCols, 0);
	m_volumeInfoList.InsertColumn(1, _T("字段值"),   LVCFMT_CENTER, rect.Width() / nCols, 1);
}


void CAbusLabelerDlg::InitTumorInfoList()
{
	CRect rect;
	int nCols = 11;
	m_tumorInfoList.GetClientRect(&rect);
	m_tumorInfoList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_tumorInfoList.InsertColumn(0, _T("Tumor编号"), LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(1, _T("BI-RADS"),   LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(2, _T("形态"),      LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(3, _T("生长方向"),  LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(4, _T("边界"),      LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(5, _T("内部回声"),  LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(6, _T("后方回声"),  LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(7, _T("钙化"),      LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(8, _T("伴随现象"),  LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(9, _T("其他"),      LVCFMT_CENTER, rect.Width() / nCols);
	m_tumorInfoList.InsertColumn(10, _T("伪彩色"),   LVCFMT_CENTER, rect.Width() / nCols);
}

void CAbusLabelerDlg::InitTumorTypeString()
{
	m_strBirads = { "2", "3", "4A","4B", "4C","5","6" };
	m_strMorph = { "椭圆","圆形", "不规则形" };
	m_strDirection = { "平行","不平行" };
	m_strBorder = { "平滑清晰分界", "边界模糊","成角边界","细分叶边界", "毛刺样边界", "强回声晕" };
	m_strInnerEcho = { "无回声", "高回声", "复杂性囊肿和实性成分", "低回声", "等回声", "杂乱回声" };
	m_strBackEcho = { "无后方回声", "增强", "声影", "混合类型" };
	m_strCacif = { "肿块内钙化", "肿块外钙化", "导管内钙化" };
	m_strConPhen = { "结构扭变", "导管改变", "皮肤增厚", "皮肤收缩", "水肿", "无血液", "内部血液信号", "边缘血液信号" };
	m_strOtherCase = { "单纯囊肿", "簇状微囊","复杂囊肿", "皮肤或皮肤上肿块", "异物含假体",
		"淋巴结-乳房内淋巴结", "腋窝淋巴结", "术后积液", "脂肪坏死","动脉急性或假性动脉瘤", "胸壁浅表血栓性静脉炎" };
}

void CAbusLabelerDlg::InsertTumor(int tumor_id, TumorInfo info)
{
	CString strTmp;
	strTmp.Format("Tumor ID : %d", tumor_id);
	int count = m_tumorInfoList.GetItemCount();
	if (tumor_id >= count)
	{
		m_tumorInfoList.InsertItem(tumor_id, strTmp);//添加第一个学生数据
	}

	m_tumorInfoList.SetItemText(tumor_id, 1, m_strBirads[info.birads].c_str());
	m_tumorInfoList.SetItemText(tumor_id, 2, m_strMorph[info.morph].c_str());
	m_tumorInfoList.SetItemText(tumor_id, 3, m_strDirection[info.derection].c_str());

	// 插入边界
	string str_boder = "";
	for (int i = 0; i < m_strBorder.size(); i++)
	{
		if (info.boder_type[i])
		{
			str_boder += m_strBorder[i] + " | ";
		}
	}

	m_tumorInfoList.SetItemText(tumor_id, 4, str_boder.c_str());
	m_tumorInfoList.SetItemText(tumor_id, 5, m_strInnerEcho[info.inner_echo].c_str());
	m_tumorInfoList.SetItemText(tumor_id, 6, m_strBackEcho[info.back_echo].c_str());
	string str_cacif = "";
	for (int i = 0; i < m_strCacif.size(); i++)
	{
		if (info.cacif_type[i])
		{
			str_cacif += m_strCacif[i] + " | ";
		}
	}
	m_tumorInfoList.SetItemText(tumor_id, 7, str_cacif.c_str());

	string str_con_phen = "";
	for (int i = 0; i < 5; i++)
	{
		if (info.con_phen.type[i])
		{
			str_con_phen += m_strConPhen[i] + " | ";
		}
	}
	if (info.con_phen.boold_type)
	{
		str_con_phen += m_strConPhen[info.con_phen.boold_type -1 + 5];
	}
	m_tumorInfoList.SetItemText(tumor_id, 8, str_con_phen.c_str());
	string str_other = "";
	for (int i = 0; i < 9; i++)
	{
		if (info.other_case.othercase[i])
		{
			str_other += m_strOtherCase[i] + " | ";
		}
	}

	if (info.other_case.type)
	{
		// 正常情况不写入报告
		str_other += m_strOtherCase[info.other_case.type - 1 + 9];
	}
	m_tumorInfoList.SetItemText(tumor_id, 9, str_other.c_str());
	CString str_value;
	str_value.Format(_T("%d"), info.pesudo_color);
	m_tumorInfoList.SetItemText(tumor_id, 10, str_value);
	UpdateData(FALSE);
}

void CAbusLabelerDlg::InitButtun()
{
	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	CButton * pBtn;
	pBtn = (CButton *)GetDlgItem(IDC_BUTTON1);
	pBtn->SetIcon(hIcon);
	pBtn = (CButton *)GetDlgItem(IDC_BUTTON2);
	pBtn->SetIcon(hIcon);
	pBtn = (CButton *)GetDlgItem(IDC_BUTTON3);
	pBtn->SetIcon(hIcon);

	hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
	pBtn = (CButton *)GetDlgItem(IDC_BUTTON_CA1);
	pBtn->SetIcon(hIcon);
	pBtn = (CButton *)GetDlgItem(IDC_BUTTON_CA2);
	pBtn->SetIcon(hIcon);
	pBtn = (CButton *)GetDlgItem(IDC_BUTTON_CA3);
	pBtn->SetIcon(hIcon);
}

void CAbusLabelerDlg::InitSliderBar()
{
	CSliderCtrl   *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	pSlidCtrl->SetRange(0, 1);
	pSlidCtrl->SetTicFreq(1);
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
	pSlidCtrl->SetRange(0, 1);
	pSlidCtrl->SetTicFreq(1);
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
	pSlidCtrl->SetRange(0, 1);
	pSlidCtrl->SetTicFreq(1);
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_FUSION);
	pSlidCtrl->SetRange(0, 100);
	pSlidCtrl->SetTicFreq(1);
}

void CAbusLabelerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_INFO, m_volumeInfoList);
	DDX_Control(pDX, IDC_LIST_TUMOR, m_tumorInfoList);
	DDX_Radio(pDX, IDC_RADIOC0, m_nLineType);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressState);
	DDX_Control(pDX, IDC_STATIC_VIEW1, m_AxialView);
	DDX_Control(pDX, IDC_STATIC_VIEW2, m_SagittalView);
	DDX_Control(pDX, IDC_STATIC_VIEW3, m_CoronalView);
	//DDX_Control(pDX, IDC_STATIC_VIEW4, m_3DView);
	
}

BEGIN_MESSAGE_MAP(CAbusLabelerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_COMMAND(ID_OPEN_FILE, &CAbusLabelerDlg::OnOpenFile)
	ON_MESSAGE(WM_SHOW_POS, &CAbusLabelerDlg::OnShowPos)
	ON_MESSAGE(WM_DSC_PROGRESS, &CAbusLabelerDlg::OnUpdateDSCProgress)
	ON_MESSAGE(WM_UPDATE_VIEW, &CAbusLabelerDlg::OnUpdate2Views)
	ON_MESSAGE(WM_NEXT_VIEW, &CAbusLabelerDlg::OnNextView)
	ON_MESSAGE(WM_WRITE_TO_LABEL, &CAbusLabelerDlg::OnWriteToLabel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO1, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC1, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC2, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC3, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC4, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC5, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC6, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC7, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_RADIOC8, &CAbusLabelerDlg::OnBnClickedRadio)
	ON_BN_CLICKED(IDC_BUTTON1, &CAbusLabelerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CAbusLabelerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CAbusLabelerDlg::OnBnClickedButton3)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_CA1, &CAbusLabelerDlg::OnBnClickedButtonCa1)
	ON_BN_CLICKED(IDC_BUTTON_CA2, &CAbusLabelerDlg::OnBnClickedButtonCa2)
	ON_BN_CLICKED(IDC_BUTTON_CA3, &CAbusLabelerDlg::OnBnClickedButtonCa3)
	ON_COMMAND(ID_SAVE, &CAbusLabelerDlg::OnSave)
	ON_WM_HSCROLL()
	ON_NOTIFY(NM_CLICK, IDC_LIST_TUMOR, &CAbusLabelerDlg::OnNMClickListTumor)
END_MESSAGE_MAP()


// CAbusLabelerDlg 消息处理程序

BOOL CAbusLabelerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_ctrMenu.LoadMenu(IDR_MENU1);
	SetMenu(&m_ctrMenu);
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitStausBar();
	InitTimer();
	Init3View();
	InitVolumeInfoList();
	InitTumorTypeString();
	InitTumorInfoList();
	/*
	TumorInfo info = { 0, 0 };
	info.birads = 2;
	info.boder_type[1] = TRUE;
	info.boder_type[3] = TRUE;
	info.morph = 2;
	info.derection = 0;
	info.inner_echo = 2;
	info.back_echo = 1;
	info.cacif_type[0] = TRUE;
	info.con_phen.boold_type = 1;
	info.con_phen.type[3] = TRUE;
	info.con_phen.type[4] = TRUE;
	m_tumors.push_back(info);
	InsertTumor(0, info);
	//info.other_case[3] = TRUE;
	//info.other_case[4] = TRUE;
	//info.other_case[10] = TRUE;
	*/
	InitButtun();
	InitSliderBar();
	m_bShow = FALSE;
	m_bSave = FALSE;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAbusLabelerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAbusLabelerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



LRESULT CAbusLabelerDlg::OnShowPos(WPARAM pos_mes, LPARAM view_id)
{
	PositionMessage* pos = (PositionMessage *)pos_mes;
	CString strPos;
	strPos.Format(_T("坐标: (%d, %d)"), pos->x, pos->y);
	m_wndStatusBar.SetPaneText(1, strPos);

	CPoint p(pos->x, pos->y);
	COLORREF color;
	switch (view_id)
	{
	case 1:
		color = m_AxialView.GetDC()->GetPixel(p);
		break;
	case 2:
		color = m_SagittalView.GetDC()->GetPixel(p);
		break;
	case 3:
		color = m_CoronalView.GetDC()->GetPixel(p);
		break;
	}
	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);
	CString strRGB;
	strRGB.Format(_T("RGB(%d, %d, %d)"), r, g, b);
	m_wndStatusBar.SetPaneText(2, strRGB);
	return 0;
}

LRESULT CAbusLabelerDlg::OnUpdateDSCProgress(WPARAM pos, LPARAM lp)
{
	m_ProgressState.SetPos((int)pos);
	if (pos == 100)
	{
		// 显示图像
		m_view_index[2] = m_volume.m_dsc_image_height / 2;
		m_view_index[1] = m_volume.m_dsc_image_width / 2;
		m_view_index[0] = m_volume.GetFrames() / 2;
		m_volume.CreatLabelBuffer();
		Show3View(m_view_index[2], m_view_index[1], m_view_index[0]);
		SetSliderBar();
		m_bShow = TRUE;
	}
	return 0;
}

LRESULT CAbusLabelerDlg::OnUpdate2Views(WPARAM pos_mes, LPARAM view_id)
{
	if (!m_bShow)
	{
		return 0;
	}
	NormPosition* pos = (NormPosition *) pos_mes;
	int height = 0, width = 0, depth = 0;
	CSliderCtrl* pSlidCtrl = NULL;
	switch (view_id)
	{
	case 1:
		width = pos->x * m_volume.m_dsc_image_width -1;
		height = pos->y * m_volume.m_dsc_image_height -1;
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
		pSlidCtrl->SetPos(width);
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
		pSlidCtrl->SetPos(height);
		UpdateView2(height, width, depth);
		UpdateView3(height, width, depth);
		m_view_index[1] = width;
		m_view_index[2] = height;
		break;
	case 2:
		depth = pos->x * m_volume.GetFrames() - 1;
		height = pos->y * m_volume.m_dsc_image_height -1;
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
		pSlidCtrl->SetPos(depth);
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
		pSlidCtrl->SetPos(height);
		m_view_index[0] = depth;
		m_view_index[2] = height;
		UpdateView1(height, width, depth);
		UpdateView3(height, width, depth);
		break;
	case 3:
		width = pos->x * m_volume.m_dsc_image_width - 1;
		depth = pos->y * m_volume.GetFrames() - 1;
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
		pSlidCtrl->SetPos(depth);
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
		pSlidCtrl->SetPos(width);
		m_view_index[0] = depth;
		m_view_index[1] = width;
		UpdateView1(height, width, depth);
		UpdateView2(height, width, depth);
		break;
	}
	return 0;
}

LRESULT CAbusLabelerDlg::OnNextView(WPARAM z_delta, LPARAM view_id)
{
	int  delta = 0;
	if (z_delta == 1)
	{
		delta = 1;
	}
	else
	{
		delta = -1;
	}
	
	if (m_bShow)
	{
		switch (view_id)
		{
		case 1:
			if (m_view_index[0] + delta >= m_volume.GetFrames() - 1)
			{
				m_view_index[0] = m_volume.GetFrames() - 1;
			}
			else if (m_view_index[0] + delta <= 0)
			{
				m_view_index[0] = 0;
			}
			else
			{
				m_view_index[0] = m_view_index[0] + delta;
			}
			UpdateView1(0, 0, m_view_index[0]);
			break;
		case 2:
			if (m_view_index[1] + delta >= m_volume.m_dsc_image_width - 1)
			{
				m_view_index[1] = m_volume.m_dsc_image_width - 1;
			}
			else if (m_view_index[1] + delta <= 0)
			{
				m_view_index[1] = 0;
			}
			else
			{
				m_view_index[1] = m_view_index[1] + delta;
			}
			UpdateView2(0, m_view_index[1], 0);
			break;
		case 3:
			if (m_view_index[2] + delta >= m_volume.m_dsc_image_height - 1)
			{
				m_view_index[2] = m_volume.m_dsc_image_height - 1;
			}
			else if (m_view_index[2] + delta <= 0)
			{
				m_view_index[2] = 0;
			}
			else
			{
				m_view_index[2] = m_view_index[2] + delta;
			}
			UpdateView3(m_view_index[2], 0, 0);
			break;
		}
		// 更新滑动条
		CSliderCtrl   *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
		pSlidCtrl->SetPos(m_view_index[0]);
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
		pSlidCtrl->SetPos(m_view_index[1]);
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
		pSlidCtrl->SetPos(m_view_index[2]);
	}
	return 0;
}

LRESULT CAbusLabelerDlg::OnWriteToLabel(WPARAM pImageIn, LPARAM view_id)
{
	CImage * pImage = (CImage *)pImageIn;
	switch (view_id)
	{
	case 1:
		WriteLabelView1(pImage);
		break;
	case 2:
		WriteLabelView2(pImage);
		break;
	case 3:
		WriteLabelView3(pImage);
		break;
	}
	return 0;
}

void CAbusLabelerDlg::WriteLabelView1(CImage *pImage)
{
	uchar* pimg = (uchar*)pImage->GetBits(); //获取CImage的像素存贮区的指针
											 // 图像还原为初始大小
	int height = pImage->GetHeight();
	int width = pImage->GetWidth();
	Uint8* pBuffer = new Uint8[height * width];
	Uint8* pLabelBuffer = m_volume.GetLabelBuffer();
	int step = pImage->GetPitch();			// 每行的字节数,注意这个返回值有正有负
	COLORREF color;
	Uint8 value;
	int index = 0;
	int cur_image_index = 0;
	int label_height = m_volume.m_dsc_image_height;
	int label_width = m_volume.m_dsc_image_width;
	int label_depth = m_volume.GetFrames();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// 获取坐标颜色
			color = RGB(*(pimg + i * step + j * 3), *(pimg + i * step + j * 3 + 1), *(pimg + i * step + j * 3 + 2));
			value = m_AxialView.PesudoColorToGray(color);
			pBuffer[width*i + j] = value;
		}
	}
	// 将label放缩汇原始大小
	cv::Mat label1(cv::Size(width, height), CV_8U, pBuffer);
	cv::Mat re_label1;
	cv::resize(label1, re_label1, cv::Size(m_volume.m_dsc_image_width, m_volume.m_dsc_image_height), cv::INTER_NEAREST);
	cur_image_index = m_view_index[0];
	// 复制回label volume
	for (int i = 0; i < m_volume.m_dsc_image_height; i++)
	{
		for (int j = 0; j < m_volume.m_dsc_image_width; j++)
		{
			index = label_height * label_width* cur_image_index + label_width * i + j;
			pLabelBuffer[index] = re_label1.at<uchar>(i, j);
		}
	}
	delete[] pBuffer;
}

void CAbusLabelerDlg::WriteLabelView2(CImage *pImage)
{
	uchar* pimg = (uchar*)pImage->GetBits(); //获取CImage的像素存贮区的指针
											 // 图像还原为初始大小
	int height = pImage->GetHeight();
	int width = pImage->GetWidth();
	Uint8* pBuffer = new Uint8[height * width];
	Uint8* pLabelBuffer = m_volume.GetLabelBuffer();
	int step = pImage->GetPitch();			// 每行的字节数,注意这个返回值有正有负
	COLORREF color;
	Uint8 value;
	int index = 0;
	int cur_image_index = 0;
	int label_height = m_volume.m_dsc_image_height;
	int label_width = m_volume.m_dsc_image_width;
	int label_depth = m_volume.GetFrames();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			color = RGB(*(pimg + i * step + j * 3), *(pimg + i * step + j * 3 + 1), *(pimg + i * step + j * 3 + 2));
			value = m_SagittalView.PesudoColorToGray(color);
			pBuffer[width*i + j] = value;
		}
	}
	// 将label放缩汇原始大小
	cv::Mat label2(cv::Size(width, height), CV_8U, pBuffer);
	cv::Mat re_label2;
	cv::resize(label2, re_label2, cv::Size(m_volume.GetFrames(), m_volume.m_dsc_image_height), cv::INTER_NEAREST);
	cur_image_index = m_view_index[1];
	// 复制回label volume
	for (int i = 0; i < m_volume.m_dsc_image_height; i++)
	{
		for (int j = 0; j < m_volume.GetFrames(); j++)
		{
			index = label_height * label_width* j + label_width * i + cur_image_index;
			pLabelBuffer[index] = re_label2.at<uchar>(i, j);
		}
	}
	delete[] pBuffer;
}


void CAbusLabelerDlg::WriteLabelView3(CImage *pImage)
{
	uchar* pimg = (uchar*)pImage->GetBits(); //获取CImage的像素存贮区的指针
											 // 图像还原为初始大小
	int height = pImage->GetHeight();
	int width = pImage->GetWidth();
	Uint8* pBuffer = new Uint8[height * width];
	Uint8* pLabelBuffer = m_volume.GetLabelBuffer();
	int step = pImage->GetPitch();			// 每行的字节数,注意这个返回值有正有负
	COLORREF color;
	Uint8 value;
	int index = 0;
	int cur_image_index = 0;
	int label_height = m_volume.m_dsc_image_height;
	int label_width = m_volume.m_dsc_image_width;
	int label_depth = m_volume.GetFrames();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			color = RGB(*(pimg + i * step + j * 3), *(pimg + i * step + j * 3 + 1), *(pimg + i * step + j * 3 + 2));
			value = m_CoronalView.PesudoColorToGray(color);
			pBuffer[width*i + j] = value;
		}
	}
	cur_image_index = m_view_index[2];
	// 将label放缩汇原始大小
	cv::Mat label3(cv::Size(width, height), CV_8U, pBuffer);
	cv::Mat re_label3;
	cv::resize(label3, re_label3, cv::Size(m_volume.m_dsc_image_width, m_volume.GetFrames()), cv::INTER_NEAREST);
	// 复制回label volume
	for (int i = 0; i < m_volume.GetFrames(); i++)
	{
		for (int j = 0; j < m_volume.m_dsc_image_width; j++)
		{
			index = label_height * label_width* i + label_width * cur_image_index + j;
			pLabelBuffer[index] = re_label3.at<uchar>(i, j);
		}
	}
	delete[] pBuffer;
}

void CAbusLabelerDlg::UpdateVolumeInfo()
{
	m_volumeInfoList.DeleteAllItems();
	
	m_volumeInfoList.InsertItem(0, "Patient Name");
	m_volumeInfoList.SetItemText(0, 1, m_volume.GetPatientName());

	m_volumeInfoList.InsertItem(1, "Patient ID");
	m_volumeInfoList.SetItemText(1, 1, m_volume.GetPatientID());

	m_volumeInfoList.InsertItem(2, "Patient Age");
	CString age;
	age.Format("%d", m_volume.GetPatientAge());
	m_volumeInfoList.SetItemText(2, 1, age);

	CString sex;
	if (m_volume.GetPatientSex())
	{
		sex = "Male";
	}
	else
	{
		sex = "Female";
	}
	m_volumeInfoList.InsertItem(3, "Patient Sex");
	m_volumeInfoList.SetItemText(3, 1, sex);

	m_volumeInfoList.InsertItem(4, "Volume Height");
	CString height;
	height.Format("%03d", m_volume.GetHeight());
	m_volumeInfoList.SetItemText(4, 1, height);

	CString width;
	m_volumeInfoList.InsertItem(5, "Volume Width");
	width.Format("%03d", m_volume.GetWidth());
	m_volumeInfoList.SetItemText(5, 1, width);

	CString frame;
	m_volumeInfoList.InsertItem(6, "Volume Frames");
	frame.Format("%03d", m_volume.GetFrames());
	m_volumeInfoList.SetItemText(6, 1, frame);

	m_volumeInfoList.InsertItem(7, "Volume View");
	CString view_name = m_volume.GetViewName();
	m_volumeInfoList.SetItemText(7, 1, view_name);

	CString spacing;
	m_volumeInfoList.InsertItem(8, "Slice Spacing");
	spacing.Format("%.3f", m_volume.GetSliceSpacing());
	m_volumeInfoList.SetItemText(8, 1, spacing);

	double s_x = 0;
	double s_y = 0;
	m_volume.GetPixelSpacing(s_x, s_y);
	m_volumeInfoList.InsertItem(9, "Pixel Spacing");
	CString p_spacing;
	p_spacing.Format("(%.3f, %.3f）", s_x, s_y);
	m_volumeInfoList.SetItemText(9, 1, p_spacing);
	UpdateData(FALSE);
}


void CAbusLabelerDlg::Show3View(int x, int y, int z)
{
	UpdateView1(x, y, z);
	UpdateView2(x, y, z);
	UpdateView3(x, y, z);
}


void CAbusLabelerDlg::UpdateView1(int x, int y, int z)
{
	// 计算图片比例
	int height = m_volume.m_dsc_image_height;
	int width = m_volume.m_dsc_image_width;
	CRect rect;
	m_AxialView.GetClientRect(&rect);
	// 缩放图像
	// width不变
	double scale_ratio = rect.Width() * 1.0 / width;
	int new_height = height * scale_ratio;
	int new_width = rect.Width();
	// show view 1
	Uint8 * pImage = new Uint8[m_volume.m_dsc_image_height * m_volume.m_dsc_image_width];
	m_volume.GetViewImage(pImage, 1, x, y, z);
	cv::Mat image(cv::Size(m_volume.m_dsc_image_width, m_volume.m_dsc_image_height), CV_8U, pImage);
	cv::Mat view1_img;
	cv::resize(image, view1_img, cv::Size(new_width, new_height));

	// 获取标注图像，并融合显示
	m_volume.GetViewLabel(pImage, 1, x, y, z);
	cv::Mat label_img(cv::Size(m_volume.m_dsc_image_width, m_volume.m_dsc_image_height), CV_8U, pImage);
	cv::Mat re_label_img;
	cv::resize(label_img, re_label_img, cv::Size(new_width, new_height), cv::INTER_NEAREST);
	m_AxialView.ShowCVImage(view1_img, re_label_img);
	delete[] pImage;
}

void CAbusLabelerDlg::UpdateView2(int x, int y, int z)
{
	// show view2
	// 获取视图长宽
	CRect rect;
	int height = m_volume.m_dsc_image_height;
	double slice_s = m_volume.GetSliceSpacing();
	double height_s, width_s;
	m_volume.GetPixelSpacing(height_s, width_s);
	int width = m_volume.GetFrames()*slice_s / width_s;

	m_SagittalView.GetClientRect(&rect);
	double scale_ratio = rect.Width() * 1.0 / width;
	int new_height = height * scale_ratio;
	int new_width = rect.Width();
	// buffer
	Uint8 * pImage = new Uint8[m_volume.m_dsc_image_height * m_volume.GetFrames()];
	// 转换位原始dsc volume坐标
	m_volume.GetViewImage(pImage, 2, x, y, z);
	cv::Mat image(cv::Size(m_volume.GetFrames(), m_volume.m_dsc_image_height), CV_8U, pImage);
	cv::Mat view2_img;
	cv::resize(image, view2_img, cv::Size(new_width, new_height));

	// 获取标注图像，并融合显示
	m_volume.GetViewLabel(pImage, 2, x, y, z);
	cv::Mat label_img(cv::Size(m_volume.GetFrames(), m_volume.m_dsc_image_height), CV_8U, pImage);
	cv::Mat re_label_img;
	cv::resize(label_img, re_label_img, cv::Size(new_width, new_height), cv::INTER_NEAREST);
	m_SagittalView.ShowCVImage(view2_img, re_label_img);
	delete[] pImage;
}


void CAbusLabelerDlg::UpdateView3(int x, int y, int z)
{
	CRect rect;
	double height_s, width_s;
	m_volume.GetPixelSpacing(height_s, width_s);
	double slice_s = m_volume.GetSliceSpacing();
	// 物理dsc volume size
	int height = m_volume.GetFrames() * slice_s / width_s;
	int width = m_volume.GetWidth();
	m_CoronalView.GetClientRect(&rect);
	// 显示dsc volume size
	double scale_ratio = (rect.Width() - 50) * 1.0 / width;
	int new_height = height * scale_ratio;
	int new_width = (rect.Width() - 50);

	// buffer
	Uint8 * pImage = new Uint8[m_volume.GetFrames()*m_volume.GetWidth()];
	// 转换位原始dsc volume坐标
	m_volume.GetViewImage(pImage, 3, x, y, z);
	cv::Mat image(cv::Size(width, m_volume.GetFrames()), CV_8U, pImage);
	cv::Mat view1_img;
	cv::resize(image, view1_img, cv::Size(new_width, new_height));

	// 获取标注图像，并融合显示
	m_volume.GetViewLabel(pImage, 3, x, y, z);
	cv::Mat label_img(cv::Size(width, m_volume.GetFrames()), CV_8U, pImage);
	cv::Mat re_label_img;
	cv::resize(label_img, re_label_img, cv::Size(new_width, new_height), cv::INTER_NEAREST);
	m_CoronalView.ShowCVImage(view1_img, re_label_img);
	delete[] pImage;
}

void CAbusLabelerDlg::ScenePosToDSCImage(int x, int dim)
{

}

void CAbusLabelerDlg::SetSliderBar()
{
	CSliderCtrl   *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
	pSlidCtrl->SetRange(0, m_volume.GetFrames() - 1);
	pSlidCtrl->SetTicFreq(1);
	pSlidCtrl->SetPos(m_view_index[0]);
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
	pSlidCtrl->SetRange(0, m_volume.m_dsc_image_width -1);
	pSlidCtrl->SetTicFreq(1);
	pSlidCtrl->SetPos(m_view_index[1]);
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
	pSlidCtrl->SetRange(0, m_volume.m_dsc_image_height - 1);
	pSlidCtrl->SetTicFreq(1);
	pSlidCtrl->SetPos(m_view_index[2]);
}

void CAbusLabelerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent) 
	{
	case TIME_TIMER:
	{
		CString strTime;
		// 获取系统当前时间，并保存到curTime   
		CTime curTime = CTime::GetCurrentTime();
		// 格式化curTime，将字符串保存到strTime   
		strTime = curTime.Format(_T("%Y年%m月%d日 %H:%M:%S"));
		// 在状态栏的时间窗格中显示系统时间字符串   
		m_wndStatusBar.SetPaneText(0, strTime);
		break;
	}
	case VIEW1_TIMER:
	{
		UpdateView1(0, 0, m_view_index[0]);
		m_view_index[0] = (m_view_index[0] + 1) % m_volume.GetFrames();
		break;
	}
	case VIEW2_TIMER:
		UpdateView2(0, m_view_index[1], 0);
		m_view_index[1] = (m_view_index[1] + 1) % m_volume.m_dsc_image_width;
		break;
	case VIEW3_TIMER:
		UpdateView3(m_view_index[2], 0, 0);
		m_view_index[2] = (m_view_index[2] + 1) % m_volume.m_dsc_image_height;
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CAbusLabelerDlg::OnOpenFile()
{
	// TODO: 在此添加命令处理程序代码
	// 是否忘记保存
	if (!m_bSave && m_bShow)
	{
		int ret = AfxMessageBox(_T("是否保存Label？"), MB_OKCANCEL | MB_ICONQUESTION);
		if (ret == 1)
		{
			OnSave();
		}
	}
	CString filter = _T("DICOM(*.dcm)|*.dcm | Nii File(*.nii)|*.nii;*.nii.gz | 所有文件(*.*)|*.*||");
	CFileDialog fileDLg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter, NULL);
	CString path_name;
	if (IDOK == fileDLg.DoModal())
	{
		m_bSave = false;
		m_tumors.clear();
		m_volumeInfoList.DeleteAllItems();
		m_nLineType = 0;
		m_cnt_save_photo = 0;
		m_bShow = FALSE;
		path_name = fileDLg.GetPathName();			// 获取文件路径  
		m_fPath = fileDLg.GetFolderPath();
		m_fileName = fileDLg.GetFileName();
		SetWindowText(m_fileName);
		const char * cstr = (LPCTSTR)path_name;
		// 打开文件
		if (m_volume.LoadDicomFile(cstr))
		{
			UpdateVolumeInfo();
			m_volume.CreatDSCBuffer();
			StratDSCThread();
			/*
			if (m_pVtkImage)
			{
				m_pVtkImage->Delete();
			}
			m_pVtkImage = vtkImageData::New();
			m_pVtkImage->SetOrigin(0, 0, 0);
			m_pVtkImage->SetSpacing(1, 1, 1);		// 设置体素间距
			m_pVtkImage->SetDimensions(m_volume.GetHeight(), m_volume.GetWidth(), m_volume.GetFrames());
			m_pVtkImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
			Uint8 * scalar_buf = (Uint8*)m_pVtkImage->GetScalarPointer();
			Uint8 * pUint8 = m_volume.GetPixelData();
			memcpy(scalar_buf, pUint8, m_volume.GetHeight()*m_volume.GetWidth()*m_volume.GetFrames());
			*/
			/*
			Uint8 * pImage = new Uint8[m_volume.GetHeight() * m_volume.GetWidth()];
			m_volume.GetFirstFrame(pImage);
			CRect rect;
			GetDlgItem(IDC_STATIC_VIEW1)->GetClientRect(&rect);
			const cv::Mat view1_image(cv::Size(m_volume.GetWidth(), m_volume.GetHeight()), CV_8U, pImage);
			cv::Mat shrink;
			resize(view1_image, shrink, cv::Size(rect.Width(), rect.Height()));
			imshow("view1", shrink);
			*/		
			UpdateData(FALSE);
		}
		else
		{
			MessageBox(_T("打开文件失败，请重新选择"));
		}
		
	}
}


void CAbusLabelerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	// TODO: 在此处添加消息处理程序代码
}


void CAbusLabelerDlg::OnBnClickedRadio()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if (m_nLineType && m_bShow)
	{
		// 在表格中增加一行
		int cnt_in = 0;
		for (int i = 0; i < m_tumors.size(); i++)
		{
			if (m_tumors[i].pesudo_color != m_nLineType)
			{
				cnt_in++;
			}
		}
		// 如果是新的颜色， 增加tumor数量
		if (cnt_in == m_tumors.size())
		{
			TumorInfo info = { 0, 0 };
			info.pesudo_color = m_nLineType;
			m_tumors.push_back(info);
			InsertTumor(m_volume.m_num_Tumors, info);
			m_volume.m_num_Tumors++;
		}
		m_AxialView.m_b_draw_mode = TRUE;
		m_CoronalView.m_b_draw_mode = TRUE;
		m_SagittalView.m_b_draw_mode = TRUE;
		if (!m_volume.GetLabelBuffer())
		{
			m_volume.CreatLabelBuffer();
		}
	}
	else
	{
		m_AxialView.m_b_draw_mode = FALSE;
		m_CoronalView.m_b_draw_mode = FALSE;
		m_SagittalView.m_b_draw_mode = FALSE;	
	}
	m_AxialView.m_line_type = m_nLineType;
	m_CoronalView.m_line_type = m_nLineType;
	m_SagittalView.m_line_type = m_nLineType;
}



HBRUSH CAbusLabelerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	if (pWnd->GetDlgCtrlID() == IDC_RADIO1)//特定的某一个标签，IDC_STATIC_FONT为标签控件ID
	{
		pDC->SetBkMode(BKMODE_LAST);
		pDC->SetBkColor(RGB(255, 0, 0));
	}
	if (pWnd->GetDlgCtrlID() == IDC_RADIO2)
	{
		pDC->SetBkMode(BKMODE_LAST);
		pDC->SetBkColor(RGB(0, 255, 0));
	}
	if (pWnd->GetDlgCtrlID() == IDC_RADIO3)//特定的某一个标签，IDC_STATIC_FONT为标签控件ID
	{
		pDC->SetBkMode(BKMODE_LAST);
		pDC->SetBkColor(RGB(0, 0, 255));
	}
	if (pWnd->GetDlgCtrlID() == IDC_RADIO4a)
	{
		pDC->SetBkMode(BKMODE_LAST);//透明
		pDC->SetBkColor(RGB(255, 255, 0));
	}
	if (pWnd->GetDlgCtrlID() == IDC_RADIO4b)
	{
		pDC->SetBkMode(BKMODE_LAST);//透明
		pDC->SetBkColor(RGB(0, 255, 255));
	}
	if (pWnd->GetDlgCtrlID() == IDC_RADIO5)
	{
		pDC->SetBkMode(BKMODE_LAST);
		pDC->SetBkColor(RGB(255, 255, 0));
	}
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CAbusLabelerDlg::StratDSCThread()
{
	DSCThreadParm* pParm = new DSCThreadParm;
	pParm->hWnd = this->GetSafeHwnd();
	pParm->pVolumeData = m_volume.GetPixelData();
	pParm->pBuffer = m_volume.GetDSCPixelData();
	pParm->padding_len = m_volume.GetBorderPaddingLen();
	pParm->height = m_volume.GetHeight();
	pParm->width = m_volume.GetWidth();
	pParm->frame = m_volume.GetFrames();
	pParm->depth_s = m_volume.GetSliceSpacing();
	double s1, s2;
	m_volume.GetPixelSpacing(s1, s2);
	pParm->height_s = s1;
	pParm->width_s = s2;
	m_hThread = AfxBeginThread((AFX_THREADPROC)CalcDSCThreadFunc, (LPVOID)pParm);
}


UINT CalcDSCThreadFunc(LPVOID pParm)
{
	DSCThreadParm* pParmRev = (DSCThreadParm*)pParm;
	HWND hWnd = pParmRev->hWnd;
	unsigned char * pVolume = pParmRev->pVolumeData;
	unsigned char * pBuffer = pParmRev->pBuffer;
	int padding_len = pParmRev->padding_len;
	int height = pParmRev->height;
	int width = pParmRev->width;
	int frame = pParmRev->frame;
	double s_x = pParmRev->height_s;
	double s_y = pParmRev->width_s;
	double s_z = pParmRev->depth_s;
	int new_height = height * s_x / s_y;         
	int new_width = width;
	int new_frame = frame * s_z / s_y;
	ABUSAlg alg(padding_len);
	int progress = 0;
//#pragma omp parallel for num_threads(4)
	for (int i = 0; i < frame; i++)
	{
		Uint8* pArr = pVolume + i * height * width;
		cv::Mat org_image(cv::Size(width, height), CV_8U, pArr);
		cv::Mat re_image;
		cv::resize(org_image, re_image, cv::Size(new_width, new_height));
		cv::Mat dsc_image;
		alg.DSC2D(dsc_image, re_image, cv::INTER_LINEAR);
		// 拷贝数据回缓存中
		pArr = pBuffer + i * (new_height + padding_len) * new_width;
		memcpy(pArr, (Uint8*)dsc_image.data, new_width*(new_height + padding_len));
		// save image;
		//CString name;
		//name.Format(_T("dsc_image%d.bmp"), i);
		//const char * cstr = (LPCTSTR)name;
		//cv::imwrite(cstr, dsc_image);
		progress = (i + 1.0) / frame * 100;
		::PostMessage(hWnd, WM_DSC_PROGRESS, progress, 10);
	}
	delete pParmRev;
	return 0;
}

UINT ReDSCLabelAndSaveThreadFunc(LPVOID pParm)
{
	ReDSCLabelThreadParm* pParmRev = (ReDSCLabelThreadParm*)pParm;
	HWND hWnd = pParmRev->hWnd;
	MedicalVolume* pMedicalVolume = pParmRev->pVolume;
	int frame = pMedicalVolume->GetFrames();
	int height = pMedicalVolume->m_dsc_image_height;
	int width = pMedicalVolume->m_dsc_image_width;
	Uint8* pVolume = pMedicalVolume->GetLabelBuffer();
	ABUSAlg alg(pMedicalVolume->GetBorderPaddingLen());

	int new_height = 0, new_width = 0, new_frame = 0;
	new_height = pMedicalVolume->GetHeight();
	new_width = pMedicalVolume->GetWidth();

	vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
	// Specify the size of the image data
	imageData->SetDimensions(new_width, new_height, frame);
	imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
	double s1, s2;
	pMedicalVolume->GetPixelSpacing(s1, s2);
	double spacing[3] = { s2, s1, pMedicalVolume->GetSliceSpacing()};
	imageData->SetSpacing(spacing);
	unsigned char* pVoxel = static_cast<unsigned char*>(imageData->GetScalarPointer(0, 0, 0));
	#pragma omp parallel for num_threads(4)
	for (int i = 0; i < frame; i++)
	{
		Uint8* pArr = pVolume + i * height * width;
		cv::Mat org_image(cv::Size(width, height), CV_8U, pArr);
		cv::Mat re_dsc;
		alg.ReDSC2D(re_dsc, org_image, cv::INTER_NEAREST);
		cv::Mat image;
		cv::resize(re_dsc, image, cv::Size(new_width, new_height));
		// 拷贝数据回vtkData中
		pArr = pVoxel + i * new_height * new_width;
		memcpy(pArr, (Uint8*)image.data, new_width*new_height);
	}
	vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
	writer->SetInputData(imageData);
	writer->SetFileName(pParmRev->file_name);
	writer->Write();
	delete pParmRev;
	return 0;
}


void CAbusLabelerDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	// 自动播放
	static bool bStart = TRUE;
	if (m_bShow && bStart)
	{
		bStart = FALSE;
		SetTimer(VIEW1_TIMER, 100, NULL);
	}
	else
	{
		bStart = TRUE;
		KillTimer(VIEW1_TIMER);
	}
	
}


void CAbusLabelerDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	static bool bStart = TRUE;
	if (m_bShow && bStart)
	{
		bStart = FALSE;
		SetTimer(VIEW2_TIMER, 100, NULL);
	}
	else
	{
		bStart = TRUE;
		KillTimer(VIEW2_TIMER);
	}
	
}


void CAbusLabelerDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	static bool bStart = TRUE;
	if (m_bShow && bStart)
	{
		bStart = FALSE;
		SetTimer(VIEW3_TIMER, 100, NULL);
	}
	else
	{	
		bStart = TRUE;
		KillTimer(VIEW3_TIMER);
	}
	
}


void CAbusLabelerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (!m_bShow)
	{
		return;
	}

	int pos = 0;
	CSliderCtrl   *pSlidCtrl = NULL;
	if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER1)
	{
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER1);
		pos = pSlidCtrl->GetPos();
		m_view_index[0] = pos;
		UpdateView1(0, 0, pos);
	}
	else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER2)
	{
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
		pos = pSlidCtrl->GetPos();
		m_view_index[1] = pos;
		UpdateView2(0, pos, 0);
	}
	else if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER3)
	{
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
		pos = pSlidCtrl->GetPos();
		m_view_index[2] = pos;
		UpdateView3(pos, 0, 0);
	}

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

/*
void CAbusLabelerDlg::OnBnClickedButtonColor()
{
	// TODO: 在此添加控件通知处理程序代码
	COLORREF color = RGB(255, 0, 0);     
	CColorDialog colorDlg(color);       
	CString strTmp;
	if (IDOK == colorDlg.DoModal())
	{
		color = colorDlg.GetColor();
		strTmp.Format("%d", GetRValue(color));
		SetDlgItemText(IDC_STATIC_R, (LPCTSTR)strTmp);
		strTmp.Format("%d", GetGValue(color));
		SetDlgItemText(IDC_STATIC_G, (LPCTSTR)strTmp);
		strTmp.Format("%d", GetBValue(color));
		SetDlgItemText(IDC_STATIC_B, (LPCTSTR)strTmp);
	}
}
*/


void CAbusLabelerDlg::OnBnClickedButtonCa1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_fPath.IsEmpty())
	{
		CString name;
		name.Format("view1_%d.bmp", m_cnt_save_photo);
		name = m_fPath + "\\" + name;
		m_AxialView.SavePhoto(name);
	}
	m_cnt_save_photo++;
}


void CAbusLabelerDlg::OnBnClickedButtonCa2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_fPath.IsEmpty())
	{
		CString name;
		name.Format("view2_%d.bmp", m_cnt_save_photo);
		name = m_fPath + "\\" + name;
		m_SagittalView.SavePhoto(name);
	}
}


void CAbusLabelerDlg::OnBnClickedButtonCa3()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_fPath.IsEmpty())
	{
		CString name;
		name.Format("view3_%d.bmp", m_cnt_save_photo);
		name = m_fPath + "\\" + name;
		m_CoronalView.SavePhoto(name);
	}
}


void CAbusLabelerDlg::OnSave()
{
	// TODO: 在此添加命令处理程序代码
	BOOL isOpen = FALSE;			//是否打开(否则为保存)
	CString defaultDir = m_fPath;	//默认打开的文件路径
	CString fileName = _T("demo.nii");			//默认打开的文件名
	CString filter = _T("文件 (*.nii; *.nii.gz;*.csv;)|*.nii; *.nii.gz;*.csv;||");				//文件过虑的类型
	CFileDialog openFileDlg(isOpen, defaultDir, fileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);
	//openFileDlg.GetOFN().lpstrInitialDir = _T("E:\\FileTest\\test.doc");
	openFileDlg.GetOFN().lpstrTitle = _T("保存标注图像和Tumor信息");
	INT_PTR result = openFileDlg.DoModal();
	CString file_folder;
	CString file_name;
	if (result == IDOK) 
	{
		file_name = openFileDlg.GetFileTitle();
		file_folder = openFileDlg.GetFolderPath();
	}
	else
	{
		return;
	}
	if (m_tumorInfoList.GetItemCount() > 0)
	{
		CString strFileName = file_folder + "\\" + file_name + ".csv";
		CFile file;
		if (!file.Open(strFileName, CFile::modeCreate | CFile::modeWrite))
		{
			MessageBox(_T("文件此时不能打开"), _T("提示"));
			return;
		}
		CString strHead = _T("Tumor编号,BIRADS,形态,生长方向,边界,内部回声,后方回声,钙化,伴随现象\r\n");
		file.Write(strHead, strHead.GetLength());
		for (int nItem = 0; nItem < m_tumorInfoList.GetItemCount(); nItem++)
		{
			CString strData;
			for (int i = 0; i < 10; i++)
			{
				strData += m_tumorInfoList.GetItemText(nItem, i);
				if (9 == i)
				{
					strData += _T("\r\n");
				}
				else
				{
					strData += _T(",");
				}
			}
			file.Write(strData, strData.GetLength());
		}
		file.Close();
		// 后台保存文件
		ReDSCLabelThreadParm* pParm = new ReDSCLabelThreadParm;
		pParm->pVolume = &m_volume;
		pParm->file_name = file_folder + "\\" + file_name + ".nii";
		pParm->hWnd = this->GetSafeHwnd();
		m_hThread = AfxBeginThread((AFX_THREADPROC)ReDSCLabelAndSaveThreadFunc, (LPVOID)pParm);
		m_bSave = true;
	}
}


void CAbusLabelerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl   *pSlidCtrl = NULL;
	if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER_FUSION)
	{
		pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_FUSION);
		double alpha = pSlidCtrl->GetPos() / 100.0;
		m_AxialView.m_alpha = alpha;
		m_SagittalView.m_alpha = alpha;
		m_CoronalView.m_alpha = alpha;
		// 刷新视图
		if (m_bShow)
		{
			Show3View(m_view_index[2], m_view_index[1], m_view_index[0]);
		}
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CAbusLabelerDlg::OnNMClickListTumor(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	NM_LISTVIEW  *pEditCtrl = (NM_LISTVIEW *)pNMHDR;
	int tumor_id = pEditCtrl->iItem;
	if (tumor_id >= m_tumors.size())
	{
		return;
	}
	CTumorInfoDlg dlg;
	dlg.m_info = m_tumors[tumor_id];
	if (dlg.DoModal())
	{
		m_tumors[tumor_id] = dlg.m_info;
		InsertTumor(tumor_id, dlg.m_info);
	}
	*pResult = 0;
}

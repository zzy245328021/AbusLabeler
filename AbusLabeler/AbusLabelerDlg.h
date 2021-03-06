
// AbusLabelerDlg.h: 头文件
//

#pragma once
#include "CImageView.h"
#include "MedicalVolume.h"
#include "TumorInfoDlg.h"
#include <vector>
#include <string>
#define WM_SHOW_POS WM_USER+10
#define WM_DSC_PROGRESS WM_USER+11
#define WM_UPDATE_VIEW  WM_USER+12
#define WM_NEXT_VIEW	WM_USER+13
#define WM_WRITE_TO_LABEL	WM_USER+14
#define TIME_TIMER 1
#define VIEW1_TIMER 2
#define VIEW2_TIMER 3
#define VIEW3_TIMER 4
using std::vector;
using std::string;


struct PositionMessage
{
	int x;
	int y;
};

struct NormPosition
{
	double x;
	double y;
};

typedef struct
{
	HWND hWnd;
	unsigned char * pVolumeData;
	unsigned char * pBuffer;
	int padding_len;
	int height;
	int width;
	int frame;
	double height_s;
	double width_s;
	double depth_s;
}DSCThreadParm;

typedef struct
{
	HWND hWnd;
	CString file_name;
	MedicalVolume * pVolume;
}ReDSCLabelThreadParm;


// CAbusLabelerDlg 对话框
class CAbusLabelerDlg : public CDialogEx
{
// 构造
public:
	CAbusLabelerDlg(CWnd* pParent = nullptr);	// 标准构造函数
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABUSLABELER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CStatusBar m_wndStatusBar;
	CMenu m_ctrMenu;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnShowPos(WPARAM, LPARAM);		//处理鼠标悬停消息
	afx_msg LRESULT OnUpdateDSCProgress(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdate2Views(WPARAM, LPARAM);
	afx_msg LRESULT OnNextView(WPARAM, LPARAM);
	afx_msg LRESULT OnWriteToLabel(WPARAM, LPARAM);
	void WriteLabelView1(CImage *pImage);
	void WriteLabelView2(CImage *pImage);
	void WriteLabelView3(CImage *pImage);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnOpenFile();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRadio();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

public:
	void InitStausBar();
	void InitTimer();
	void Init3View();
	void InitVolumeInfoList();
	void InitTumorInfoList();
	void InitTumorTypeString();
	void InsertTumor(int tumor_id, TumorInfo info);
	void InitButtun();
	void InitSliderBar();
	void UpdateVolumeInfo();
	void StratDSCThread();
	void Show3View(int x, int y, int z);
	void UpdateView1(int x, int y, int z);
	void UpdateView2(int x, int y, int z);
	void UpdateView3(int x, int y, int z);
	void ScenePosToDSCImage(int x, int dim);
	void SetSliderBar();
	// volume信息
	MedicalVolume m_volume;
	CListCtrl m_volumeInfoList;
	// 标记肿瘤的信息
	CListCtrl m_tumorInfoList;
	CProgressCtrl m_ProgressState;
	// 根据BIRADS分级设置绘制线条类型
	int m_nLineType;
	CImageView m_AxialView;
	CImageView m_SagittalView;
	CImageView m_CoronalView;
	int m_view_index[3];
	CWinThread* m_hThread;
	int m_cnt_save_photo;
	DWORD m_ThreadID;
	CString m_fPath;
	CString m_fileName;

	vector<TumorInfo> m_tumors;
	vector<string> m_strBirads;
	vector<string> m_strMorph;
	vector<string> m_strDirection;
	vector<string> m_strBorder;
	vector<string> m_strInnerEcho;
	vector<string> m_strBackEcho;
	vector<string> m_strCacif;
	vector<string> m_strConPhen;
	vector<string> m_strOtherCase;
	bool m_bSave;
	afx_msg void OnBnClickedButton1();
	bool m_bShow;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButtonCa1();
	afx_msg void OnBnClickedButtonCa2();
	afx_msg void OnBnClickedButtonCa3();
	afx_msg void OnSave();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnNMClickListTumor(NMHDR *pNMHDR, LRESULT *pResult);
};

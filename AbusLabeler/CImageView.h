#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
using std::vector;


// CImageView
class CImageView : public CStatic
{
	DECLARE_DYNAMIC(CImageView)

public:
	CImageView();
	virtual ~CImageView();
	void CreatMemDC();
protected:
	DECLARE_MESSAGE_MAP()
public:
	void SetViewID(int view_id);
	void DrawCusor(CDC* pDC, int x, int y);
	void SavePhoto(CString name);
	afx_msg void OnPaint();
	void MatToCImage(cv::Mat& mat, cv::Mat& label, CImage& cImage, CImage& cImage2, double alpha);
	void ShowCVImage(cv::Mat vMat, cv::Mat label);
	void ScreenToLabel();
	void LabelToMemDC(cv::Mat &label, CImage& image_label);
	CDC m_memDC;
	CDC m_label_memDC;
	CBitmap m_bmp;
	CBitmap m_label_bmp;
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	COLORREF GetPesudoColor(int index);
	uchar PesudoColorToGray(COLORREF color);
	afx_msg void OnDestroy();
	bool m_use_mem_dc;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
private:
	CString m_view_name;
	int m_view_id;
	CPoint m_old_point;
public:
	bool m_b_draw_line;
	bool m_b_draw_mode;
	int m_line_type;
	int m_offset_height;
	int m_offset_width;
	COLORREF m_line_color;
	vector<CPoint> m_points;
	double m_alpha;
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};



// CImageView.cpp: 实现文件
//

#include "stdafx.h"
#include "AbusLabeler.h"
#include "CImageView.h"
#include "AbusLabelerDlg.h"

// CImageView

IMPLEMENT_DYNAMIC(CImageView, CStatic)

CImageView::CImageView()
{
	m_b_draw_line = FALSE;
	m_use_mem_dc = FALSE;
	m_b_draw_mode = FALSE;
}

CImageView::~CImageView()
{

}

void CImageView::CreatMemDC()
{
	CRect rect;
	GetClientRect(&rect);
	CDC * pDC = GetDC();
	// 双缓冲显示缓冲
	m_memDC.CreateCompatibleDC(pDC);	//依附窗口DC创建兼容内存DC
	m_bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());	//创建兼容位图，大小与绘图区一致
	m_memDC.SelectObject(&m_bmp);		//将位图选择进内存DC，显示出来
	// label缓存
	m_label_memDC.CreateCompatibleDC(pDC);	//依附窗口DC创建兼容内存DC
	m_label_bmp.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());	//创建兼容位图，大小与绘图区一致
	m_label_memDC.SelectObject(&m_label_bmp);		//将位图选择进内存DC，显示出来

	ReleaseDC(pDC);
	m_use_mem_dc = TRUE;
}

void CImageView::SetViewID(int view_id)
{
	m_view_id = view_id;
	switch (view_id)
	{
	case 1:
		m_view_name = "横截面";
		break;
	case 2:
		m_view_name = "矢状面";
		break;
	case 3:
		m_view_name = "冠状面";
		break;
	}
}

void CImageView::DrawCusor(CDC* pDC, int x, int y)
{
	CRect rect;
	GetWindowRect(&rect);
	int win_height = rect.Height();
	int win_width = rect.Width();
	int radius = 5;
	int left = 0, right = 0, top = 0, down = 0;
	if (x - radius >= 0)
	{
		left = x - radius;
	}
	else
	{
		left = 0;
	}

	if (x + radius >= win_width)
	{
		right = win_width - 1; 
	}
	else
	{ 
		right = x + radius;
	}

	if (y - radius >= 0)
	{
		top = y - radius;
	}
	else
	{
		top = 0;
	}
	if (y + radius >= win_height)
	{
		down = win_height - 1;
	}
	else
	{
		down = y + radius;
	}
	pDC->MoveTo(left, y);
	pDC->LineTo(right, y);
	pDC->MoveTo(x, top);
	pDC->LineTo(x, down);
	pDC->MoveTo(left, top);
	pDC->LineTo(right, down);
	pDC->MoveTo(right, top);
	pDC->LineTo(left, down);
}

void CImageView::SavePhoto(CString name)
{
	CDC* pDC = this->GetDC();
	CRect rect;  
	GetWindowRect(&rect);
	CImage image;
	image.Create(rect.Width(), rect.Height(), 32);
	::BitBlt(image.GetDC(), 0, 0, rect.Width(), rect.Height(), pDC->m_hDC, 0, 0, SRCCOPY);
	image.Save(name);
	ReleaseDC(pDC);
	image.ReleaseDC();
}

BEGIN_MESSAGE_MAP(CImageView, CStatic)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()



// CImageView 消息处理程序
void CImageView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CStatic::OnPaint()
	CDC * pDC = GetDC();
	CRect rect;
	GetClientRect(&rect);
	if (m_use_mem_dc)
	{
		m_memDC.SetTextColor(RGB(255, 255, 255));
		m_memDC.SetBkMode(TRANSPARENT);
		m_memDC.TextOut(0, 0, m_view_name);
		pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &m_memDC, 0, 0, SRCCOPY);
	}
	ReleaseDC(pDC);
}


void CImageView::MatToCImage(cv::Mat& mat, cv::Mat& label, CImage& cImage, CImage& cImage2, double alpha)
{
	int width = mat.cols;
	int height = mat.rows;
	int channels = mat.channels();
	cImage.Destroy();						//这一步是防止重复利用造成内存问题
	cImage.Create(width, height, 3*8);

	cImage2.Destroy();						//这一步是防止重复利用造成内存问题
	cImage2.Create(width, height, 3*8);

	uchar* ps;
	uchar* pl;
	uchar* pimg = (uchar*)cImage.GetBits(); //获取CImage的像素存贮区的指针
	uchar* pimg2 = (uchar*)cImage2.GetBits(); //获取CImage的像素存贮区的指针
	int step = cImage.GetPitch();			// 每行的字节数,注意这个返回值有正有负
	COLORREF color;
	for (int i = 0; i < height; i++)
	{
		ps = mat.ptr<uchar>(i);
		pl = label.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			color = GetPesudoColor(pl[j]);
			if (color == 0)				// 如果是Label像素是黑色，显示原图像素
			{
				*(pimg + i * step + j * 3) = ps[j];
				*(pimg + i * step + j * 3 + 1) = ps[j];
				*(pimg + i * step + j * 3 + 2) = ps[j];
			}
			else
			{	// 如果是Label像素是其他颜色，显示混合颜色
				*(pimg + i * step + j * 3) = (1 - alpha) * ps[j] + alpha * GetRValue(color);
				*(pimg + i * step + j * 3 + 1) = (1 - alpha) * ps[j] + alpha * GetGValue(color);
				*(pimg + i * step + j * 3 + 2) = (1 - alpha) * ps[j] + alpha * GetBValue(color);
			}
			*(pimg2 + i * step + j * 3) = GetRValue(color);
			*(pimg2 + i * step + j * 3 + 1) = GetGValue(color);
			*(pimg2 + i * step + j * 3 + 2) = GetBValue(color);
		}
	}

}

void CImageView::ShowCVImage(cv::Mat vMat, cv::Mat label)
{
	if (vMat.empty()) 
	{
		return;
	}
	//获取图片的宽 高度
	int img_width = vMat.cols;
	int img_height = vMat.rows;
	//获取Picture Control控件的大小
	CRect rect;
	GetWindowRect(&rect);
	int win_height = rect.Height();
	int win_width = rect.Width();
	//把opencv的图转为CImage,才能显示在图像控件上
	CImage cimg;
	CImage cimg2;
	MatToCImage(vMat, label, cimg, cimg2, m_alpha);
	CBrush brush(RGB(0, 0, 0));
	m_memDC.FillRect(rect, &brush);
	// 绘制图像在中心
	m_offset_height = (win_height - img_height) / 2;
	m_offset_width = (win_width - img_width) / 2;
	// 分离图像和label和显示
	cimg.Draw(m_memDC.m_hDC, CRect(m_offset_width, m_offset_height, img_width, img_height));
	LabelToMemDC(label, cimg2);
	OnPaint();
}


void CImageView::ScreenToLabel()
{
	CRect rect;
	GetWindowRect(&rect);
	CImage image;
	image.Create(rect.Width() - 2 * m_offset_width, rect.Height() - 2 * m_offset_height, 24);
	::BitBlt(image.GetDC(), 0, 0, image.GetWidth(), image.GetHeight(), m_label_memDC.m_hDC, m_offset_width, m_offset_height, SRCCOPY);
	GetParent()->SendMessage(WM_WRITE_TO_LABEL, (WPARAM)&image, m_view_id);
	image.ReleaseDC();
}

void CImageView::LabelToMemDC(cv::Mat &label, CImage& image_label)
{
	CRect rect;
	GetWindowRect(&rect);
	int win_height = rect.Height();
	int win_width = rect.Width();
	int img_width = label.cols;
	int img_height = label.rows;
	CBrush brush(RGB(0, 0, 0));
	m_label_memDC.FillRect(rect, &brush);
	image_label.Draw(m_label_memDC.m_hDC, CRect(m_offset_width, m_offset_height, img_width - m_offset_width, img_height - m_offset_height));
}

void CImageView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  添加您的代码以绘制指定项

}


COLORREF CImageView::GetPesudoColor(int index)
{
	COLORREF color;
	switch (index)
	{
	case 1:
		color = RGB(0, 0, 255);
		break;
	case 2:
		color = RGB(0, 255, 0);
		break;
	case 3:
		color = RGB(255, 0, 0);
		break;
	case 4:
		color = RGB(255, 255, 0);
		break;
	case 5:
		color = RGB(255, 0, 255);
		break;
	case 6:
		color = RGB(0, 255, 255);
		break;
	case 7:
		color = RGB(125, 0, 0);
		break;
	case 8:
		color = RGB(0, 125, 0);
		break;
	case 9:
		color = RGB(0, 0, 125);
		break;
	default:
		color = RGB(0, 0, 0);
	}
	return color;
}

uchar CImageView::PesudoColorToGray(COLORREF color)
{
	uchar index;
	switch (color)
	{
	case RGB(0, 0, 255):
		index = 1;
		break;
	case RGB(0, 255, 0):
		index = 2;
		break;
	case RGB(255, 0, 0):
		index = 3;
		break;
	case RGB(255, 255, 0):
		index = 4;
		break;
	case RGB(255, 0, 255):
		index = 5;
		break;
	case RGB(0, 255, 255):
		index = 6;
		break;
	case RGB(125, 0, 0):
		index = 7;
		break;
	case RGB(0, 125, 0):
		index = 8;
		break;
	case RGB(0, 0, 125):
		index = 9;
		break;
	default:
		index = 0;
	}

	return index;
}

void CImageView::OnDestroy()
{
	CStatic::OnDestroy();
	// TODO: 在此处添加消息处理程序代码
	m_memDC.DeleteDC();		//删除DC
	m_bmp.DeleteObject();
	m_label_memDC.DeleteDC();
	m_label_bmp.DeleteObject();
}


void CImageView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	PositionMessage mes;
	mes.x = point.x;
	mes.y = point.y;
	GetParent()->SendMessage(WM_SHOW_POS, (WPARAM)&mes, m_view_id);

	if (m_b_draw_mode && m_b_draw_line)
	{
		CDC* pDC = GetDC();
		CPen PenForDrawAxis;
		COLORREF color;
		color = GetPesudoColor(m_line_type);
		PenForDrawAxis.CreatePen(PS_SOLID, 2, color);
		m_line_color = color;
		pDC->SelectObject(PenForDrawAxis);
		m_label_memDC.SelectObject(PenForDrawAxis);
		m_points.push_back(point);
		pDC->MoveTo(m_old_point);
		pDC->LineTo(point);
		m_label_memDC.MoveTo(m_old_point);
		m_label_memDC.LineTo(point);
		m_old_point = point;
		ReleaseDC(pDC);
	}
	CStatic::OnMouseMove(nFlags, point);
}


void CImageView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_b_draw_line = TRUE;
	if (m_b_draw_mode)
	{
		m_old_point = point;
		m_points.clear();
		m_points.push_back(point);
	}
	else
	{
		OnPaint();
		// 切换视图
		CDC* pDC = GetDC();
		CPen PenForDrawAxis;
		CPen*pPen;
		PenForDrawAxis.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
		pDC->SelectObject(PenForDrawAxis);
		DrawCusor(pDC, point.x, point.y);
		ReleaseDC(pDC);
		NormPosition mes;
		CRect rect;
		GetWindowRect(&rect);
		int win_height = rect.Height();
		int win_width = rect.Width();
		// 坐标转换
		mes.x = (point.x - 1.0 * m_offset_width) / (win_width - 2.0 * m_offset_width);
		if (mes.x < 0)
		{
			mes.x = 0;
		}
		else if (mes.x > 1)
		{
			mes.x = 1.0;
		}
		mes.y = (point.y - 1.0 * m_offset_height) / (win_height - 2.0 * m_offset_height);
		if (mes.y < 0)
		{
			mes.y = 0;
		}
		else if (mes.y > 1)
		{
			mes.y = 1.0;
		}
		GetParent()->SendMessage(WM_UPDATE_VIEW, (WPARAM)&mes, m_view_id);
	}
	CStatic::OnLButtonDown(nFlags, point);
}


void CImageView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_b_draw_line = FALSE;
	if (m_b_draw_mode)
	{
		CRgn rgn;
		CPoint * arrpt = new CPoint[m_points.size()];
		for (int i = 0; i < m_points.size(); i++)
		{
			arrpt[i] = m_points[i];
		}
		rgn.CreatePolygonRgn(arrpt, m_points.size(), ALTERNATE);
		CDC* pDC = GetDC();
		CBrush bsh(m_line_color);
		pDC->FillRgn(&rgn, &bsh);

		// 更新label界面
		m_label_memDC.FillRgn(&rgn, &bsh);
		ScreenToLabel();
		rgn.DeleteObject();
		delete[] arrpt;
		ReleaseDC(pDC);
		// 保存绘图到缓冲
	}
	CStatic::OnLButtonUp(nFlags, point);
}


BOOL CImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(zDelta > 0)
	{
		GetParent()->SendMessage(WM_NEXT_VIEW, 1, m_view_id);
	}
	else
	{
		GetParent()->SendMessage(WM_NEXT_VIEW, 2, m_view_id);
	}
	return CStatic::OnMouseWheel(nFlags, zDelta, pt);
}

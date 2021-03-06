// CvtkView3D.cpp: 实现文件
//

#include "stdafx.h"
#include "AbusLabelerDlg.h"
#include "CvtkView3D.h"


// CvtkView3D

IMPLEMENT_DYNAMIC(CvtkView3D, CStatic)

CvtkView3D::CvtkView3D()
{

}

CvtkView3D::~CvtkView3D()
{
}


BEGIN_MESSAGE_MAP(CvtkView3D, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CvtkView3D 消息处理程序




void CvtkView3D::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	CRect rect;
	GetClientRect(rect);
	m_Renderer = vtkSmartPointer<vtkRenderer>::New();
	m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	m_RenderWindow->SetParentId(this->m_hWnd);
	m_RenderWindow->SetSize(rect.Width(), rect.Height());
	m_RenderWindow->AddRenderer(m_Renderer);

	if (m_RenderWindow->GetInteractor() == NULL)
	{
		vtkSmartPointer<vtkRenderWindowInteractor> RenderWindowInteractor =
			vtkSmartPointer<vtkRenderWindowInteractor>::New();
		RenderWindowInteractor->SetRenderWindow(m_RenderWindow);
		RenderWindowInteractor->Initialize();
	}

	CStatic::PreSubclassWindow();
}


void CvtkView3D::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CStatic::OnPaint()
}

void CvtkView3D::Render()
{
	m_Renderer->ResetCamera();
	m_RenderWindow->Render();
}

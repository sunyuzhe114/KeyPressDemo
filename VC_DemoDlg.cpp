
// VC_DemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VC_Demo.h"
#include "VC_DemoDlg.h"
#include "afxdialogex.h"
#include "msdk.h"
#include "UsbHidKeyCode.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>
#include <io.h> 
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
using namespace cv;
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVC_DemoDlg 对话框
double rate = 2;//* dbZoomScale
CVC_DemoDlg *pDlg;
bool bStop = false;
DWORD m_dTimeBegin = 0;
DWORD m_timeLimit = 0;
/// Global Variables
bool use_mask = false;
Mat img;
Mat templ;
Mat mask; Mat result;
const char* image_window = "Source Image";
const char* result_window = "Result window";
bool MatchingMethod();
int match_method = TM_SQDIFF_NORMED;
int max_Trackbar = 5;
//! [declare]
float dbZoomScale = 1.0;
/// Function Headers
void MatchingMethod(int, void*);
DWORD WINAPI    changeUser_Thread(LPVOID pp);
CVC_DemoDlg::CVC_DemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVC_DemoDlg::IDD, pParent)
	, m_intMinute(55)
	, m_edit_keyword(_T("勇士")
	)
{
	m_rate = 2.5;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVC_DemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_intMinute);
	DDV_MinMaxInt(pDX, m_intMinute, 0, 3000);
	DDX_Control(pDX, IDC_EDIT2, m_editLog);
	DDX_Control(pDX, IDC_EDIT3, m_editLogInfor);
	DDX_Control(pDX, IDC_LIST1, m_listWindow);
	DDX_Text(pDX, IDC_EDIT4, m_edit_keyword);
	DDX_Text(pDX, IDC_EDIT5, m_rate);
	DDX_Control(pDX, IDC_EDIT5, m_editRate);
}

BEGIN_MESSAGE_MAP(CVC_DemoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CVC_DemoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CVC_DemoDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CVC_DemoDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS, &CVC_DemoDlg::OnBnClickedButtonKeypress)
	ON_BN_CLICKED(IDC_BUTTON_MOVER, &CVC_DemoDlg::OnBnClickedButtonMover)
	ON_BN_CLICKED(IDC_BUTTON_MOVETO, &CVC_DemoDlg::OnBnClickedButtonMoveto)
	ON_BN_CLICKED(IDC_BUTTON_GETMOUSEPOS, &CVC_DemoDlg::OnBnClickedButtonGetmousepos)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS3, &CVC_DemoDlg::OnBnClickedButtonKeypress3)
	ON_BN_CLICKED(IDC_BUTTON_OPEN2, &CVC_DemoDlg::OnBnClickedButtonOpen2)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS5, &CVC_DemoDlg::OnBnClickedButtonKeypress5)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS4, &CVC_DemoDlg::OnBnClickedButtonKeypress4)
	ON_EN_CHANGE(IDC_EDIT2, &CVC_DemoDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS6, &CVC_DemoDlg::OnBnClickedButtonKeypress6)
	ON_EN_CHANGE(IDC_EDIT5, &CVC_DemoDlg::OnEnChangeEdit5)
END_MESSAGE_MAP()


// CVC_DemoDlg 消息处理程序

BOOL CVC_DemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	msdk_handle = INVALID_HANDLE_VALUE;	//初始为未打开
	pDlg = this;
	// Get desktop dc


	HDC desktopDc = CreateDC(_T("display"), NULL, NULL, NULL);
	// Get native resolution
	int horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);

	dbZoomScale = horizontalDPI / 96.0f;
	CString strScale;
	strScale.Format("分辨比例%0.2lf", dbZoomScale);
	m_editLog.SetWindowTextA(strScale);
	m_editRate.SetWindowTextA("2.5"); 
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVC_DemoDlg::OnPaint()
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
HCURSOR CVC_DemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVC_DemoDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
}

void CVC_DemoDlg::OnBnClickedButtonOpen()
{

	CString l_Str;
	if (msdk_handle != INVALID_HANDLE_VALUE) {
		AfxMessageBox("端口已经打开，请关闭端口再重新打开");
	}
	msdk_handle = M_Open(1);
	M_ResolutionUsed(msdk_handle, 1920, 1080);
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		AfxMessageBox("端口打开失败，请确认您的USB设备已经插上电脑");
	}
	else {
		GetDlgItem(IDC_BUTTON_OPEN)->EnableWindow(false);
	}
}

void CVC_DemoDlg::OnBnClickedButtonClose()
{
	if (msdk_handle != INVALID_HANDLE_VALUE) {
		M_Close(msdk_handle);
		msdk_handle = INVALID_HANDLE_VALUE;
		GetDlgItem(IDC_BUTTON_OPEN)->EnableWindow(true);
	}
	else {
		AfxMessageBox("还未打开端口，请先打开端口");
	}
}

DWORD WINAPI    checkThread_Game_old(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;

	for (int j = 0; j < 500; j++)
	{
		if (GetTickCount() - m_dTimeBegin > m_timeLimit * 60 * 1000)
			bStop = true;

		if (bStop)break;

		RetSw = M_DelayRandom(800, 1000);
		CString strInfor;
		strInfor.Format("%ld秒", (GetTickCount() - m_dTimeBegin) / 1000);
		pDlg->m_editLog.SetWindowTextA(strInfor);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(300, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_w, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		RetSw = M_KeyPress(msdk_handle, Keyboard_r, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		if (bStop)break;

		RetSw = M_KeyPress(msdk_handle, Keyboard_e, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_w, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_d, 1);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;

		if (j % 2 == 0)
			RetSw = M_KeyPress(msdk_handle, Keyboard_PageDown, 1);

		RetSw = M_DelayRandom(700, 1100);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		if (bStop)break;
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(600, 1500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_t, 1);
		RetSw = M_DelayRandom(600, 1500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_s, 1);
		RetSw = M_DelayRandom(600, 1500);
		if (bStop)break;
		if (j % 2 == 0)
			RetSw = M_KeyPress(msdk_handle, Keyboard_F10, 1);



		RetSw = M_ReleaseAllKey(msdk_handle);
	}
	RetSw = M_KeyPress(msdk_handle, Keyboard_F12, 1);
	RetSw = M_ReleaseAllKey(msdk_handle);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
	return 0;
}
DWORD WINAPI    LoginUser_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;

	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, 1385 / rate, 110 / rate);
		RetSw = M_DelayRandom(800, 1000);
	}
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftClick(msdk_handle, 1);
	/*RetSw = M_LeftDown(msdk_handle );
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftUp(msdk_handle );*/



	//按5号键，
	RetSw = M_KeyPress(msdk_handle, Keyboard_5, 1);
	RetSw = M_DelayRandom(400, 600);
	infor += "按5号键\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	//点击确认
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(1800, 2000);
	infor += "按确定\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	//滚动3次到了 
	for (int i = 0; i < 3; i++)
	{
		RetSw = M_MouseWheel(msdk_handle, 1);
		RetSw = M_DelayRandom(1000, 1200);
	}

	infor += "滚轮3次\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);

	//走到地点
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1518) / rate), (int)((277) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);

	infor += "选定坐标\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);

	//点击确认
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(1800, 2000);
	infor += "按确定\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);


	//走到地点
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_MoveTo(msdk_handle, (int)((1753) / rate), (int)((503) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_RightClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);

	}
	infor += "走到地点\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	//点击 
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftDoubleClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftDoubleClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	infor += "点击\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
	RetSw = M_ReleaseAllMouse(msdk_handle);
	RetSw = M_ReleaseAllKey(msdk_handle);
	pDlg->OnBnClickedButtonKeypress();


	return 0;
}
DWORD WINAPI    changeUser_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;



	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, 1385 / rate, 110 / rate);;
		RetSw = M_DelayRandom(800, 1000);
	}
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftClick(msdk_handle, 1);
	/*RetSw = M_LeftDown(msdk_handle );
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftUp(msdk_handle );*/


	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
	RetSw = M_DelayRandom(1800, 2000);
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1534) / rate), (int)((454) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	infor += "Keyboard_ESCAPE\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	RetSw = M_KeyPress(msdk_handle, Keyboard_RightArrow, 1);
	RetSw = M_DelayRandom(1100, 2100);
	RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
	RetSw = M_DelayRandom(400, 600);
	RetSw = M_DelayRandom(3000, 4000);
	//按5号键，
	RetSw = M_KeyPress(msdk_handle, Keyboard_5, 1);
	RetSw = M_DelayRandom(400, 600);
	infor += "按5号键\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	//点击确认
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(1800, 2000);
	infor += "按确定\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	//滚动3次到了 
	for (int i = 0; i < 3; i++)
	{
		RetSw = M_MouseWheel(msdk_handle, 1);
		RetSw = M_DelayRandom(1000, 1200);
	}

	infor += "滚轮3次\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);

	//走到地点
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1518) / rate), (int)((277) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);

	infor += "选定坐标\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);

	//点击确认
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(1800, 2000);
	infor += "按确定\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);


	//走到地点
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_MoveTo(msdk_handle, (int)((1753) / rate), (int)((503) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_RightClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);

	}
	infor += "走到地点\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	//点击 
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftDoubleClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftDoubleClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	infor += "点击\r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
	pDlg->OnBnClickedButtonKeypress();

	/*RetSw = M_ReleaseAllMouse(msdk_handle);
	RetSw = M_ReleaseAllKey(msdk_handle);*/
	return 0;
}

DWORD WINAPI    checkThread_Game(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;
	while (bStop == false)
	{
		if (GetTickCount() - m_dTimeBegin > m_timeLimit * 60 * 1000)
		{
			bStop = true;
		}


		if (bStop)break;

		RetSw = M_DelayRandom(800, 1000);
		CString strInfor;
		strInfor.Format("%ld秒", (GetTickCount() - m_dTimeBegin) / 1000);
		pDlg->m_editLog.SetWindowText(strInfor);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(300, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_w, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		RetSw = M_KeyPress(msdk_handle, Keyboard_r, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		if (bStop)break;

		RetSw = M_KeyPress(msdk_handle, Keyboard_e, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_w, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_d, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		pDlg->saveScreen();
		bool bFind = MatchingMethod();
		if (bFind == TRUE)
		{
			RetSw = M_KeyPress(msdk_handle, Keyboard_PageDown, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			if (bStop)break;
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_F10, 1);
			RetSw = M_DelayRandom(1100, 2000);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			continue;
		}
		if (bStop)break;
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);

		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;

		//如果长时间无法通关，要放一次大招

		if (GetTickCount() - m_dTimeBeginPress_F10 > 300)
		{
			m_dTimeBeginPress_F10 = GetTickCount();
			RetSw = M_KeyPress(msdk_handle, Keyboard_a, 1);

			RetSw = M_DelayRandom(1400, 2600);
		}
		pDlg->saveScreen();
		bFind = MatchingMethod();
		if (bFind == TRUE)
		{
			RetSw = M_KeyPress(msdk_handle, Keyboard_PageDown, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			if (bStop)break;
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_F10, 1);
			RetSw = M_DelayRandom(3100, 4500);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			m_dTimeBeginPress_F10 = GetTickCount();
			continue;
		}


		RetSw = M_DelayRandom(700, 800);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;

		RetSw = M_DelayRandom(200, 500);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(600, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_t, 1);
		RetSw = M_DelayRandom(600, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_s, 1);
		RetSw = M_DelayRandom(600, 1000);
		if (bStop)break;



		/*ctrl+a  + delete
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyDown(msdk_handle,Keyboard_LeftControl);
		RetSw = M_KeyDown(msdk_handle, Keyboard_PageUp);
		RetSw = M_DelayRandom(500, 800);ts
		RetSw = M_KeyDown(msdk_handle,Keyboard_Delete);
		RetSw = M_DelayRandom(80, 200);*/

		RetSw = M_ReleaseAllKey(msdk_handle);
	}
	RetSw = M_KeyPress(msdk_handle, Keyboard_F12, 1);
	RetSw = M_DelayRandom(600, 1000);
	RetSw = M_ReleaseAllKey(msdk_handle);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
	return 0;
}
/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
bool MatchingMethod()
{
	img = cv::imread("d:\\s.bmp", IMREAD_COLOR);
	templ = cv::imread("d:\\f.bmp", IMREAD_COLOR);
	//! [copy_source]
	/// Source image to display
	Mat img_display;
	img.copyTo(img_display);
	//! [copy_source]

	//! [create_result_matrix]
	/// Create the result matrix
	int result_cols = img.cols - templ.cols + 1;
	int result_rows = img.rows - templ.rows + 1;

	result.create(result_rows, result_cols, CV_32FC1);
	//! [create_result_matrix]

	//! [match_template]
	/// Do the Matching and Normalize

	matchTemplate(img, templ, result, match_method);

	//! [match_template]

	//! [normalize]
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());
	//! [normalize]

	//! [best_match]
	/// Localizing the best match with minMaxLoc
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//! [best_match]

	//! [match_loc]
	/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
	if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
	}
	else
	{
		matchLoc = maxLoc;
	}

	//! [match_loc]

	//! [imshow]
	/// Show me what you got

	//rectangle(img_display, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);
	//rectangle(result, matchLoc, Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows), Scalar::all(0), 2, 8, 0);

	//imshow(image_window, img_display);
	//imshow(result_window, result);
	//! [imshow]
	CString infor;
	infor.Format("x=%ld,y=%ld,maxVal=%0.2lf", matchLoc.x, matchLoc.y, maxVal);
	if (matchLoc.y <= 45 && maxVal > 0.5 &&matchLoc.x > 1650 && matchLoc.x < 1920)
	{
		infor += "检测正确";
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		return TRUE;
	}
	else
	{
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		return FALSE;
	}

}

void CVC_DemoDlg::saveScreen()
{
	CWnd *pDesktop = this->GetDesktopWindow();
	CDC *pdeskdc = pDesktop->GetDC();
	CRect re;
	//获取窗口的大小
	pDesktop->GetClientRect(&re);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pdeskdc, re.Width(), re.Height());
	//创建一个兼容的内存画板
	CDC memorydc;
	memorydc.CreateCompatibleDC(pdeskdc);
	//选中画笔
	CBitmap *pold = memorydc.SelectObject(&bmp);
	//绘制图像
	memorydc.BitBlt(0, 0, re.Width(), re.Height(), pdeskdc, 0, 0, SRCCOPY);
	//获取鼠标位置，然后添加鼠标图像
	CPoint po;
	GetCursorPos(&po);
	HICON hinco = (HICON)GetCursor();
	memorydc.DrawIcon(po.x - 10, po.y - 10, hinco);
	//选中原来的画笔
	memorydc.SelectObject(pold);
	BITMAP bit;
	bmp.GetBitmap(&bit);
	//定义 图像大小（单位：byte）
	DWORD size = bit.bmWidthBytes * bit.bmHeight;
	LPSTR lpdata = (LPSTR)GlobalAlloc(GPTR, size);
	//后面是创建一个bmp文件的必须文件头
	BITMAPINFOHEADER pbitinfo;
	pbitinfo.biBitCount = 24;
	pbitinfo.biClrImportant = 0;
	pbitinfo.biCompression = BI_RGB;
	pbitinfo.biHeight = bit.bmHeight;
	pbitinfo.biPlanes = 1;
	pbitinfo.biSize = sizeof(BITMAPINFOHEADER);
	pbitinfo.biSizeImage = size;
	pbitinfo.biWidth = bit.bmWidth;
	pbitinfo.biXPelsPerMeter = 0;
	pbitinfo.biYPelsPerMeter = 0;
	GetDIBits(pdeskdc->m_hDC, bmp, 0, pbitinfo.biHeight, lpdata, (BITMAPINFO*)
		&pbitinfo, DIB_RGB_COLORS);
	BITMAPFILEHEADER bfh;
	bfh.bfReserved1 = bfh.bfReserved2 = 0;
	bfh.bfType = ((WORD)('M' << 8) | 'B');
	bfh.bfSize = size + 54;
	bfh.bfOffBits = 54;
	//写入文件
	CFile file;
	//CString strFileName(GetAppPathW().c_str());
	//strFileName += _T("ScreenShot\\");
	//CreateDirectory((LPCTSTR)strFileName, NULL);
	//CTime t = CTime::GetCurrentTime();
	//CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
	//strFileName += tt;
	CString strFileName = _T("d:\\s.bmp");
	if (file.Open((LPCTSTR)strFileName, CFile::modeCreate | CFile::modeWrite))
	{
		file.Write(&bfh, sizeof(BITMAPFILEHEADER));
		file.Write(&pbitinfo, sizeof(BITMAPINFOHEADER));
		file.Write(lpdata, size);
		file.Close();
	}
	GlobalFree(lpdata);
	/// Global Variables 


}
void CVC_DemoDlg::OnBnClickedButtonKeypress()
{

	m_dTimeBegin = GetTickCount();
	UpdateData(true);
	m_timeLimit = m_intMinute;//分钟
	bStop = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	Sleep(3000);


	HANDLE hThread = CreateThread(NULL, 0, checkThread_Game, (LPVOID)msdk_handle, 0, NULL);

	GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(false);

}

void CVC_DemoDlg::OnBnClickedButtonMover()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		AfxMessageBox("还未打开端口，请先打开端口");
		return;
	}
	unsigned int RetSw;
	RetSw = M_MoveR(msdk_handle, -100, 100);
}

void CVC_DemoDlg::OnBnClickedButtonMoveto()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		AfxMessageBox("还未打开端口，请先打开端口");
		return;
	}
	unsigned int RetSw;
	RetSw = M_ResetMousePos(msdk_handle);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_MoveR(msdk_handle, 100, 100);
	RetSw = M_LeftClick(msdk_handle, 1);
}

void CVC_DemoDlg::OnBnClickedButtonGetmousepos()
{

	if (msdk_handle == INVALID_HANDLE_VALUE) {
		AfxMessageBox("还未打开端口，请先打开端口");
		return;
	}
	CString strtemp;
	unsigned int RetSw;
	int x_pos, y_pos;
	RetSw = M_GetCurrMousePos(msdk_handle, &x_pos, &y_pos);
	if (RetSw != 0) {
		AfxMessageBox("获取鼠标坐标错误");
		return;
	}
	else {
		strtemp.Format("x=%d, y=%d", x_pos, y_pos);
		AfxMessageBox(strtemp);
	}
}


void CVC_DemoDlg::OnBnClickedButtonKeypress3()
{
	bStop = true;
}


void CVC_DemoDlg::OnBnClickedButtonOpen2()
{
	saveScreen();
	if (MatchingMethod() == TRUE)
	{


	}

}


void CVC_DemoDlg::OnBnClickedButtonKeypress5()
{
	m_dTimeBegin = GetTickCount();
	UpdateData();
	m_timeLimit = m_intMinute;//分钟
	bStop = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	bStop = true;
	Sleep(3000);


	HANDLE hThread = CreateThread(NULL, 0, changeUser_Thread, (LPVOID)msdk_handle, 0, NULL);

}


void CVC_DemoDlg::OnBnClickedButtonKeypress4()
{
	UpdateData(TRUE);
	for (int i = (m_listWindow.GetCount() - 1); i >= 0; i--)
		m_listWindow.DeleteString(i);

	CWnd* pMainWnd = AfxGetMainWnd()->GetWindow(GW_HWNDFIRST);
	//	CWnd* pMainWnd = AfxGetMainWnd();

	DWORD timebegin = GetTickCount();
	bool bFindWindows = false;
	while (pMainWnd)//先列举所有的窗口
	{
		CString strClassName;
		CString text;
		CString strCurrentWindow;
		GetClassName(pMainWnd->m_hWnd, strClassName.GetBufferSetLength(100), 100);
		::GetWindowText(pMainWnd->m_hWnd, text.GetBufferSetLength(256), 256);
		strCurrentWindow = text;

		text.MakeLower();
		CString strInforText = text;


		if (::IsWindowVisible(pMainWnd->m_hWnd) || true)
		{



			//	if((strClassName=="#32770"&&(text.Fin2d("错误")!=-1||text.Find("microsoft visual c++")!=-1||text.Find("microsoft internet")!=-1||text.Find("安全警报")!=-1||text.Find("安全信息")!=-1||text.Find("windows internet explorer")!=-1||text.Find("连接到")!=-1) 19910205LGD
			//		||(strClassName=="Internet Explorer_TridentDlgFrame")))//||text.Find("object Error")!=-1
			if (strClassName.Find(m_edit_keyword) != -1 || text.Find(m_edit_keyword) != -1)

			{//如果是ie窗口则继续进行分析。
				CRect rect;
				::GetWindowRect(pMainWnd->m_hWnd, &rect);
				CString strWindow;
				//strWindow.Format("%lx,%s",pMainWnd->m_hWnd,strClassName); 
				strWindow.Format(_T("%lx,(%ld,%ld,%ld,%ld)"), pMainWnd->m_hWnd, rect.left, rect.top, rect.Width(), rect.Height());
				m_listWindow.AddString(strWindow);
				m_listWindow.SetCurSel(0);


				//HWND h=::GetWindow(pMainWnd->m_hWnd,GW_CHILD);
				//while(h)//分析ie窗口内部结构
				//{//广告窗口以及大部分恶意网站的弹出窗口都有一个共同特点
				//	//只有文本显示区，没有工具栏
				//	//若一个窗口为ie窗口则看其是否有工具栏，及可知它是否为广告窗口

				//	GetClassName(h,strClassName.GetBufferSetLength(100),100);
				//	::GetWindowText(h,text.GetBufferSetLength(256),256); 

				//	h=::GetWindow(h,GW_HWNDNEXT);

				//}

			}

		}
		if (::IsWindow(pMainWnd->m_hWnd))
			pMainWnd = pMainWnd->GetWindow(GW_HWNDNEXT);
	}
}


void CVC_DemoDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CVC_DemoDlg::OnBnClickedButtonKeypress6()
{
	m_dTimeBegin = GetTickCount();
	UpdateData();
	m_timeLimit = m_intMinute;//分钟
	bStop = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	bStop = true;
	Sleep(3000);


	HANDLE hThread = CreateThread(NULL, 0, LoginUser_Thread, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码
}


void CVC_DemoDlg::OnEnChangeEdit5()
{
	UpdateData();
	rate = m_rate;
}

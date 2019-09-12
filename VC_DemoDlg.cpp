
// VC_DemoDlg.cpp : ʵ���ļ�
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
#define APP_NAME "notepad++"
int  Game_state = -1;//Game_state = 100;�û��Ѿ����꣬200������������
int not_in_game_time = 0;//��⵽δ����Ϸ�д���
// CVC_DemoDlg �Ի���
int Global_checkTime = 0;
double rate = 2.5;//* dbZoomScale
CVC_DemoDlg *pDlg;
bool bStop = false;
bool bFullStop = false;
bool bChangeUser = true;
DWORD m_dTimeBegin = 0;
DWORD m_timeLimit = 0;
const UINT TIMER_LENGTH = 600000;
LONG dleft = 1120;
LONG dtop = 0;
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
int checkGame_state( );
DWORD WINAPI    changeUser_Thread(LPVOID pp);
void addLog(CString infor);
void addLog_important(CString infor);
void addLog(CString infor)
{
	CTime time=CTime::GetCurrentTime();
	CString date = time.Format("%H:%M:%S");
	CString strshow = date + "   " + infor;
	pDlg->m_listLog.InsertString(0,strshow);
	if (pDlg->m_listLog.GetCount() > 1000)
	{
		pDlg->m_listLog.ResetContent();
	}
	pDlg->m_editLogInfor.SetWindowTextA(strshow);
}
void addLog_important(CString infor)
{
	CTime time = CTime::GetCurrentTime();
	CString date = time.Format("%H:%M:%S");
	CString strshow = date + "   " + infor;
	pDlg->m_list_time_log.InsertString(0, strshow);
	 
}
int my_M_MoveTo(HANDLE m_hdl, int x, int y) 
{ 
	int SCREEN_CX = pDlg->m_screenWidth;//#1920  
	long changeX = (dleft - (SCREEN_CX - 800))/rate;
	long changeY = dtop / rate;


	
	//long changeX = (dleft - 1120)/rate;
	//long changeY = dtop/rate ;
	CString infor;
	infor.Format("move mouse to %ld,%ld", x + changeX, y + changeY);
	addLog(infor);
	return M_MoveTo(  m_hdl,   x + changeX,   y+changeY);
	//return M_MoveTo(m_hdl, x  , y );
}
//��ָ��ͼλ�ã�letf,top,right,buttom��ָ����Χ
CPoint findImage(string strPath_findImage,int left,int top,int right,int bottom)
{
	CPoint pt(0, 0);
	try {
		//pDlg->saveScreen();
		
		img = cv::imread("d:\\s.bmp", IMREAD_COLOR);
		templ = cv::imread(strPath_findImage, IMREAD_COLOR);
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

		//x = 1727, y = 70,can't tell you if game is over or other
		bool bResult = -1;
		int SCREEN_CX = pDlg->m_screenWidth;//#1920

		//x = 1727, y = 70,can't tell you if game is over or other
		//matchloc.x ,matchloc.y�������ϵͳ����
		//changeX,changeY �����Ϸ������
		long changeX = matchLoc.x -dleft;
		long changeY = matchLoc.y - dtop;
;
		/* long changeX = dleft - 1120;
		long changeY = dtop - 0; */
		infor.Format("x=%ld,y=%ld,maxVal=%0.2lf,changeX=%ld,changeY=%ld", matchLoc.x, matchLoc.y, maxVal, changeX, changeY);
		addLog("find image" + infor);
		//if ((matchLoc.x - changeX) > 1255 && (matchLoc.x - changeX) < 1615 && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <=430 && maxVal > 0.5 )
		if (changeX > left && changeX <right && changeY >= top && changeY <= bottom && maxVal > 0.5)
		{

			pt.x = matchLoc.x;
			pt.y = matchLoc.y;
			return pt;
		}
		else
		{
			infor += "δ��⵽��ť ";
			addLog(infor);
			bResult = -1; Game_state = -1;
			pt.x = 0;
			pt.y = 0;
			return pt;
		}
		img.release();
		templ.release();
		return pt;
	}
	catch (Exception &e)
	{
		addLog("error findimg");
	}
	pt.x = 0;
	pt.y = 0;
	return pt;

}
CPoint findSureButton_state()
{
	CPoint pt(0,0);
	try {

		 
	
	
	img = cv::imread("d:\\s.bmp", IMREAD_COLOR);
	templ = cv::imread("d:\\confirm.png", IMREAD_COLOR);
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
	
	//x = 1727, y = 70,can't tell you if game is over or other
	bool bResult = -1;
	int SCREEN_CX = pDlg->m_screenWidth;//#1920
	 
	//x = 1727, y = 70,can't tell you if game is over or other
 
	long changeX = dleft - (SCREEN_CX - 800);
	long changeY = dtop - 0;
	/* long changeX = dleft - 1120;
	long changeY = dtop - 0; */
	 infor.Format("x=%ld,y=%ld,maxVal=%0.2lf,changeX=%ld,changeY=%ld", matchLoc.x, matchLoc.y, maxVal, changeX, changeY);
	 addLog("find sure button" + infor);
	//if ((matchLoc.x - changeX) > 1255 && (matchLoc.x - changeX) < 1615 && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <=430 && maxVal > 0.5 )
	 if ((matchLoc.x - changeX) > (SCREEN_CX - 695) && (matchLoc.x - changeX) < (SCREEN_CX-305) && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <= 430 && maxVal > 0.5)
	{ 
		 
		pt.x = matchLoc.x;
		pt.y = matchLoc.y;
		return pt;
	}
	else
	{
		infor += "δ��⵽��ť ";
		addLog(infor);
		bResult = -1;Game_state = -1;
		pt.x = 0;
		pt.y = 0;
		return pt;
	}
	img.release();
	templ.release();
	return pt;
	}
	catch (Exception &e)
	{
		addLog("catch error findSureButton_state");
	}
	pt.x = 0;
	pt.y = 0;
	return pt;

}
//0��ʾ�Ѿ��ʺ��ù�
//1��ʾ����������
//-1��ʾδ��⵽
int checkGame_state()
{
	try
	{
		img = cv::imread("d:\\s.bmp", IMREAD_COLOR);
		templ = cv::imread("d:\\fightagain.png", IMREAD_COLOR);
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
		int SCREEN_CX = pDlg->m_screenWidth;//#1920
		CString infor;
		//x = 1727, y = 70,can't tell you if game is over or other
		bool bResult = -1;
		long changeX = dleft - (SCREEN_CX - 800);
		long changeY = dtop - 0;
		infor.Format("x=%ld,y=%ld,maxVal=%0.2lf,changeX=%ld,changeY=%ld", matchLoc.x, matchLoc.y, maxVal, changeX, changeY);

		if ((matchLoc.x - changeX) > (SCREEN_CX - 200) && (matchLoc.x - changeX) < SCREEN_CX && (matchLoc.y - changeY) >= 0 && (matchLoc.y - changeY) <= 75 && maxVal > 0.5)
		{
			//here may have error
				//ȡɫ����
			try
			{

				Mat NewImg = img(Rect(matchLoc.x, matchLoc.y, 80, 30));
				Mat means, stddev, covar;
			cv:Scalar tempVal = cv::mean(NewImg);
				float matMean = tempVal.val[0];
				CString strResult;
				//42 34 56//not change to grey
				// 43 29 45 //grey



				strResult.Format("means  : %0.0f %0.0f %0.0f\n", tempVal.val[0], tempVal.val[1], tempVal.val[2]);//RGB��ͨ�������Ծ�ֵ�����3��һ��

				//imshow("test", NewImg);
				if (tempVal.val[1] <= 30 && tempVal.val[2] <= 46)
				{
					CString str;
					CTime t = CTime::GetCurrentTime();
					CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

					str.Format("%s==>%s\r\n", tt, "�ʺ��Ѿ�ʹ����� 0");
					/*CStdioFile file;
					if (file.Open(_T("d:\\log.txt"), CFile::typeText | CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate, NULL))
					{
						file.SeekToEnd();
						file.WriteString(str);
						file.Close();
					}*/
					addLog_important(str);
					addLog("�ʺ��Ѿ�ʹ����� 0");
					Game_state = 100;
					bResult = 100;// 
				}
				else
				{
					infor = "��⵽��Ϸ���������� 1";
					addLog(infor);
					bResult = 200;//��⵽��Ϸ����������
					Game_state = 200;
					//	pDlg->MessageBoxA("��⵽��Ϸ����������,", "error", MB_OK);
				}
			}
			catch (exception& e)
			{
				return -1;
			}
		}
		else
		{
			infor += "δ��⵽���� -1 ";
			addLog(infor);
			//pDlg->MessageBoxA("δ��⵽����,", "error", MB_OK);
			bResult = -1; Game_state = -1;
		}
		img.release();
		templ.release();
		return bResult;
	}
	catch (Exception &e)
	{
		addLog("error checkGame_state");
		return -1;
	}
}
/**
 * @function MatchingMethod
 * @brief Trackbar callbackGame_state =100;
 */
bool MatchingMethod()
{
	try
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
		img.release();
		templ.release();;
		CString infor;
		long changeX = dleft - 1120;
		long changeY = dtop - 0;
		infor.Format("x=%ld,y=%ld,maxVal=%0.2lf,changeX=%ld,changeY=%ld", matchLoc.x, matchLoc.y, maxVal, changeX, changeY);
		addLog("MatchingMethod  " + infor);
		if ((matchLoc.x - changeX) > 1650 && (matchLoc.x - changeX) < 1920 && (matchLoc.y - changeY) >= 0 && (matchLoc.y - changeY) <= 45 && maxVal > 0.5)
			//if (matchLoc.y <= 45 && maxVal > 0.5 &&matchLoc.x > 1650 && matchLoc.x < 1920)
		{
			infor += "�����ȷ";
			pDlg->m_editLogInfor.SetWindowTextA(infor);
			return TRUE;
		}
		else
		{
			pDlg->m_editLogInfor.SetWindowTextA(infor);
			return FALSE;
		}

	}
	catch (Exception &e)
	{
		addLog("error MatchingMethod");
		return FALSE;
	}
}
CVC_DemoDlg::CVC_DemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVC_DemoDlg::IDD, pParent)
	, m_intMinute(120)
	, m_edit_keyword(_T("��ʿ")
	)
	, m_checkTimes(5)
	, m_screenWidth(1920)
	, bHuangLong(FALSE)
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
	DDX_Text(pDX, IDC_EDIT6, m_checkTimes);
	DDX_Control(pDX, IDC_LIST2, m_listLog);
	DDX_Text(pDX, IDC_EDIT7, m_screenWidth);
	DDX_Check(pDX, IDC_CHECK1, bHuangLong);
	DDX_Control(pDX, IDC_LIST3, m_list_time_log);
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
	ON_EN_CHANGE(IDC_EDIT6, &CVC_DemoDlg::OnEnChangeEdit6)
	ON_BN_CLICKED(IDC_BUTTON_MOVER2, &CVC_DemoDlg::OnBnClickedButtonMover2)
	ON_EN_CHANGE(IDC_EDIT1, &CVC_DemoDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS7, &CVC_DemoDlg::OnBnClickedButtonKeypress7)
	ON_LBN_SELCHANGE(IDC_LIST1, &CVC_DemoDlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON_GETMOUSEPOS2, &CVC_DemoDlg::OnBnClickedButtonGetmousepos2)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS8, &CVC_DemoDlg::OnBnClickedButtonKeypress8)
	ON_EN_CHANGE(IDC_EDIT7, &CVC_DemoDlg::OnEnChangeEdit7)
	ON_BN_CLICKED(IDC_BUTTON_OPEN3, &CVC_DemoDlg::OnBnClickedButtonOpen3)
	ON_LBN_SELCHANGE(IDC_LIST2, &CVC_DemoDlg::OnLbnSelchangeList2)
	ON_BN_CLICKED(IDC_CHECK1, &CVC_DemoDlg::OnBnClickedCheck1)
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CVC_DemoDlg ��Ϣ�������

BOOL CVC_DemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	msdk_handle = INVALID_HANDLE_VALUE;	//��ʼΪδ��
	pDlg = this;
	// Get desktop dc


	HDC desktopDc = CreateDC(_T("display"), NULL, NULL, NULL);
	// Get native resolution
	int horizontalDPI = GetDeviceCaps(desktopDc, LOGPIXELSX);

	dbZoomScale = horizontalDPI / 96.0f;
	CString strScale;
	strScale.Format("�ֱ����%0.2lf", dbZoomScale);
	m_editLog.SetWindowTextA(strScale);

	CString infor;

	::GetPrivateProfileString(APP_NAME, "m_rate", "", infor.GetBufferSetLength(256), 256, "d://keypressDemo.ini");
	if (infor != "")
	{
		m_editRate.SetWindowTextA(infor);
		rate = atof(infor);
		m_rate = rate;
	}
	::GetPrivateProfileString(APP_NAME, "m_screenWidth", "", infor.GetBufferSetLength(256), 256, "d://keypressDemo.ini");
	if (infor != "")
	{
		SetDlgItemText(IDC_EDIT7,infor);
		m_screenWidth = atol(infor);
		 
	}
	::GetPrivateProfileString(APP_NAME, "bHuangLong", "", infor.GetBufferSetLength(256), 256, "d://keypressDemo.ini");
	if (infor != "")
	{
		//SetDlgItemText(IDC_EDIT7, infor);
		bHuangLong = atol(infor);
		if(bHuangLong==1)
		((CButton*)(GetDlgItem(IDC_CHECK1)))->SetCheck(1);

	}
	SetTimer(0, TIMER_LENGTH, NULL);
	::SetWindowPos((HWND)(this->m_hWnd), HWND_TOP, 0, 0, 800, 600, SWP_SHOWWINDOW| SWP_NOSIZE);
	OnBnClickedButtonKeypress4();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CVC_DemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
		AfxMessageBox("�˿��Ѿ��򿪣���رն˿������´�");
	}
	msdk_handle = M_Open(1);
	M_ResolutionUsed(msdk_handle, 1920, 1080);
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		AfxMessageBox("�˿ڴ�ʧ�ܣ���ȷ������USB�豸�Ѿ����ϵ���");
	}
	else {
		GetDlgItem(IDC_BUTTON_OPEN)->EnableWindow(false);
	}
}

void CVC_DemoDlg::OnBnClickedButtonClose()
{
	ShellExecute(this->m_hWnd, "open", "https://github.com/sunyuzhe114/KeyPressDemo/blob/master/x64/Release/VC_Demo.exe", NULL, NULL, SW_SHOWMAXIMIZED);
}

//�Ѿ���changeUser_And_Login_Thread�ϲ�
DWORD WINAPI    LoginUser_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;
	do {

		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, 1385 / rate, 110 / rate);
			RetSw = M_DelayRandom(800, 1000);
		}
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftClick(msdk_handle, 1);
		/*RetSw = M_LeftDown(msdk_handle );
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftUp(msdk_handle );*/

		RetSw = M_DelayRandom(3000, 4000);
		//��5�ż���
		RetSw = M_KeyPress(msdk_handle, Keyboard_5, 1);
		RetSw = M_DelayRandom(400, 600);
		infor += "��5�ż�\r\n";
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		//���ȷ��
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(1800, 2000);
		infor += "��ȷ��\r\n";
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		//����3�ε��� 
		for (int i = 0; i < 3; i++)
		{
			RetSw = M_MouseWheel(msdk_handle, 1);
			RetSw = M_DelayRandom(1000, 1200);
		}

		infor += "����3��\r\n";
		pDlg->m_editLogInfor.SetWindowTextA(infor);

		//�ߵ��ص�
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1518) / rate), (int)((277) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);

		infor += "ѡ������\r\n";
		pDlg->m_editLogInfor.SetWindowTextA(infor);

		//���ȷ��
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(1800, 2000);
		infor += "��ȷ��\r\n";
		pDlg->m_editLogInfor.SetWindowTextA(infor);


		//�ߵ��ص�
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = my_M_MoveTo(msdk_handle, (int)((1753) / rate), (int)((503) / rate));
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = M_RightClick(msdk_handle, 1);
			RetSw = M_DelayRandom(800, 1000);

		}
		infor += "�ߵ��ص�\r\n";
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		//��� 
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftDoubleClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftDoubleClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
		infor += "���\r\n";
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
		pDlg->begin_check_game();

	} while (0);
	return 0;
}
DWORD WINAPI    changeUser_And_Login_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;

	do {

		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, 1385 / rate, 110 / rate);;
			RetSw = M_DelayRandom(800, 1000);
		}
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftClick(msdk_handle, 1);
		/*RetSw = M_LeftDown(msdk_handle );
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftUp(msdk_handle );*/

		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
		RetSw = M_DelayRandom(2800, 3000);
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1517) / rate), (int)((454) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(3800, 4500);
		addLog("���ѡ����ɫ");
		if (bStop)break;

		if (bChangeUser == true)
		{
			addLog("���·����Ҽ�");
			RetSw = M_KeyPress(msdk_handle, Keyboard_RightArrow, 1);
			RetSw = M_DelayRandom(2100, 2300);

		}


		RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
		RetSw = M_DelayRandom(1400, 1600);
		RetSw = M_DelayRandom(6000, 7000);
		//��5�ż���
		if (bStop)break;
		RetSw = M_KeyPress(msdk_handle, Keyboard_5, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_DelayRandom(400, 600);
		addLog("��5�ż�");
		//���ȷ��
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(1800, 2000);
		 
		addLog("��ȷ��");
		if (bStop)break;
		//����3�ε��� 
		for (int i = 0; i < 3; i++)
		{
			RetSw = M_MouseWheel(msdk_handle, -1);
			RetSw = M_DelayRandom(2200, 2500);
		}
		if (bStop)break;
		 
		addLog("����3��");
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		//�ߵ��ص�
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1518) / rate), (int)((277) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftClick(msdk_handle, 1);



		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		
		addLog("ѡ������");
		if (bStop)break;
		//���ȷ��
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);

		addLog("���ȷ��");
		if (bStop)break;
		RetSw = M_DelayRandom(1800, 2000);
		addLog("�����Ļ���￪ʼ�ƶ�"); 

		if (bStop)break;
		//�ߵ��ص�
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = my_M_MoveTo(msdk_handle, (int)((1753) / rate), (int)((503) / rate));
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = M_RightClick(msdk_handle, 1);
			RetSw = M_DelayRandom(800, 1000);

		}
		addLog("�һ��ߵ��ص�");
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = my_M_MoveTo(msdk_handle, (int)((1660) / rate), (int)((503) / rate));
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = M_RightClick(msdk_handle, 1);
			RetSw = M_DelayRandom(800, 1000);

		}
		if (bStop)break; 
		addLog("�һ��ߵ��ص�"); 

		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		addLog("���������ƶ�һ��");
		RetSw = M_KeyPress(msdk_handle, Keyboard_Douhao, 1);
		RetSw = M_DelayRandom(800, 1000);
		addLog("�����ƶ�һ��");
		RetSw = M_KeyPress(msdk_handle, Keyboard_Douhao, 1);
		RetSw = M_DelayRandom(800, 1000);
		addLog("�����ƶ�һ��");
		RetSw = M_KeyPress(msdk_handle, Keyboard_Douhao, 1);
		RetSw = M_DelayRandom(800, 1000);
		addLog("�����ƶ�����");
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_JuHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_JuHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);


		if (bStop)break;
		//��� 
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			if (pDlg->bHuangLong == true)
			{
				RetSw = my_M_MoveTo(msdk_handle, (int)((1710) / rate), (int)((405) / rate));
				addLog("�ƶ�������");
			}
			else
			{
				RetSw = my_M_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
				addLog("�ƶ�������");
			}
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftDoubleClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftDoubleClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
		
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(2200, 4200);

		RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
		addLog("���¿ո�׼����Ϸ");

		pDlg->begin_check_game();
		if (bStop)break;
	} while (0);
	infor += "exit \r\n";
	pDlg->m_editLogInfor.SetWindowTextA(infor);
	return 0;
}

DWORD WINAPI    checkThread_Game(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_BIGSKILL = 0;

	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = my_M_MoveTo(msdk_handle, 1385 / rate, 110 / rate);
		RetSw = M_DelayRandom(800, 1000);
	}
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftClick(msdk_handle, 1);
	/*RetSw = M_LeftDown(msdk_handle );
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftUp(msdk_handle );*/

	RetSw = M_DelayRandom(3000, 4000);

	while (bStop == false)
	{
		if (GetTickCount() - m_dTimeBegin > m_timeLimit * 60 * 1000)
		{
			bStop = true;
		}


		if (bStop)break;

		RetSw = M_DelayRandom(800, 1000);
		CString strInfor;
		strInfor.Format("%ld��", (GetTickCount() - m_dTimeBegin) / 1000);
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
		RetSw = M_KeyUp(msdk_handle, Keyboard_z);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		if (bStop)break;

		RetSw = M_KeyPress(msdk_handle, Keyboard_e, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
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
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_g, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		pDlg->saveScreen();
		checkGame_state();
		strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
		pDlg->m_editLogInfor.SetWindowTextA(strInfor);

		if (Game_state == 100 || Game_state == 200)
		{ 
			CString strInfor;
			strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
			pDlg->m_editLogInfor.SetWindowTextA(strInfor);


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
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_DelayRandom(200, 500);
			//������Ե��һ���ٰ�F10

			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = my_M_MoveTo(msdk_handle, 1385 / rate, 110 / rate);
				RetSw = M_DelayRandom(800, 1000);
			}
			RetSw = M_DelayRandom(800, 1000);
			RetSw = M_LeftClick(msdk_handle, 1);


			RetSw = M_DelayRandom(200, 500);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_F10, 1);
			RetSw = M_DelayRandom(3100, 4500);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);

			if (Game_state == 100)
			{
				bStop = true;
				if (bStop)break;

			}
			else
			{

				continue;
			}
		}

		if (bStop)break;
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);

		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_v);

		RetSw = M_DelayRandom(400, 600);

		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_s, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		if (bStop)break;

		//�����ʱ���޷�ͨ�أ�Ҫ��һ�δ���

		if (GetTickCount() - m_dTimeBeginPress_BIGSKILL > 120000)
		{
			CString infor;
			infor.Format("m_dTimeBeginPress_BIGSKILL continue remains %ld \r\n", pDlg->m_checkTimes - Global_checkTime);
			pDlg->m_editLogInfor.SetWindowTextA(infor);
			m_dTimeBeginPress_BIGSKILL = GetTickCount();
			RetSw = M_KeyPress(msdk_handle, Keyboard_a, 1);
			RetSw = M_DelayRandom(1400, 2600);/*
			RetSw = M_KeyPress(msdk_handle, Keyboard_PageDown, 1);
			RetSw = M_DelayRandom(400, 600);*/
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
			//RetSw = M_KeyPress(msdk_handle, Keyboard_F10, 1);
			RetSw = M_DelayRandom(2400, 2600);
		}

		pDlg->saveScreen();
	    checkGame_state(); 
		strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
		pDlg->m_editLogInfor.SetWindowTextA(strInfor);

		if (Game_state == 100|| Game_state == 200)
		{

			
			CString strInfor;
			strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
			pDlg->m_editLogInfor.SetWindowTextA(strInfor);

			
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
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_KeyPress(msdk_handle, Keyboard_F10, 1);
			RetSw = M_DelayRandom(3100, 4500);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe); 

			if (Game_state == 100)
			{
				bStop = true;
				if (bStop)break;

			}
			else
			{

			continue;
			}
		}
		CPoint cp = findImage("d://ingame.png", 340, 3, 460, 114);
		if (cp.x != 0 && cp.y != 0)
		{

			CString str;
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

			str.Format("%s==>%s (%ld,%ld)\n", tt, "����������Ϸ�� 0", cp.x, cp.y);
			addLog(str);
			not_in_game_time = 0;
		}
		else
		{

			CString str;
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S"); 
			str.Format("%s==>%s (%ld,%ld)\n", tt, "δ����Ϸ�� 0", cp.x, cp.y);
			addLog(str);
			not_in_game_time++;
			if (not_in_game_time >= 2)
			{
				not_in_game_time = 0;
				Game_state = 300;
				break;
			}
		}


		RetSw = M_DelayRandom(700, 800);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;

		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(600, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_t, 1);
		RetSw = M_DelayRandom(600, 1000);
		RetSw = M_KeyPress(msdk_handle, Keyboard_s, 1);
		RetSw = M_DelayRandom(600, 1000);
		if (bStop)break;


	 

		RetSw = M_ReleaseAllKey(msdk_handle);
	}
	RetSw = M_KeyPress(msdk_handle, Keyboard_F12, 1);
	RetSw = M_DelayRandom(600, 1000);

	pDlg->m_editLogInfor.SetWindowTextA("exit checkThread_Game \r\n");

	 

	RetSw = M_ReleaseAllKey(msdk_handle);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);

	RetSw = M_DelayRandom(10000, 15000);
	if (Game_state == 300)
	{
		pDlg->OnBnClickedButtonKeypress5();
	}
	else if (Global_checkTime < pDlg->m_checkTimes && bFullStop == false&& Game_state <= 200)
	{
		CString infor;
		infor.Format("stop continue remains %ld \r\n", pDlg->m_checkTimes-Global_checkTime);
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		Global_checkTime++;
		//�������ֽ⶯������I�� �����ֽ�
		//�������ֽ⶯������I�� �����ֽ�
		RetSw = M_DelayRandom(800, 1000);
		//�������ֽ⶯������I�� �����ֽ�
		RetSw = M_KeyPress(msdk_handle, Keyboard_DanYinHao, 1);
		RetSw = M_DelayRandom(2200, 3000);
		//�ֽ�װ��
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 600);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1566) / rate), (int)((336) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);

		//ȫ���ֽ�װ��
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 600);
			RetSw = my_M_MoveTo(msdk_handle, (int)((1371) / rate), (int)((347) / rate));
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);
		pDlg->saveScreen();
		CPoint pt = findSureButton_state();
		 
		infor.Format("����ȷ����ť%d,%d", pt.x, pt.y);
		addLog(infor);
		if (pt.x == -1)
		{
			addLog("����ȷ����ťfail");
		}
		else
		{
			pt.x += 15;
			pt.y += 15;
			//ȷ�Ϸ���װ��
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = M_DelayRandom(500, 600);
				//����ʹ�õ��Ǿ�������
				RetSw = M_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
				RetSw = M_DelayRandom(500, 600);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = M_LeftClick(msdk_handle, 1);

			infor.Format("�������� MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
			addLog(infor);

			RetSw = M_DelayRandom(800, 1000);
			RetSw = M_DelayRandom(6200, 9000);


			RetSw = M_DelayRandom(6200, 9000);
		}
		RetSw = M_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
		RetSw = M_DelayRandom(2200, 3000);

		pDlg->OnBnClickedButtonKeypress5();
	}
	else
	{
		CString infor;
		infor.Format("full stop  time remains %ld \r\n", pDlg->m_checkTimes - Global_checkTime);

		pDlg->m_editLogInfor.SetWindowTextA(infor);
	}

	return 0;
}


void CVC_DemoDlg::saveScreen()
{
	CWnd *pDesktop = this->GetDesktopWindow();
	CDC *pdeskdc = pDesktop->GetDC();
	CRect re;
	//��ȡ���ڵĴ�С
	pDesktop->GetClientRect(&re);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pdeskdc, re.Width(), re.Height());
	//����һ�����ݵ��ڴ滭��
	CDC memorydc;
	memorydc.CreateCompatibleDC(pdeskdc);
	//ѡ�л���
	CBitmap *pold = memorydc.SelectObject(&bmp);
	//����ͼ��
	memorydc.BitBlt(0, 0, re.Width(), re.Height(), pdeskdc, 0, 0, SRCCOPY);
	//��ȡ���λ�ã�Ȼ��������ͼ��
	CPoint po;
	GetCursorPos(&po);
	HICON hinco = (HICON)GetCursor();
	memorydc.DrawIcon(po.x - 10, po.y - 10, hinco);
	//ѡ��ԭ���Ļ���
	memorydc.SelectObject(pold);
	BITMAP bit;
	bmp.GetBitmap(&bit);
	//���� ͼ���С����λ��byte��
	DWORD size = bit.bmWidthBytes * bit.bmHeight;
	LPSTR lpdata = (LPSTR)GlobalAlloc(GPTR, size);
	//�����Ǵ���һ��bmp�ļ��ı����ļ�ͷ
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
	//д���ļ�
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

void CVC_DemoDlg::begin_check_game()
{
m_dTimeBegin = GetTickCount();
	UpdateData(true);
	m_timeLimit = m_intMinute;//����
	bStop = false;
	bFullStop = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}  
	Sleep(3000);


	HANDLE hThread = CreateThread(NULL, 0, checkThread_Game, (LPVOID)msdk_handle, 0, NULL);

	GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(false);
}
void CVC_DemoDlg::OnBnClickedButtonKeypress()
{
	OnBnClickedButtonKeypress4();
	begin_check_game();
	

}

void CVC_DemoDlg::OnBnClickedButtonMover()
{
	m_checkTimes =6;
	UpdateData(false);
}

void CVC_DemoDlg::OnBnClickedButtonMoveto()
{ 
	//if (msdk_handle == INVALID_HANDLE_VALUE) {
	//	AfxMessageBox("��δ�򿪶˿ڣ����ȴ򿪶˿�");
	//	return;
	//}
	//unsigned int RetSw;
	//RetSw = M_ResetMousePos(msdk_handle);
	//RetSw = M_DelayRandom(800, 1000);
	//RetSw = M_MoveR(msdk_handle, 100 / rate, 100 / rate);
	//RetSw = M_LeftClick(msdk_handle, 1);
	m_checkTimes = 0;
	UpdateData(false);
}

void CVC_DemoDlg::OnBnClickedButtonGetmousepos()
{
	Global_checkTime = 0;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		AfxMessageBox("��δ�򿪶˿ڣ����ȴ򿪶˿�");
		return;
	}
	CString strtemp;
	unsigned int RetSw;
	int x_pos, y_pos;
	RetSw = M_GetCurrMousePos(msdk_handle, &x_pos, &y_pos);
	if (RetSw != 0) {
		AfxMessageBox("��ȡ����������");
		return;
	}
	else {
		SetTimer(0, TIMER_LENGTH, NULL);
		m_listLog.ResetContent();
		m_editLogInfor.SetWindowTextA("reset");
	}
}


void CVC_DemoDlg::OnBnClickedButtonKeypress3()
{
	bStop = true;
	bFullStop = true;
}


void CVC_DemoDlg::OnBnClickedButtonOpen2()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	} 

	


	saveScreen();
	 
	 int RetSw =  M_DelayRandom(2800, 3000);

	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = my_M_MoveTo(msdk_handle, 1385 / rate, 110 / rate);
		RetSw = M_DelayRandom(800, 1000);
	}
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftClick(msdk_handle, 1);
	
	
	RetSw = M_DelayRandom(800, 1000);
	//�������ֽ⶯������I�� �����ֽ�
	RetSw = M_KeyPress(msdk_handle, Keyboard_DanYinHao, 1);
	RetSw = M_DelayRandom(2200, 3000);
	//�ֽ�װ��
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_DelayRandom(500, 600);
		RetSw = my_M_MoveTo(msdk_handle, (int)((1566) / rate), (int)((336) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);

	//ȫ���ֽ�װ��
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_DelayRandom(500, 600);
		RetSw = my_M_MoveTo(msdk_handle, (int)((1371) / rate), (int)((351) / rate));
		RetSw = M_DelayRandom(500, 600);
	}
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	saveScreen();
	CPoint pt= findSureButton_state();
	
	CString infor;
	infor.Format("����ȷ����ť%d,%d",pt.x,pt.y);
	addLog(infor);
	if (pt.x == -1)
	{
		addLog("����ȷ����ťfail");
	}
	else
	{
		pt.x += 15;
		pt.y += 15;
		//ȷ�Ϸ���װ��
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 600);
			//����ʹ�õ��Ǿ�������
			RetSw = M_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
			RetSw = M_DelayRandom(500, 600);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftClick(msdk_handle, 1);
		
		infor.Format("�������� MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
		addLog(infor);
		
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(6200, 9000);


		RetSw = M_DelayRandom(6200, 9000);
	}
	RetSw = M_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
	RetSw = M_DelayRandom(2200, 3000);

	


}


void CVC_DemoDlg::OnBnClickedButtonKeypress5()
{
	m_dTimeBegin = GetTickCount();
	UpdateData();
	m_timeLimit = m_intMinute;//����
	bStop = false;
	bChangeUser = true;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	 
	Sleep(3000);


	HANDLE hThread = CreateThread(NULL, 0, changeUser_And_Login_Thread, (LPVOID)msdk_handle, 0, NULL);

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
	while (pMainWnd)//���о����еĴ���
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



			//	if((strClassName=="#32770"&&(text.Fin2d("����")!=-1||text.Find("microsoft visual c++")!=-1||text.Find("microsoft internet")!=-1||text.Find("��ȫ����")!=-1||text.Find("��ȫ��Ϣ")!=-1||text.Find("windows internet explorer")!=-1||text.Find("���ӵ�")!=-1) 19910205LGD
			//		||(strClassName=="Internet Explorer_TridentDlgFrame")))//||text.Find("object Error")!=-1
			if (strClassName.Find(m_edit_keyword) != -1 || text.Find(m_edit_keyword) != -1)

			{//�����ie������������з�����
				CRect rect;
				::GetWindowRect(pMainWnd->m_hWnd, &rect);
				CString strWindow;
				//strWindow.Format("%lx,%s",pMainWnd->m_hWnd,strClassName); 
				if (rect.Width() > 500 && rect.Height() > 500)
				{
					dleft = rect.left;
					dtop = rect.top;
					strWindow.Format(_T("%lx,(%ld,%ld,%ld,%ld)"), pMainWnd->m_hWnd, rect.left, rect.top, rect.Width(), rect.Height());
					m_listWindow.AddString(strWindow);
					m_listWindow.SetCurSel(0);
				}

				//HWND h=::GetWindow(pMainWnd->m_hWnd,GW_CHILD);
				//while(h)//����ie�����ڲ��ṹ
				//{//��洰���Լ��󲿷ֶ�����վ�ĵ������ڶ���һ����ͬ�ص�
				//	//ֻ���ı���ʾ����û�й�����
				//	//��һ������Ϊie���������Ƿ��й�����������֪���Ƿ�Ϊ��洰��

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
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}

void CVC_DemoDlg::playerlogin()
{
	m_dTimeBegin = GetTickCount(); 
	UpdateData();
	m_timeLimit = m_intMinute;//����
	bStop = false;
	bChangeUser = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	 
	Sleep(3000);


	HANDLE hThread = CreateThread(NULL, 0, changeUser_And_Login_Thread, (LPVOID)msdk_handle, 0, NULL);// TODO: �ڴ���ӿؼ�֪ͨ����������

}
void CVC_DemoDlg::OnBnClickedButtonKeypress6()
{
	OnBnClickedButtonKeypress4();
	playerlogin();
	}


void CVC_DemoDlg::OnEnChangeEdit5()
{
	UpdateData();
	rate = m_rate;
	CString rr;
	rr.Format("%0.2lf", rate);
	CWinApp* pApp = AfxGetApp();
	::WritePrivateProfileString(APP_NAME, "m_rate", rr, "d://keypressDemo.ini");
}


void CVC_DemoDlg::OnEnChangeEdit6()
{
	UpdateData();

}


void CVC_DemoDlg::OnBnClickedButtonMover2()
{
	m_checkTimes = 9;
	UpdateData(false);
}


void CVC_DemoDlg::OnEnChangeEdit1()
{
	
}

void CVC_DemoDlg::setShowWindowSize(int posX, int posY, int width, int height)
{
	CWnd* pMainWnd = AfxGetMainWnd()->GetWindow(GW_HWNDFIRST);


	DWORD timebegin = GetTickCount();
	bool bFindWindows = false;
	while (pMainWnd)//���о����еĴ���
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



			//	if((strClassName=="#32770"&&(text.Find("����")!=-1||text.Find("microsoft visual c++")!=-1||text.Find("microsoft internet")!=-1||text.Find("��ȫ����")!=-1||text.Find("��ȫ��Ϣ")!=-1||text.Find("windows internet explorer")!=-1||text.Find("���ӵ�")!=-1) 
			//		||(strClassName=="Internet Explorer_TridentDlgFrame")))//||text.Find("object Error")!=-1
			if (strClassName.Find(m_edit_keyword) != -1 || text.Find(m_edit_keyword) != -1)

			{//�����ie������������з�����

				CString strSelectedWindow, strWindow;
				if (m_listWindow.GetCurSel() != -1)
					m_listWindow.GetText(m_listWindow.GetCurSel(), strSelectedWindow);
				strWindow.Format(_T("%lx"), pMainWnd->m_hWnd);

				if (strSelectedWindow.Find(strWindow) != -1)
				{

					HWND h = ::GetWindow(pMainWnd->m_hWnd, GW_CHILD);
					while (h)//����ie�����ڲ��ṹ
					{//��洰���Լ��󲿷ֶ�����վ�ĵ������ڶ���һ����ͬ�ص�
						//ֻ���ı���ʾ����û�й�����
						//��һ������Ϊie���������Ƿ��й�����������֪���Ƿ�Ϊ��洰��

						GetClassName(h, strClassName.GetBufferSetLength(100), 100);
						::GetWindowText(h, text.GetBufferSetLength(256), 256);
						//TRACE("%s %s\r\n",strClassName,text);


						h = ::GetWindow(h, GW_HWNDNEXT);

					}

					//	 ::PostMessage(pMainWnd->m_hWnd,WM_KEYDOWN,VK_RIGHT, 0x014d0001);
					//	 ::PostMessage(pMainWnd->m_hWnd,WM_KEYUP,VK_RIGHT,0xc14d0001);//MAKEKEYLPARAM(0x4d, 1)
				   //	::keybd_event(

				   /*	INPUT   keyInput[2];
					   memset(&keyInput,   0,   sizeof(INPUT)*2);

					   keyInput[0].type   =   INPUT_KEYBOARD;
					   keyInput[0].ki.wVk   =   VK_RIGHT;
					   keyInput[0].ki.dwFlags   =   WM_KEYDOWN;

					   keyInput[1].type   =   INPUT_KEYBOARD;
					   keyInput[1].ki.wVk   =   VK_RIGHT;
					   keyInput[1].ki.dwFlags   =   WM_KEYUP;

					   SendInput(2,   &keyInput[0],   sizeof(INPUT)); */
					   //::MoveWindow(pMainWnd->m_hWnd,0,0,512,384,true);
					::ShowWindow(pMainWnd->m_hWnd, SW_NORMAL);
					::MoveWindow((HWND)(pMainWnd->m_hWnd), posX, posY, width, height, true);
					//::SetWindowPos((HWND)(pMainWnd->m_hWnd), HWND_TOP, 0, 0, 400, 300, SWP_SHOWWINDOW);
				}

			}

		}
		if (::IsWindow(pMainWnd->m_hWnd))
			pMainWnd = pMainWnd->GetWindow(GW_HWNDNEXT);
	}

}
void CVC_DemoDlg::OnBnClickedButtonKeypress7()
{
	setShowWindowSize(GetSystemMetrics(SM_CXSCREEN) - 800, 0, 800, 600);
}


void CVC_DemoDlg::OnLbnSelchangeList1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
}


void CVC_DemoDlg::OnBnClickedButtonGetmousepos2()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	unsigned int RetSw;
	RetSw = M_ResetMousePos(msdk_handle);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = my_M_MoveTo(msdk_handle, 100, 100);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftClick(msdk_handle, 2);
	RetSw = M_DelayRandom(800, 1000);
}


void CVC_DemoDlg::OnBnClickedButtonKeypress8()
{
	ShellExecute(this->m_hWnd, "open", "��������.bat", NULL, "D:\\", SW_SHOWMAXIMIZED);
}


void CVC_DemoDlg::OnEnChangeEdit7()
{
	UpdateData(); 
	CString rr;
	rr.Format("%d", m_screenWidth);
	CWinApp* pApp = AfxGetApp();
	::WritePrivateProfileString(APP_NAME, "m_screenWidth", rr, "d://keypressDemo.ini");
}


void CVC_DemoDlg::OnBnClickedButtonOpen3()
{
	//img = cv::imread("d:\\s.bmp", IMREAD_COLOR);
	//try {

	//	Mat NewImg = img(Rect(40, -5, 80, 30));
	//}
	//catch (Exception &e)
	//{
	//	addLog("catch error");
	//}
	//
	//checkGame_state();
//	OnBnClickedButtonKeypress4();
	pDlg->saveScreen();
	CPoint cp = findImage("d://ingame.png", 340, 3, 460,114);
	if (cp.x != 0 && cp.y != 0)
	{

	CString str;
	CTime t = CTime::GetCurrentTime();
	CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

	str.Format("%s==>%s (%ld,%ld)\n", tt, "����������Ϸ�� 0",cp.x,cp.y);
	addLog(str);
	}
	else 
	{

		CString str;
		CTime t = CTime::GetCurrentTime();
		CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

		str.Format("%s==>%s (%ld,%ld)\n", tt, "δ����Ϸ�� 0", cp.x, cp.y);
		addLog(str);
	}
	
	//CStdioFile file;
	//if (file.Open(_T("d:\\log.txt"), CFile::typeText | CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate, NULL))
	//{
	//	file.SeekToEnd();
	//	file.WriteString(str);
	//	file.Close();
	//}
}


void CVC_DemoDlg::OnLbnSelchangeList2()
{
	CString infor;
	m_listLog.GetText(m_listLog.GetCurSel(), infor);
	m_editLogInfor.SetWindowTextA(infor);
	
}


void CVC_DemoDlg::OnBnClickedCheck1()
{

	CString strInfor;
	UpdateData();
	CString rr;
	rr.Format("%d", bHuangLong);
	::WritePrivateProfileString(APP_NAME, "bHuangLong", rr, "d://keypressDemo.ini");
	if (bHuangLong)
	{
		strInfor.Format("change to huanglong %d", bHuangLong);
		addLog(strInfor);
		
	}
	else
	{
		strInfor.Format("change to qinglong  %d", bHuangLong);
		addLog(strInfor);
	}

}


void CVC_DemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	 
	CTime time = CTime::GetCurrentTime();
	if (time.GetHour() ==6 && time.GetMinute() <10)
	{
		 
		CWnd* pMainWnd = AfxGetMainWnd()->GetForegroundWindow();

		CString strClassName;
		CString text;
		CString strCurrentWindow;
		GetClassName(pMainWnd->m_hWnd, strClassName.GetBufferSetLength(100), 100);
		::GetWindowText(pMainWnd->m_hWnd, text.GetBufferSetLength(256), 256);
		if (text.Find(m_edit_keyword) != -1)
		{
			addLog("����");
			KillTimer(0);
			playerlogin();
		}
		else
		{
			if (text.Find("�����") != -1)
			{
				::PostMessage(pMainWnd->m_hWnd, WM_CLOSE, NULL, NULL);
			}
			addLog("lost focus " + text);
		}


		
		
	}
	else
	{
	/*	CWnd* pMainWnd = AfxGetMainWnd()->GetForegroundWindow();
		 
		CString strClassName;
		CString text;
		CString strCurrentWindow;
		GetClassName(pMainWnd->m_hWnd, strClassName.GetBufferSetLength(100), 100);
		::GetWindowText(pMainWnd->m_hWnd, text.GetBufferSetLength(256), 256);
		if (text.Find(m_edit_keyword) != -1)
		{
			addLog("check " + text);
		}
		else
		{
			if (text.Find("�����") != -1)
			{
				::PostMessage(pMainWnd->m_hWnd, WM_CLOSE, NULL, NULL);
			}
			addLog("lost focus " + text);
		}*/

		
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CVC_DemoDlg::OnClose()
{
	KillTimer(0);

	CDialogEx::OnClose();
}

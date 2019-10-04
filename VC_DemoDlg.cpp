
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
#define APP_NAME "notepad++"
 
int  Game_state = -1;//Game_state = 100;用户已经用完，200，还可以再玩
int not_in_game_time = 0;//检测到未在游戏中次数
// CVC_DemoDlg 对话框
int Global_checkTime = 0;
double rate = 2.5;//* dbZoomScale
CString str_matchineName;
CVC_DemoDlg* pDlg;
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
Mat mask;
Mat result;
const char* image_window = "Source Image";
const char* result_window = "Result window";
bool MatchingMethod();
int match_method = TM_SQDIFF_NORMED;
int max_Trackbar = 5;

//! [declare]
float dbZoomScale = 1.0;
/// Function Headers
int checkGame_state();
bool bGonlyDnf = false;
bool b_NUM4_Dnf = false;
DWORD WINAPI    changeUser_Thread(LPVOID pp);
void addLog(CString infor);
void addLog_important(CString infor);
void addLog(CString infor)
{
	CTime time = CTime::GetCurrentTime();
	CString date = time.Format("%H:%M:%S");
	CString strshow = date + "   " + infor;
	pDlg->m_listLog.InsertString(0, strshow);
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
///移动到游戏窗口的相对位置
int move_to_relativePos(HANDLE m_hdl, int x, int y)
{
	M_ResetMousePos(m_hdl);
	long changeX =(x + dleft) / rate;
	long changeY =( y +dtop) / rate;
	CString infor;
	infor.Format("move mouse to %ld,%ld", x + changeX, y + changeY);
	addLog(infor);
	return M_MoveTo(m_hdl,  changeX,  changeY);

}
bool isDNFWindow()
{
	if (bGonlyDnf==FALSE)return true;

	CWnd* pMainWnd = AfxGetMainWnd()->GetForegroundWindow();

	CString strClassName;
	CString text;
	CString strCurrentWindow;
	GetClassName(pMainWnd->m_hWnd, strClassName.GetBufferSetLength(100), 100);
	::GetWindowText(pMainWnd->m_hWnd, text.GetBufferSetLength(256), 256);
	if (text.Find("勇士") != -1)
	{

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
//int my_hook_KeyPress(HANDLE msdk_handle, Keyboard_KongGe, 1);
int my_hook_KeyPress(HANDLE m_hdl, int HidKeyCode, int Nbr)
{
	if (isDNFWindow())
	{
		 M_KeyDown(m_hdl, HidKeyCode);
		 M_DelayRandom(300, 400);
		return M_KeyUp(m_hdl, HidKeyCode);
			//return M_KeyPress(m_hdl, HidKeyCode, Nbr);
	}
	else
	{
		addLog("当前窗口非dnf");
		return 0;
	}
}
int my_hook_right_Click(HANDLE m_hdl, int times)
{
	int x_pos, y_pos;
	int RetSw = M_GetCurrMousePos(m_hdl, &x_pos, &y_pos);

	//x_pos *= rate;
	//y_pos *= rate;


	CString strinfor;
	strinfor.Format("on %ld,%ld,R click win(%ld,%ld,%ld,%ld)", x_pos, y_pos, dleft, dtop, dleft + 800, dtop + 600);
	addLog(strinfor);
	//if (x_pos > dleft && x_pos<(dleft + 800) && y_pos>dtop && y_pos < (dtop + 600))
	if(true)
	{
		if(isDNFWindow())
		return M_RightClick(m_hdl, times);
	} 
	else
	{
		addLog("beyond scope");
		return 0;
	}
	return 0;
}
int my_hook_left_Click(HANDLE m_hdl, int times)
{
	int x_pos, y_pos;
	int RetSw = M_GetCurrMousePos(m_hdl, &x_pos, &y_pos);

	x_pos *= rate;
	y_pos *= rate;

	
	CString strinfor;
	strinfor.Format("on %ld,%ld,lclick win(%ld,%ld,%ld,%ld)",x_pos,y_pos,dleft,dtop,dleft+800,dtop+600);
	addLog(strinfor);
	 
	if (isDNFWindow())
	return M_LeftClick(m_hdl, times);
	 
	return 0;
}
int my_hook_MoveTo(HANDLE m_hdl, int x, int y)
{
	 

	 
	CString infor;
	
	if (x < 500 && y < 100)
	{
		infor.Format("cancel move mouse to %ld,%ld", x, y);
		addLog(infor);
		return 0;
	}
	else
	{	infor.Format("move mouse to %ld,%ld", x  , y  );
		addLog(infor);
		return M_MoveTo(m_hdl, x  , y  );//return M_MoveTo3(m_hdl, x  , y );
	}
	
	
}
int my_new_MoveTo(HANDLE m_hdl, int x, int y)
{
	int SCREEN_CX = pDlg->m_screenWidth;//#1920  
	long changeX = (dleft - (SCREEN_CX - 800)) / rate;
	long changeY = dtop / rate;



	//long changeX = (dleft - 1120)/rate;
	//long changeY = dtop/rate ;
	CString infor;
	infor.Format("my_new_MoveTo mouse to %ld,%ld", x + changeX, y + changeY);
	addLog(infor);
	//return M_MoveTo(m_hdl, x + changeX, y + changeY);
	return M_MoveTo3(m_hdl, (x + changeX)*rate, (y + changeY) * rate);
	//return M_MoveTo(m_hdl, x  , y );
}

//找指定图位置，letf,top,right,buttom在指定范围 = > 发现在在游戏中 0 (1463, 54)
CPoint findImage(string strPath_findImage, int left, int top, int right, int bottom)
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
		//matchloc.x ,matchloc.y是相对于系统坐标
		//changeX,changeY 相对游戏框坐标
		long changeX = matchLoc.x - dleft;
		long changeY = matchLoc.y - dtop;
		img.release();
		templ.release();
		/* long changeX = dleft - 1120;
		long changeY = dtop - 0; */
		//390,54 1030 54
		infor.Format("X=%ld,Y=%ld,x=%ld,y=%ld,maxVal=%0.2lf,", changeX, changeY, matchLoc.x, matchLoc.y, maxVal);
		addLog("game " + infor);
		//if ((matchLoc.x - changeX) > 1255 && (matchLoc.x - changeX) < 1615 && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <=430 && maxVal > 0.5 )
		if (changeX > left && changeX < right && changeY >= top && changeY <= bottom && maxVal > 0.5)
		{

			pt.x = matchLoc.x;
			pt.y = matchLoc.y;
			return pt;
		}
		else
		{
			infor += "未检测到按钮 ";
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
	catch (Exception& e)
	{
		addLog("error findimg");
	}
	pt.x = 0;
	pt.y = 0;
	return pt;

}
CPoint findSureButton_state()
{
	CPoint pt(0, 0);
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
		img.release();
		templ.release();

		long changeX = dleft - (SCREEN_CX - 800);
		long changeY = dtop - 0;
		/* long changeX = dleft - 1120;
		long changeY = dtop - 0; */
		infor.Format("x=%ld,y=%ld,maxVal=%0.2lf,changeX=%ld,changeY=%ld", matchLoc.x, matchLoc.y, maxVal, changeX, changeY);
		addLog("find sure button" + infor);
		//if ((matchLoc.x - changeX) > 1255 && (matchLoc.x - changeX) < 1615 && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <=430 && maxVal > 0.5 )
		if ((matchLoc.x - changeX) > (SCREEN_CX - 695) && (matchLoc.x - changeX) < (SCREEN_CX - 305) && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <= 430 && maxVal > 0.5)
		{

			pt.x = matchLoc.x;
			pt.y = matchLoc.y;
			return pt;
		}
		else
		{
			infor += "未检测到按钮 ";
			addLog(infor);
			bResult = -1; Game_state = -1;
			pt.x = 0;
			pt.y = 0;
			return pt;
		}

		return pt;
	}
	catch (Exception& e)
	{
		addLog("catch error findSureButton_state");
	}
	pt.x = 0;
	pt.y = 0;
	return pt;

}
//0表示已经帐号用光
//1表示还可以再玩
//-1表示未检测到
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
				//取色分析
			try
			{

				Mat NewImg = img(Rect(matchLoc.x, matchLoc.y, 80, 30));
				Mat means, stddev, covar;
				cv:Scalar tempVal = cv::mean(NewImg);
				float matMean = tempVal.val[0];
				CString strResult;
				//42 34 56//not change to grey
				// 43 29 45 //grey



				strResult.Format("means  : %0.0f %0.0f %0.0f\n", tempVal.val[0], tempVal.val[1], tempVal.val[2]);//RGB三通道，所以均值结果是3行一列

				//imshow("test", NewImg);
					if (tempVal.val[1] <= 30 && tempVal.val[2] <= 46||pDlg->bOnlyForTest)
				//if(TRUE)
				{
					CString str;
					CTime t = CTime::GetCurrentTime();
					CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

					str.Format("%s==>%s\r\n", tt, "帐号已经使用完成 0");
					CStdioFile file;
					if (file.Open(_T("d:\\log.txt"), CFile::typeText | CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate, NULL))
					{
						file.SeekToEnd();
						file.WriteString(str);
						file.Close();
					}
					addLog_important(str);
					addLog("帐号已经使用完成 0");
					Game_state = 100;
					bResult = 100;// 
				}
				else
				{
					infor = "检测到游戏还可以再玩 1";
					addLog(infor);
					bResult = 200;//检测到游戏还可以再玩
					Game_state = 200;
					//	pDlg->MessageBoxA("检测到游戏还可以再玩,", "error", MB_OK);
				}
			}
			catch (exception& e)
			{
				img.release();
				templ.release();
				infor += "error execptio -1 ";
				addLog(infor);
				return -1;
			}
		}
		else
		{
			infor += "未检测到窗口 -1 ";
			addLog(infor);
			//pDlg->MessageBoxA("未检测到窗口,", "error", MB_OK);
			bResult = -1; Game_state = -1;
		}
		img.release();
		templ.release();
		return bResult;
	}
	catch (Exception& e)
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
	catch (Exception& e)
	{
		addLog("error MatchingMethod");
		return FALSE;
	}
}
DWORD WINAPI    fenjie_zhuangbei(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
 
	pDlg->saveScreen();

	int RetSw = M_DelayRandom(2800, 3000);
	CPoint pt = findImage("d://close.png", 380, 440, 390, 460);
	if (pt.x != 0 && pt.y != 0)
	{

		CString str = "";
		CTime t = CTime::GetCurrentTime();
		CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
		str.Format("%s==>%s (%ld,%ld)\n", tt, "发现广告关闭 ", pt.x, pt.y);

		//str.Format("%s==>%s (%ld,%ld)\n", tt, "发现在在游戏中 0",cp.x,cp.y);
		addLog(str);
	}
	else
	{

		CString str = "";
		CTime t = CTime::GetCurrentTime();
		CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

		str.Format("%s==>%s (%ld,%ld)\n", tt, "未现广告 0", pt.x, pt.y);
		addLog(str);
	}

	CString infor;
	infor.Format("查找确定按钮%d,%d", pt.x, pt.y);
	addLog(infor);
	if (pt.x == 0)
	{
		addLog("查找确定按钮fail");
	}
	else
	{
		pt.x += 18;
		pt.y += 12;
		//确认分析装备
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 800);
			//这里使用的是绝对坐标
			//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
			//这里也可以写个定值
			RetSw = move_to_relativePos(msdk_handle, pt.x-dleft, pt.y-dtop);
			RetSw = M_DelayRandom(500, 600);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);

		infor.Format("绝对坐标 MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
		addLog(infor);

		RetSw = M_DelayRandom(800, 1000);
	}

	RetSw = M_DelayRandom(2800, 3000);


	//for (int i = 0; i < 1; i++)
	//{
	//	RetSw = M_ResetMousePos(msdk_handle);
	//	RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);
	//	RetSw = M_DelayRandom(800, 1000);
	//}
	addLog("点击游戏窗口");
	RetSw = move_to_relativePos(msdk_handle, 50, 50);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftClick(msdk_handle, 2);


	RetSw = M_DelayRandom(1800, 2000);
	//这里加入分解动作，按I键 ，开分解
	if (b_NUM4_Dnf)
	{
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_4, 1);
	}
	else
	{
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_DanYinHao, 1);
		RetSw = M_DelayRandom(2200, 3000);
		//分解装备
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 600);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1566) / rate), (int)((336) / rate));
			RetSw = move_to_relativePos(msdk_handle, 440, 335 );
			RetSw = M_DelayRandom(900, 1600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
	}
	
	

	//全部分解装备
	for (int i = 0; i < 1; i++)
	{
		RetSw = M_ResetMousePos(msdk_handle);
		RetSw = M_DelayRandom(500, 600);
		//RetSw = my_new_MoveTo(msdk_handle, (int)((1371) / rate), (int)((351) / rate));
		RetSw = move_to_relativePos(msdk_handle, 240, 350);
		RetSw = M_DelayRandom(1500, 2600);
	}
	RetSw = my_hook_left_Click(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	pDlg->saveScreen();
	pt = findSureButton_state();

	infor.Format("查找确定按钮%d,%d", pt.x, pt.y);
	addLog(infor);
	if (pt.x == 0)
	{
		addLog("查找确定按钮fail");
	}
	else
	{
		pt.x += 18; 
		pt.y += 12;
		//确认分析装备
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 600);
			//这里使用的是绝对坐标
			//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
			RetSw = move_to_relativePos(msdk_handle, pt.x - dleft, pt.y - dtop);;
			RetSw = M_DelayRandom(500, 600);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);

		infor.Format("绝对坐标 MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
		addLog(infor);

		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(6200, 9000);


		RetSw = M_DelayRandom(6200, 9000);
	}
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
	RetSw = M_DelayRandom(2200, 3000);


	return 0;
}
CVC_DemoDlg::CVC_DemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVC_DemoDlg::IDD, pParent)
	, m_intMinute(120)
	, m_edit_keyword(_T("勇士")
	)
	, m_checkTimes(24)
	, m_screenWidth(1920)
	, bHuangLong(FALSE)
	, bOnlyForTest(FALSE)
	, m_matchinename(_T(""))
	, bOnlyDNF(FALSE)
	, m_bUseing_Num4(FALSE)
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
	DDX_Check(pDX, IDC_CHECK2, bOnlyForTest);
	DDX_Text(pDX, IDC_EDIT8, m_matchinename);
	DDX_Check(pDX, IDC_CHECK3, bOnlyDNF);
	DDX_Check(pDX, IDC_CHECK4, m_bUseing_Num4);
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
	ON_BN_CLICKED(IDC_BUTTON_KEY_ON_SCREEN, &CVC_DemoDlg::OnBnClickedButtonKeyOnScreen)
	ON_BN_CLICKED(IDC_CHECK2, &CVC_DemoDlg::OnBnClickedCheck2)
	ON_EN_CHANGE(IDC_EDIT8, &CVC_DemoDlg::OnEnChangeEdit8)
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_CHECK3, &CVC_DemoDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CVC_DemoDlg::OnBnClickedCheck4)
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
		SetDlgItemText(IDC_EDIT7, infor);
		m_screenWidth = atol(infor);

	}
	::GetPrivateProfileString(APP_NAME, "bHuangLong", "", infor.GetBufferSetLength(256), 256, "d://keypressDemo.ini");
	if (infor != "")
	{
		//SetDlgItemText(IDC_EDIT7, infor);
		bHuangLong = atol(infor);
		if (bHuangLong == 1)
			((CButton*)(GetDlgItem(IDC_CHECK1)))->SetCheck(1);

	}
	if (bGonlyDnf == true)
		((CButton*)(GetDlgItem(IDC_CHECK3)))->SetCheck(1);

	::GetPrivateProfileString(APP_NAME, "b_NUM4_Dnf", "", infor.GetBufferSetLength(256), 256, "d://keypressDemo.ini");
	if (infor != "")
	{
		//SetDlgItemText(IDC_EDIT7, infor);
		b_NUM4_Dnf = atol(infor);
		if (b_NUM4_Dnf == 1)
			((CButton*)(GetDlgItem(IDC_CHECK4)))->SetCheck(1);

	}
	if (b_NUM4_Dnf == true)
		((CButton*)(GetDlgItem(IDC_CHECK4)))->SetCheck(1);
	::GetPrivateProfileString(APP_NAME, "m_matchname", "", infor.GetBufferSetLength(256), 256, "d://keypressDemo.ini");
	if (infor != "")
	{
		SetDlgItemText(IDC_EDIT8, infor);
		m_matchinename = infor;
		 

	}
	SetTimer(0, TIMER_LENGTH, NULL);
	::SetWindowPos((HWND)(this->m_hWnd), HWND_TOP, 0, 0, 800, 600, SWP_SHOWWINDOW | SWP_NOSIZE);
	OnBnClickedButtonKeypress4();
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
	ShellExecute(this->m_hWnd, "open", "https://github.com/sunyuzhe114/KeyPressDemo/blob/master/x64/Release/VC_Demo.exe", NULL, NULL, SW_SHOWMAXIMIZED);
}

 
DWORD WINAPI    changeUser_And_Login_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;

	do {

		/*for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);;
			RetSw = M_DelayRandom(800, 1000);
		}*/
		move_to_relativePos(msdk_handle, 50, 50);
		RetSw = M_DelayRandom(800, 1000);
		 
		RetSw = M_LeftClick(msdk_handle, 1);
		/*RetSw = M_LeftDown(msdk_handle );
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftUp(msdk_handle );*/

		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
		RetSw = M_DelayRandom(2800, 3000);
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1517) / rate), (int)((454) / rate));
			RetSw = move_to_relativePos(msdk_handle, 380, 460);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);
		RetSw = M_DelayRandom(3800, 4500);
		addLog("点击选定角色");
		if (bStop)break;

		if (bChangeUser == true)
		{
			addLog("按下方向右键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_RightArrow, 1);
			RetSw = M_DelayRandom(2100, 2300);

		}


		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 1);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break; 
		RetSw = M_DelayRandom(1000, 1100); 
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);

		//按5号键，
		if (bStop)break;
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 1);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		addLog("按5号键");
		//点击确认
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = move_to_relativePos(msdk_handle, 370, 320);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);
		RetSw = M_DelayRandom(1800, 2000);

		addLog("按确定");
		if (bStop)break;
		//滚动3次到了 
		for (int i = 0; i < 3; i++)
		{
			RetSw = M_MouseWheel(msdk_handle, -1);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100); 
		}
		if (bStop)break;

		addLog("滚轮3次");
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		//走到地点
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1510) / rate), (int)((277) / rate));
			RetSw = move_to_relativePos(msdk_handle, 385, 270);
			RetSw = M_DelayRandom(500, 600);
		}
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = my_hook_left_Click(msdk_handle, 1);


		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(2000, 2100);

		addLog("选定坐标");
		if (bStop)break;
		//点击确认
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = move_to_relativePos(msdk_handle, 370, 320);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);
		RetSw = M_DelayRandom(500, 1100);
		//点击确认二次,保证点上
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = move_to_relativePos(msdk_handle, 370, 320);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);

		addLog("点击确认");
		if (bStop)break;
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		addLog("点击屏幕人物开始移动");

		if (bStop)break;
		//走到地点
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1753) / rate), (int)((503) / rate));
				RetSw = move_to_relativePos(msdk_handle, 700, 500);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_right_Click(msdk_handle, 1);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100); 

		}
		addLog("右击走到地点");
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1660) / rate), (int)((503) / rate));
				RetSw = move_to_relativePos(msdk_handle, 577, 460);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_right_Click(msdk_handle, 1);
			RetSw = M_DelayRandom(800, 1000);

		}
		if (bStop)break;
		addLog("右击走到地点");

		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		addLog("键盘向左移动一步");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		addLog("向左移动一步");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		addLog("向左移动一步");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		addLog("向右移动二步");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_JuHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_JuHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 1);


		if (bStop)break;
		//点击 
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			if (pDlg->bHuangLong == true)
			{
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1710) / rate), (int)((405) / rate));
				RetSw = move_to_relativePos(msdk_handle, 570, 390);
				addLog("移动到黄龙");
				
			}
			else
			{
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
				RetSw = move_to_relativePos(msdk_handle, 210, 440);
				addLog("移动到青龙");
			}
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = M_LeftDoubleClick(msdk_handle, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		RetSw = M_LeftDoubleClick(msdk_handle, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);

		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);	
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);	
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);
		if (bStop)break;
		RetSw = M_DelayRandom(500, 1000);

		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 1);
		addLog("按下空格准备游戏");

		pDlg->begin_check_game();
		if (bStop)break;
	} while (0);

	addLog("登录线程停止 exit");
	return 0;
}
DWORD WINAPI    testThread_Game(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_BIGSKILL = 0;
	CString text;
	pDlg->m_editLogInfor.GetWindowTextA(text);
	//char* inputText = "sun8010305652";\USES_CONVERSION;
	USES_CONVERSION;
	char* inputText = new char[256];
	strcpy(inputText, text);
	RetSw = M_KeyInputString(msdk_handle, inputText, strlen(inputText));
	// RetSw = M_KeyInputString(msdk_handle, text.GetBuffer(0, strlen(inputText));
	/*RetSw = M_DelayRandom(3000, 4000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_s, 1);
	RetSw = M_DelayRandom(300, 1000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_u, 1);
	RetSw = M_DelayRandom(400, 600);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_n, 1);
	RetSw = M_DelayRandom(400, 600);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_8, 1);
	RetSw = M_DelayRandom(300, 1000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_0, 1);
	RetSw = M_DelayRandom(400, 600);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_1, 1);
	RetSw = M_DelayRandom(300, 1000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_0, 1);
	RetSw = M_DelayRandom(400, 600);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_3, 1);
	RetSw = M_DelayRandom(300, 1000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_0, 1);
	RetSw = M_DelayRandom(400, 600);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 1);
	RetSw = M_DelayRandom(300, 1000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_6, 1);
	RetSw = M_DelayRandom(400, 600);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 1);
	RetSw = M_DelayRandom(300, 1000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_2, 1);
	RetSw = M_DelayRandom(400, 600);
	 */

	return 0;


}
DWORD WINAPI    checkThread_Game(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_BIGSKILL = 0;

	//for (int i = 0; i < 1; i++)
	//{
	//	RetSw = M_ResetMousePos(msdk_handle);
	//	//RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);
	//	RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);
	//	RetSw = M_DelayRandom(800, 1000);
	//}
	RetSw = move_to_relativePos(msdk_handle, 50, 50);
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
		strInfor.Format("%ld秒", (GetTickCount() - m_dTimeBegin) / 1000);
		pDlg->m_editLog.SetWindowText(strInfor);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(300, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_w, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_r, 1);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_z);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
		if (bStop)break;

		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_e, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_w, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_d, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_g, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
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
			not_in_game_time = 0;
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_PageDown, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			if (bStop)break;
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = M_DelayRandom(200, 500);
			//这里可以点击一下再按F10




			RetSw = M_DelayRandom(200, 500);
			if (Game_state == 200)
			{
				/*for (int i = 0; i < 1; i++)
				{
					RetSw = M_ResetMousePos(msdk_handle);
					RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);
					RetSw = M_DelayRandom(800, 1000);
				}*/
				RetSw = move_to_relativePos(msdk_handle, 50, 50);
				RetSw = M_DelayRandom(800, 1000);
				RetSw = M_LeftClick(msdk_handle, 1);
				RetSw = M_DelayRandom(200, 500);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F10, 1);
				RetSw = M_DelayRandom(3100, 4500);
				RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
				RetSw = M_DelayRandom(400, 600);
				RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			}

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
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		if (bStop)break;
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);

		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = M_KeyUp(msdk_handle, Keyboard_v);

		RetSw = M_DelayRandom(400, 600);

		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_s, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_f, 1);
		if (bStop)break;

		//如果长时间无法通关，要放一次大招

		if (GetTickCount() - m_dTimeBeginPress_BIGSKILL > 120000)
		{
			CString infor;
			infor.Format("m_dTimeBeginPress_BIGSKILL continue remains %ld \r\n", pDlg->m_checkTimes - Global_checkTime);
			pDlg->m_editLogInfor.SetWindowTextA(infor);
			m_dTimeBeginPress_BIGSKILL = GetTickCount();
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_a, 1);
			RetSw = M_DelayRandom(1400, 2600);/*
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_PageDown, 1);
			RetSw = M_DelayRandom(400, 600);*/
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			if (bStop)break;
			//RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F10, 1);
			RetSw = M_DelayRandom(2400, 2600);
		}

		pDlg->saveScreen();
		checkGame_state();
		strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
		pDlg->m_editLogInfor.SetWindowTextA(strInfor);

		if (Game_state == 100 || Game_state == 200)
		{


			CString strInfor;
			strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
			pDlg->m_editLogInfor.SetWindowTextA(strInfor);
			not_in_game_time = 0;

			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_PageDown, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			if (bStop)break;
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);

			if (Game_state == 200)
			{
				RetSw = M_DelayRandom(200, 500);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F10, 1);
				RetSw = M_DelayRandom(3100, 4500);
				RetSw = M_KeyDown(msdk_handle, Keyboard_KongGe);
				RetSw = M_DelayRandom(400, 600);
				RetSw = M_KeyUp(msdk_handle, Keyboard_KongGe);
			}

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
		CPoint cp = findImage("d://ingamenew.png", 340, 40, 400, 60);
		if (cp.x != 0 && cp.y != 0)
		{
			//·¢ÏÖÔÚÔÚÓÎÏ·ÖÐ 0 (1400,51)


			CString str;

			str.Format("%s (%ld,%ld)\n", "发现在游戏中 0", cp.x, cp.y);
			addLog(str);
			not_in_game_time = 0;
		}
		else
		{

			CString str;
			str.Format(" %s (%ld,%ld)\n", "未在游戏中 0", cp.x, cp.y);
			addLog(str);
			not_in_game_time++;
			if (not_in_game_time >= 3)
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
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_f, 1);
		RetSw = M_DelayRandom(600, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_t, 1);
		RetSw = M_DelayRandom(600, 1000);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_s, 1);
		RetSw = M_DelayRandom(600, 1000);
		if (bStop)break;




		RetSw = M_ReleaseAllKey(msdk_handle);
	}
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F12, 1);
	RetSw = M_DelayRandom(600, 1000);

	pDlg->m_editLogInfor.SetWindowTextA("exit checkThread_Game \r\n");



	RetSw = M_ReleaseAllKey(msdk_handle);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);

	RetSw = M_DelayRandom(10000, 15000);
	if (Game_state == 300)
	{
		pDlg->playerlogin();
	}
	else if (Global_checkTime <= pDlg->m_checkTimes && bFullStop == false && Game_state <= 200)
	{
		CString infor;
		infor.Format("stop continue remains %ld \r\n", pDlg->m_checkTimes - Global_checkTime);
		pDlg->m_editLogInfor.SetWindowTextA(infor);
		Global_checkTime++;

		int RetSw = M_DelayRandom(4800, 6000);
		fenjie_zhuangbei(msdk_handle);

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
	CWnd* pDesktop = this->GetDesktopWindow();
	CDC* pdeskdc = pDesktop->GetDC();
	CRect re;
	//获取窗口的大小
	pDesktop->GetClientRect(&re);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pdeskdc, re.Width(), re.Height());
	//创建一个兼容的内存画板
	CDC memorydc;
	memorydc.CreateCompatibleDC(pdeskdc);
	//选中画笔
	CBitmap* pold = memorydc.SelectObject(&bmp);
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
		& pbitinfo, DIB_RGB_COLORS);
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

void CVC_DemoDlg::begin_check_game()
{
	m_dTimeBegin = GetTickCount();
	//UpdateData(true);
	m_timeLimit = m_intMinute;//分钟
	bStop = false;
	bFullStop = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	Sleep(3000);
	if (msdk_handle != INVALID_HANDLE_VALUE)
	{

		HANDLE hThread = CreateThread(NULL, 0, checkThread_Game, (LPVOID)msdk_handle, 0, NULL);

		GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(false);
	}
}
void CVC_DemoDlg::OnBnClickedButtonKeypress()
{
	OnBnClickedButtonKeypress4();

	begin_check_game();


}

void CVC_DemoDlg::OnBnClickedButtonMover()
{
	m_checkTimes = 6;
	UpdateData(false);
}

void CVC_DemoDlg::OnBnClickedButtonMoveto()
{
	//if (msdk_handle == INVALID_HANDLE_VALUE) {
	//	AfxMessageBox("还未打开端口，请先打开端口");
	//	return;
	//}
	//unsigned int RetSw;
	//RetSw = M_ResetMousePos(msdk_handle);
	//RetSw = M_DelayRandom(800, 1000);
	//RetSw = M_MoveR(msdk_handle, 100 / rate, 100 / rate);
	//RetSw = my_hook_left_Click(msdk_handle, 1);
	m_checkTimes = 0;
	UpdateData(false);
}

void CVC_DemoDlg::OnBnClickedButtonGetmousepos()
{
	UpdateData(TRUE);
	Global_checkTime = 0;
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
		SetTimer(0, TIMER_LENGTH, NULL);
		m_listLog.ResetContent();
		m_editLogInfor.SetWindowTextA("reset");
		CString infor;
		infor.Format("full stop  time remains %ld \r\n", pDlg->m_checkTimes - Global_checkTime);

		pDlg->m_editLogInfor.SetWindowTextA(infor);
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
	HANDLE hThread = CreateThread(NULL, 0, fenjie_zhuangbei, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

	//fenjie_zhuangbei(msdk_handle);
	


}


void CVC_DemoDlg::OnBnClickedButtonKeypress5()
{
	m_dTimeBegin = GetTickCount();
	//UpdateData();
	m_timeLimit = m_intMinute;//分钟
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

		if (::IsWindowVisible(pMainWnd->m_hWnd))
		{
			
			if (strClassName.Find(m_edit_keyword) != -1 || text.Find(m_edit_keyword) != -1 || text.Find("notpad++") != -1)
			{
				addLog("window  " + text +" class " +strClassName);
			}
			else
			{
				if (text != "")
				{
					addLog("hide window  " + text + " class " + strClassName);
					::PostMessage(pMainWnd->m_hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				}
			}
		}

		strCurrentWindow = text;

		text.MakeLower();
		CString strInforText = text;


		if (::IsWindowVisible(pMainWnd->m_hWnd))// || true)
		{



			//	if((strClassName=="#32770"&&(text.Fin2d("错误")!=-1||text.Find("microsoft visual c++")!=-1||text.Find("microsoft internet")!=-1||text.Find("安全警报")!=-1||text.Find("安全信息")!=-1||text.Find("windows internet explorer")!=-1||text.Find("连接到")!=-1) 19910205LGD
			//		||(strClassName=="Internet Explorer_TridentDlgFrame")))//||text.Find("object Error")!=-1
			if (strClassName.Find(m_edit_keyword) != -1 || text.Find(m_edit_keyword) != -1)

			{//如果是ie窗口则继续进行分析。
				CRect rect;
				::GetWindowRect(pMainWnd->m_hWnd, &rect);
				CString strWindow;
				//strWindow.Format("%lx,%s",pMainWnd->m_hWnd,strClassName); 
				if (rect.Width() > 500 && rect.Height() == 600)
				{
					dleft = rect.left;
					dtop = rect.top;
					strWindow.Format(_T("%lx,(%ld,%ld,%ld,%ld)"), pMainWnd->m_hWnd, rect.left, rect.top, rect.Width(), rect.Height());
					m_listWindow.AddString(strWindow);
					m_listWindow.SetCurSel(0);
				}
				else
				{
					
				}

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
			else
			{
				
			}

		}
		if (::IsWindow(pMainWnd->m_hWnd))
		{
			pMainWnd = pMainWnd->GetWindow(GW_HWNDNEXT);
		}
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

void CVC_DemoDlg::playerlogin()
{
	m_dTimeBegin = GetTickCount();
	//UpdateData();
	m_timeLimit = m_intMinute;//分钟
	bStop = false;
	bChangeUser = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}

	Sleep(3000);


	HANDLE hThread = CreateThread(NULL, 0, changeUser_And_Login_Thread, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

}
void CVC_DemoDlg::minized_all_the_other_windows()
{
	 
}

void CVC_DemoDlg::OnBnClickedButtonKeypress6()
{
	
	minized_all_the_other_windows();
	
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	OnBnClickedButtonKeypress4();
	if (msdk_handle != INVALID_HANDLE_VALUE)
	{
		playerlogin();
	}
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
	m_checkTimes = 15;
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



			//	if((strClassName=="#32770"&&(text.Find("错误")!=-1||text.Find("microsoft visual c++")!=-1||text.Find("microsoft internet")!=-1||text.Find("安全警报")!=-1||text.Find("安全信息")!=-1||text.Find("windows internet explorer")!=-1||text.Find("连接到")!=-1) 
			//		||(strClassName=="Internet Explorer_TridentDlgFrame")))//||text.Find("object Error")!=-1
			if (strClassName.Find(m_edit_keyword) != -1 || text.Find(m_edit_keyword) != -1)

			{//如果是ie窗口则继续进行分析。

				CString strSelectedWindow, strWindow;
				if (m_listWindow.GetCurSel() != -1)
					m_listWindow.GetText(m_listWindow.GetCurSel(), strSelectedWindow);
				strWindow.Format(_T("%lx"), pMainWnd->m_hWnd);

				if (strSelectedWindow.Find(strWindow) != -1)
				{

					HWND h = ::GetWindow(pMainWnd->m_hWnd, GW_CHILD);
					while (h)//分析ie窗口内部结构
					{//广告窗口以及大部分恶意网站的弹出窗口都有一个共同特点
						//只有文本显示区，没有工具栏
						//若一个窗口为ie窗口则看其是否有工具栏，及可知它是否为广告窗口

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
	// TODO: 在此添加控件通知处理程序代码
}


void CVC_DemoDlg::OnBnClickedButtonGetmousepos2()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	unsigned int RetSw;
	RetSw = M_ResetMousePos(msdk_handle);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = my_new_MoveTo(msdk_handle, 100, 100);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = my_hook_left_Click(msdk_handle, 2);
	RetSw = M_DelayRandom(800, 1000);
}


void CVC_DemoDlg::OnBnClickedButtonKeypress8()
{
	ShellExecute(this->m_hWnd, "open", "立即下线.bat", NULL, "D:\\", SW_SHOWMAXIMIZED);
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
	if (isDNFWindow() == TRUE)
	{
		addLog("是DNF");
	}
	else
	{
		addLog("No DNF");
	}
	//
	//23:17:48   ²éÕÒÈ·¶¨°´Å¥1018,399
	//M_ResetMousePos(msdk_handle);
	//M_DelayRandom(800, 1000);
	//M_MoveTo3(msdk_handle, int(960), int(10));
	//M_DelayRandom(800, 1000);
	// 
	// 
	//return;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}

	//
	//int RetSw = M_DelayRandom(800, 1000);
	//RetSw = M_LeftClick(msdk_handle,1);
	//
	//M_MoveTo3(msdk_handle, 250, 250);
	//RetSw = M_LeftClick(msdk_handle,1);
	//RetSw = M_DelayRandom(800, 1000);
	////move_to_relativePos(msdk_handle, 50, 50);
	//return;

	/*RetSw = M_LeftDown(msdk_handle );
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftUp(msdk_handle );*/



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


	//#测试是否在游戏中
	saveScreen();//390,54
	CPoint cp = findImage("d://close.png", 380, 440, 390, 460);
	/////CPoint cp = findImage("d://ingamenew.png", 340, 40, 400, 60);
	if (cp.x != 0 && cp.y != 0)
	{

		CString str = "";
		CTime t = CTime::GetCurrentTime();
		CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
		str.Format("%s==>%s (%ld,%ld)\n", tt, "发现广告关闭 0", cp.x, cp.y);

		//str.Format("%s==>%s (%ld,%ld)\n", tt, "发现在在游戏中 0",cp.x,cp.y);
		addLog(str);
	}
	else
	{

		CString str = "";
		CTime t = CTime::GetCurrentTime();
		CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

		str.Format("%s==>%s (%ld,%ld)\n", tt, "未在游戏中 0", cp.x, cp.y);
		addLog(str);
	}

	for (int i = 0; i < 200; i++)
	{
		M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
		M_DelayRandom(5800, 6100);
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
	if (time.GetHour() == 6 && time.GetMinute() < 10)
	{

		CWnd* pMainWnd = AfxGetMainWnd()->GetForegroundWindow();

		CString strClassName;
		CString text;
		CString strCurrentWindow;
		GetClassName(pMainWnd->m_hWnd, strClassName.GetBufferSetLength(100), 100);
		::GetWindowText(pMainWnd->m_hWnd, text.GetBufferSetLength(256), 256);
		if (text.Find(m_edit_keyword) != -1)
		{
			Global_checkTime = 0;
			addLog("早起动");
			KillTimer(0);
			playerlogin();
		}
		else
		{
			if (text.Find("迷你版") != -1)
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
				if (text.Find("迷你版") != -1)
				{
					::PostMessage(pMainWnd->m_hWnd, WM_CLOSE, NULL, NULL);
				}
				addLog("lost focus " + text);
			}*/

		addLog("ontimer check ");
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CVC_DemoDlg::OnClose()
{
	KillTimer(0);

	CDialogEx::OnClose();
}


void CVC_DemoDlg::OnBnClickedButtonKeyOnScreen()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	Sleep(3000);

	HANDLE hThread = CreateThread(NULL, 0, testThread_Game, (LPVOID)msdk_handle, 0, NULL);
}


void CVC_DemoDlg::OnBnClickedCheck2()
{ 
	UpdateData(); 
	 
	 
}


void CVC_DemoDlg::OnEnChangeEdit8()
{
	UpdateData();
	 
	CString rr;
	rr.Format("%s", m_matchinename);
	CWinApp* pApp = AfxGetApp();
	::WritePrivateProfileString(APP_NAME, "m_matchname", rr, "d://keypressDemo.ini");
}
void CVC_DemoDlg::OnOK()
{
	return;
}
 
BOOL CVC_DemoDlg::PreTranslateMessage(MSG * pMsg)
{
	//屏蔽ESC关闭窗体/
		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE;
	//屏蔽回车关闭窗体,但会导致回车在窗体上失效.
		//if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN && pMsg->wParam) return TRUE;
		else
		    return CDialog::PreTranslateMessage(pMsg);
}


LRESULT CVC_DemoDlg::OnNcHitTest(CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int ret = CDialog::OnNcHitTest(point);

	//if语句的前两行是用来禁止改变大小的，最后一行是用来禁止移动的
	if (HTTOP == ret || HTBOTTOM == ret || HTLEFT == ret || HTRIGHT == ret
		|| HTBOTTOMLEFT == ret || HTBOTTOMRIGHT == ret || HTTOPLEFT == ret || HTTOPRIGHT == ret || HTCAPTION == ret)
		return HTCLIENT;

	return ret; 
}


void CVC_DemoDlg::OnBnClickedCheck3()
{
	UpdateData();
	bGonlyDnf = bOnlyDNF;
}


void CVC_DemoDlg::OnBnClickedCheck4()
{UpdateData();
	b_NUM4_Dnf= m_bUseing_Num4;
	CString strInfor;
	UpdateData();
	CString rr;
	rr.Format("%d", b_NUM4_Dnf);
	::WritePrivateProfileString(APP_NAME, "b_NUM4_Dnf", rr, "d://keypressDemo.ini");
	// TODO: 在此添加控件通知处理程序代码
}

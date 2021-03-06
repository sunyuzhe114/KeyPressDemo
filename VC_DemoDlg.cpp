﻿
// VC_DemoDlg.cpp : 实现文件
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
 
#include <afxsock.h> 
CString strVserion = "20200926_12"; 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define APP_NAME "notepad++"
#include <Wininet.h>

#pragma comment(lib, "Wininet.lib")

bool m_exit = false;
CPoint findImage(string strPath_findImage, int left, int top, int right, int bottom);
CPoint findImage_in_subrect(string strPath_findImage, int left, int top, int right, int bottom);
CPoint findImageWholeWindow_in_subrect(string strPath_findImage, int left, int top, int right, int bottom);
CString GetLocalIP();
bool m_bMaxmizieWegame_window = false;
int one_charactor_fighttime = 0;
int MAX_ONE_CHARACTOR_PLAYTIME = 17;
BOOL HttpRequestGet(IN const CString& sHomeUrl, IN const CString& sPageUrl, OUT CString& sResult)
{
	LONG nPort = 80;
	HINTERNET hInternet;
	DWORD nGetSize;
	LPSTR lpszData = NULL;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;

	hInternet = InternetOpen(_T("Mozilla/4.0 (compatible; Indy Library)"),
		INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (NULL == hInternet)
	{
		sResult.Format(_T("Open link error. ErrCode=[%u]"), GetLastError());
		InternetCloseHandle(hInternet);
		return FALSE;
	}
	// 打开http session   
	HINTERNET hSession = InternetConnect(hInternet, sHomeUrl,
		(INTERNET_PORT)nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

	CString sHtmlHeader;
	sHtmlHeader = _T("Content-Type: application/x-www-form-urlencoded\r\n");
	sHtmlHeader += _T("Accept: text/html, */*\r\n");
	sHtmlHeader += _T("User-Agent: Mozilla/4.0 (compatible;Indy Library)\r\n");

	LPSTR pszResponse = new char[640 * 1024];
	memset(pszResponse, 0, 640 * 1024);

	HINTERNET hRequest = HttpOpenRequest(hSession, _T("GET"), sPageUrl,
		_T("HTTP/1.1"), _T(""), 0, INTERNET_FLAG_NO_AUTH |
		INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_CACHE_WRITE, 0);

	int iTimeout = 10000;
	InternetSetOption(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT,
		&iTimeout, sizeof(iTimeout));
	InternetSetOption(hRequest, INTERNET_OPTION_SEND_TIMEOUT,
		&iTimeout, sizeof(iTimeout));
	InternetSetOption(hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT,
		&iTimeout, sizeof(iTimeout));
	InternetSetOption(hRequest, INTERNET_OPTION_DATA_SEND_TIMEOUT,
		&iTimeout, sizeof(iTimeout));
	InternetSetOption(hRequest, INTERNET_OPTION_DATA_RECEIVE_TIMEOUT,
		&iTimeout, sizeof(iTimeout));
	InternetSetOption(hRequest, INTERNET_OPTION_LISTEN_TIMEOUT,
		&iTimeout, sizeof(iTimeout));

	BOOL bResult = HttpSendRequest(hRequest, sHtmlHeader.GetBuffer(),
		sHtmlHeader.GetLength(), _T(""), 0);
	sHtmlHeader.ReleaseBuffer();

	if (FALSE == bResult)
	{
		sResult.Format(_T("Send request error. ErrCode=[%u]"), GetLastError());

		InternetCloseHandle(hRequest);
		InternetCloseHandle(hSession);
		InternetCloseHandle(hInternet);

		delete[]pszResponse;

		return FALSE;
	}

	nGetSize = 0;
	// 循环读取数据    
	do
	{ // 检查在http response 还有多少字节可以读取  
		if (!InternetQueryDataAvailable(hRequest, &dwSize, 0, 0))
		{
			break;
		}
		// 读取数据  
		if (FALSE == InternetReadFile(hRequest,
			(LPVOID)&pszResponse[nGetSize], dwSize, &dwDownloaded))
		{
			nGetSize += dwSize;
			if (dwDownloaded == 0 || nGetSize > 600 * 1024)
			{// 没有剩余数据  
				break;
			}
		}
	} while (FALSE);

	pszResponse[nGetSize] = 0;
	sResult = ATL::CA2T(pszResponse);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hSession);
	InternetCloseHandle(hInternet);

	delete[]pszResponse;

	if (sResult.Find(_T("<html>")) != -1)
	{
		sResult = _T("An unknown error occurred.");
		return FALSE;
	}

	return TRUE;
}
int  Game_state = -1;//Game_state = 100;用户已经用完，200，还可以再玩
int not_in_game_time = 0;//检测到未在游戏中次数
// CVC_DemoDlg 对话框
int Global_checkTime = 0;
double rate = 2.58;//* dbZoomScale
CString str_matchineName;
CVC_DemoDlg* pDlg;
bool bStop = false;
bool bFullStop = false;
bool bChangeUser = true;
bool bChangetofirstUser = false;
DWORD m_dTimeBegin = 0;
DWORD m_timeLimit = 0;
const UINT TIMER_LENGTH = 600000;
LONG dleft = 566;
LONG dtop = 0;
/// Global Variables
bool use_mask = false;
Mat img;
Mat templ;
Mat mask;
Mat result;
const char* image_window = "Source Image";
const char* result_window = "Result window";
 
int match_method = TM_SQDIFF_NORMED;
int max_Trackbar = 5;
CString strAppPath="D:";


//! [declare]
float dbZoomScale = 1.0;
/// Function Headers
int checkGame_state();
bool bGonlyDnf = false;
bool bslowmode = false;
bool b_SiWangTa_Dnf = false;
DWORD WINAPI    changeUser_Thread(LPVOID pp);
void addLog(CString infor);
void addLog_important(CString infor);
DWORD WINAPI    change_to_first_player(LPVOID pp);
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
	CString strshow;
	strshow.Format("(%ld) %s  %s", pDlg->m_list_time_log.GetCount()+1, date, infor);
	pDlg->m_list_time_log.InsertString(0, strshow);

}
int move_to_Pos(HANDLE m_hdl, int x, int y)
{
	M_ResetMousePos(m_hdl);
	long changeX = (x    ) / rate;
	long changeY = (y    ) / rate;
	CString infor;
	infor.Format("move_to_Pos to %ld,%ld", x + changeX, y + changeY);
	addLog(infor);
	return M_MoveTo(m_hdl, changeX, changeY);

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
bool checkDNFWindow()
{
	 

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
DWORD WINAPI    StartServer(LPVOID lParam)
{
	
	//初始化Winscok
	if (!AfxSocketInit())
	{
		AfxMessageBox("init socket error");
		return 1;
	}

	m_exit = false;
	CVC_DemoDlg* aDlg = (CVC_DemoDlg*)lParam;
	 
	UINT nPort = atoi("5005");

	//socket------------------------------------------------
	CSocket aSocket, serverSocket;
	//最好不要使用aSocket.Create创建，因为容易会出现10048错误
	if (!aSocket.Socket())
	{
		char szError[256] = { 0 };
		sprintf(szError, "Create Faild: %d", GetLastError());
		AfxMessageBox(szError);
		return 1;
	}

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);

	//设置Socket的选项, 解决10048错误必须的步骤
	aSocket.SetSockOpt(SO_REUSEADDR, (void*)&bOptVal, bOptLen, SOL_SOCKET);

	//绑定端口
	if (!aSocket.Bind(nPort))
	{
		char szError[256] = { 0 };
		sprintf(szError, "Bind Faild: %d", GetLastError());
		AfxMessageBox(szError);
		return 1;
	}
	//监听
	if (!aSocket.Listen(5))
	{
		char szError[256] = { 0 };
		sprintf(szError, "Listen Faild: %d", GetLastError());
		AfxMessageBox(szError);
		return 1;
	}

	CString strText;
	addLog ("Server Start!  ");
	while (!m_exit)
	{
		//接收外部连接
		if (!aSocket.Accept(serverSocket))
		{
			continue;
		}
		else
		{
			char szRecvMsg[256] = { 0 };
			char szOutMsg[256] = { 0 };

			//接收客户端内容:阻塞
			serverSocket.Receive(szRecvMsg, 256);

			sprintf(szOutMsg, "r->%s(v->%s): %s", pDlg->m_matchinename,pDlg->m_current_Version,szRecvMsg);
			addLog (szRecvMsg);
			//aDlg->SetDlgItemText(IDC_EDIT3, szRecvMsg);
			CString strMsg ;
			strMsg.Format("%s|%s",pDlg->m_matchinename, szRecvMsg);
			//sprintf(strMsg.GetBuffer(0), "%s", szRecvMsg); 
			
			//发送内容给客户端
			serverSocket.Send(szOutMsg, strlen(szOutMsg));
			 
			if (strMsg == "stop")
			{
				aDlg->OnBnClickedButtonKeypress3();
			}
			if (strMsg == "start" || strMsg == "启动")
			{
				aDlg->SetTimer(6, 2000, NULL);
				
			}
			if (strMsg == "update")
			{
				aDlg->SetTimer(2, 2000,NULL);
			}
			if (strMsg == "g")
			{
				aDlg->OnBnClickedButtonKeyOnScreen2();
			}
			if (strMsg == "repair"|| strMsg == "批修理")
			{
				aDlg->OnBnClickedButtonMover2();
			}
			if (strMsg == "money" || strMsg == "批存")
			{
				aDlg->SetTimer(4, 2000, NULL);
				
			}
			if (strMsg == "重置检测次数"|| strMsg == "reset")
			{
				aDlg->OnBnClickedButtonGetmousepos();
			}
			
			if (strMsg == "进入"|| strMsg == "enter")
			{
				aDlg->SetTimer(3, 2000, NULL); 
			} 
			if (strMsg == "回家" || strMsg == "home")
			{
				aDlg->SetTimer(5, 2000, NULL);
			}
			if (strMsg == "关闭"|| strMsg == "close")
			{
				aDlg->SetTimer(7, 2000, NULL);
				
			}

			//关闭
			serverSocket.Close();
		}
	}

	//关闭
	aSocket.Close();
	serverSocket.Close();

	return 0;
}

 
//int my_hook_KeyPress(HANDLE msdk_handle, Keybfoard_KongGe, 1);
int my_hook_KeyPress(HANDLE m_hdl, int HidKeyCode, int Nbr)
{
	if (isDNFWindow())
	{
		if (Nbr == 1)
		{
			return M_KeyPress(m_hdl, HidKeyCode, Nbr);
		}
		else if(Nbr == 2)
		{
			M_KeyDown(m_hdl, HidKeyCode);
			M_DelayRandom(400, 510);
			return M_KeyUp(m_hdl, HidKeyCode);
		}
		else if (Nbr == 3)
		{
			//M_KeyPress(m_hdl, HidKeyCode, Nbr);

			M_KeyDown(m_hdl, HidKeyCode);
			M_DelayRandom(520, 600);
			return M_KeyUp(m_hdl, HidKeyCode);
		}
		else if (Nbr == 4)
		{
			//M_KeyPress(m_hdl, HidKeyCode, Nbr);

			M_KeyDown(m_hdl, HidKeyCode);
			M_DelayRandom(400, 500);
		 M_KeyUp(m_hdl, HidKeyCode);
			return M_KeyPress(m_hdl, HidKeyCode, Nbr);
		}
	}
	else
	{
		addLog("当前窗口非dnf");
		
	}
	return 0;
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
	if(bslowmode==false)
	{
		if(isDNFWindow())

		return M_RightClick(m_hdl, times);
	} 
	else 
	{
		M_RightDown(m_hdl);
		M_DelayRandom(500, 600);
		M_RightUp(m_hdl);
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
	 
	if (isDNFWindow()&&times<=3)
	{
		if (bslowmode)
		{
			for (int i = 0; i < times; i++)
			{
				M_LeftDown(m_hdl);
				M_DelayRandom(450, 550);
				M_LeftUp(m_hdl);
				M_DelayRandom(300, 340);
			}
			return 0;
			// M_LeftClick(m_hdl, times);
		}
		else
		{
			return M_LeftClick(m_hdl, 1);
		}
		
	}
	if (times == 1001)
	{
		M_LeftDown(m_hdl);
		M_DelayRandom(450, 500);
		M_LeftUp(m_hdl);
		return 0;
	}
	if (times == 4)
	{
		M_LeftDown(m_hdl);
		M_DelayRandom(450, 500);
		M_LeftUp(m_hdl);
		return M_LeftClick(m_hdl, 2);
	}
	if (times == 5)
	{
		M_LeftDown(m_hdl);
		M_DelayRandom(450, 500);
		M_LeftUp(m_hdl);
		M_DelayRandom(550, 700);

		M_LeftDown(m_hdl);
		M_DelayRandom(450, 500);
		M_LeftUp(m_hdl);
		return 0;
	}
	if ( times == 10)
	{
		M_LeftDown(m_hdl);
		M_DelayRandom(1500, 2100);
		M_LeftUp(m_hdl); 
	}
	 
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
bool findImageInWholeWindow_and_click(string strPath_findImage, int left, int top, int right, int bottom, HANDLE msdk_handle, int times, int findWay = 0, int x_add = 0, int y_add = 0)
{
	int RetSw = 0;
	CPoint pt = findImageWholeWindow_in_subrect(strPath_findImage, left, top, right, bottom);
	 


	CString infor;
	infor.Format("查找 %s 按钮%d,%d", strPath_findImage.c_str(), pt.x, pt.y);
	addLog(infor);
	if (pt.x == 0|| pt.y == 0)
	{

		return false;
	}
	else
	{

		pt.x += 18;
		pt.y += 12;
		if (x_add != 0 || y_add != 0)
		{
			pt.x += x_add;
			pt.y += y_add;
		}

		//确认分析装备
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 800);
			//这里使用的是绝对坐标
			//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
			//这里也可以写个定值
			RetSw = move_to_Pos(msdk_handle, pt.x , pt.y );
			RetSw = M_DelayRandom(500, 600);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, times);
		RetSw = M_DelayRandom(500, 600);
		RetSw = M_DelayRandom(500, 600);
		return true;
	}

}
bool findImage_and_click(string strPath_findImage, int left, int top, int right, int bottom,HANDLE msdk_handle,int times,int findWay=0,int x_add=0,int y_add=0)
{
	int RetSw = 0;
	CPoint pt = findImage_in_subrect(strPath_findImage, left, top, right, bottom);
	if(findWay==1)
	       pt = findImage(strPath_findImage, left, top, right, bottom);//


	CString infor;
	infor.Format("查找 %s 按钮%d,%d", strPath_findImage.c_str(), pt.x, pt.y);
	addLog(infor);
	if (pt.x == 0)
	{
		 
		return false;
	}
	else
	{
		
			pt.x += 18;
			pt.y += 12;
		if (x_add != 0 || y_add != 0)
		{
			pt.x += x_add;
			pt.y += y_add;
		}
	
		//确认分析装备
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			RetSw = M_DelayRandom(500, 800);
			//这里使用的是绝对坐标
			//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
			//这里也可以写个定值
			RetSw = move_to_relativePos(msdk_handle, pt.x - dleft, pt.y - dtop);
			RetSw = M_DelayRandom(500, 600);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, times);
		RetSw = M_DelayRandom(500, 600);
		RetSw = M_DelayRandom(500, 600);
		return true;
	}

}
//找指定图位置，letf,top,right,buttom在指定范围 = > 发现在在游戏中 0 (1463, 54)
CPoint findImageWholeWindow_in_subrect(string strPath_findImage, int left, int top, int right, int bottom)
{
	CPoint pt(0, 0);
	try {
		pDlg->saveScreen();



		//img = cv::imread(  "s.bmp", IMREAD_COLOR);
		img = cv::imread(strAppPath.GetBuffer(0) + (string)"s.bmp", IMREAD_COLOR);
		//这里可以取left,top,right,bottom中的子图
		Rect rect(left, dtop + top, right - left, bottom - top);
		Mat image_ori = img(rect);
		if (pDlg->bOnlyForTest)
		{
			imshow("ori", image_ori);
			cv::waitKey(-1);
		}
		string finalPath = strAppPath.GetBuffer(0) + strPath_findImage;
		//templ = cv::imread( strPath_findImage, IMREAD_COLOR);
		templ = cv::imread(finalPath, IMREAD_COLOR);
		//! [copy_source]
		/// Source image to display
		Mat img_display;
		image_ori.copyTo(img_display);
		//! [copy_source]

		//! [create_result_matrix]
		/// Create the result matrix
		int result_cols = img.cols - templ.cols + 1;
		int result_rows = img.rows - templ.rows + 1;

		result.create(result_rows, result_cols, CV_32FC1);
		//! [create_result_matrix]

		//! [match_template]
		/// Do the Matching and Normalize

		matchTemplate(image_ori, templ, result, match_method);

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
		long changeX = matchLoc.x ;
		long changeY = matchLoc.y ;
		img.release();
		image_ori.release();
		templ.release();
		/* long changeX = dleft - 1120;
		long changeY = dtop - 0; */
		//390,54 1030 54
		infor.Format("X=%ld,Y=%ld,x=%ld,y=%ld,maxVal=%0.2lf,", changeX, changeY, matchLoc.x, matchLoc.y, maxVal);
		addLog("find  " + infor);
		//if ((matchLoc.x - changeX) > 1255 && (matchLoc.x - changeX) < 1615 && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <=430 && maxVal > 0.5 )
		if (maxVal > 0.5)
		{

			pt.x = matchLoc.x  ;
			pt.y = matchLoc.y  ;
			return pt;
		}
		else
		{
			/*infor = infor+"未检测到 " ;
			addLog(infor);*/
			bResult = -1; Game_state = -1;
			pt.x = 0;
			pt.y = 0;
			return pt;
		}
		img.release();
		templ.release();
		image_ori.release();
		return pt;
	}
	catch (Exception & e)
	{
		CString strMfc = strPath_findImage.c_str();
		addLog("error findImage_in_subrect " + strMfc);
	}
	pt.x = 0;
	pt.y = 0;
	return pt;

}
//找指定图位置，letf,top,right,buttom在指定范围 = > 发现在在游戏中 0 (1463, 54)
CPoint findImage_in_subrect(string strPath_findImage, int left, int top, int right, int bottom)
{
	CPoint pt(0, 0);
	try {
		pDlg->saveScreen();



		//img = cv::imread(  "s.bmp", IMREAD_COLOR);
		img = cv::imread(strAppPath.GetBuffer(0) + (string)"s.bmp", IMREAD_COLOR);
		//这里可以取left,top,right,bottom中的子图
		Rect rect(dleft+left, dtop+top,  right-left,  bottom-top);
		Mat image_ori = img(rect);
		if (pDlg->bOnlyForTest )
		{
			imshow("ori", image_ori);
			cv::waitKey(-1);
		}
		string finalPath = strAppPath.GetBuffer(0) + strPath_findImage;
		//templ = cv::imread( strPath_findImage, IMREAD_COLOR);
		templ = cv::imread(finalPath, IMREAD_COLOR);
		//! [copy_source]
		/// Source image to display
		Mat img_display;
		image_ori.copyTo(img_display);
		//! [copy_source]

		//! [create_result_matrix]
		/// Create the result matrix
		int result_cols = img.cols - templ.cols + 1;
		int result_rows = img.rows - templ.rows + 1;

		result.create(result_rows, result_cols, CV_32FC1);
		//! [create_result_matrix]

		//! [match_template]
		/// Do the Matching and Normalize

		matchTemplate(image_ori, templ, result, match_method);

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
		image_ori.release();
		templ.release();
		/* long changeX = dleft - 1120;
		long changeY = dtop - 0; */
		//390,54 1030 54
		infor.Format("X=%ld,Y=%ld,x=%ld,y=%ld,maxVal=%0.2lf,", changeX, changeY, matchLoc.x, matchLoc.y, maxVal);
		addLog("game " + infor);
		//if ((matchLoc.x - changeX) > 1255 && (matchLoc.x - changeX) < 1615 && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <=430 && maxVal > 0.5 )
		if ( maxVal > 0.5)
		{

			pt.x = matchLoc.x+left+dleft;
			pt.y = matchLoc.y+top+dtop;
			return pt;
		}
		else
		{
			/*infor = infor+"未检测到 " ;
			addLog(infor);*/
			bResult = -1; Game_state = -1;
			pt.x = 0;
			pt.y = 0;
			return pt;
		}
		img.release();
		templ.release();
		image_ori.release();
		return pt;
	}
	catch (Exception & e)
	{
		CString strMfc = strPath_findImage.c_str();
		addLog("error findImage_in_subrect "+ strMfc);
	}
	pt.x = 0;
	pt.y = 0;
	return pt;

}
//找指定图位置，letf,top,right,buttom在指定范围 = > 发现在在游戏中 0 (1463, 54)
CPoint findImage(string strPath_findImage, int left, int top, int right, int bottom)
{
	CPoint pt(0, 0);
	try {
		pDlg->saveScreen();
		
		 

		//img = cv::imread(  "s.bmp", IMREAD_COLOR);
		img = cv::imread(strAppPath.GetBuffer(0) + (string)"s.bmp", IMREAD_COLOR);
		
		if (pDlg->bOnlyForTest)
		{//这里可以取left,top,right,bottom中的子图
		    Rect rect(dleft + left, dtop + top, right - left, bottom - top);
		
			Mat image_ori = img(rect);
			imshow("ori", image_ori);
			cv::waitKey(-1);
		}
		string finalPath = strAppPath.GetBuffer(0) + strPath_findImage;
		//templ = cv::imread( strPath_findImage, IMREAD_COLOR);
		templ = cv::imread(finalPath, IMREAD_COLOR);
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
		//image_ori.release();
		/* long changeX = dleft - 1120;
		long changeY = dtop - 0; */
		//390,54 1030 54
		//infor.Format("X=%ld,Y=%ld,x=%ld,y=%ld,maxVal=%0.2lf,", changeX, changeY, matchLoc.x, matchLoc.y, maxVal);
		//addLog("game " + infor);
		//if ((matchLoc.x - changeX) > 1255 && (matchLoc.x - changeX) < 1615 && (matchLoc.y - changeY) >= 275 && (matchLoc.y - changeY) <=430 && maxVal > 0.5 )
		if (changeX > left && changeX < right && changeY >= top && changeY <= bottom && maxVal > 0.5)
		{

			pt.x = matchLoc.x;
			pt.y = matchLoc.y;
			return pt;
		}
		else
		{
			/*infor = infor+"未检测到 " ;
			addLog(infor);*/
			bResult = -1; Game_state = -1;
			pt.x = 0;
			pt.y = 0;
			return pt;
		}
		img.release();
		templ.release();
		//image_ori.release();
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
		 

		img = cv::imread(strAppPath.GetBuffer(0) + (string)"s.bmp", IMREAD_COLOR);
		templ = cv::imread(strAppPath.GetBuffer(0) + (string)"confirm.png", IMREAD_COLOR);
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
	CString infor;
	//x = 1727, y = 70,can't tell you if game is over or other
	bool bResult = -1;
	try
	{
		CPoint cp = findImage("fightagain.png", 600, 50, 650, 90);
		if (cp.x != 0 && cp.y != 0)
		{
			addLog("游戏完成一次检测");
			
			M_DelayRandom(6600, 7400);
			CPoint cp = findImage("fightagain.png", 600, 50, 650, 90);

			if (cp.x != 0 && cp.y != 0)
			{
				addLog("游戏完成二次检测");


				img = cv::imread(strAppPath.GetBuffer(0) + (string)"s.bmp", IMREAD_COLOR);

				try
				{

					Mat NewImg = img(Rect(cp.x, cp.y, 80, 30));
					Mat means, stddev, covar;
				cv:Scalar tempVal = cv::mean(NewImg);
					float matMean = tempVal.val[0];
					CString strResult;
					//42 34 56//not change to grey
					// 43 29 45 //grey
					// 43 31 49
					//21:40 : 04   means : 43 31 50


					//这里使用了浮点数表示大小比较时要注意
					strResult.Format("means  : %0.1f %0.1f %0.1f\n", tempVal.val[0], tempVal.val[1], tempVal.val[2]);//RGB三通道，所以均值结果是3行一列
					addLog(strResult);
					//imshow("test", NewImg);
					if (tempVal.val[1] <= 32.9 && tempVal.val[2] <= 51 || pDlg->bOnlyForTest|| one_charactor_fighttime>= MAX_ONE_CHARACTOR_PLAYTIME)
						//if(TRUE)
					{

						CString str;
						CTime t = CTime::GetCurrentTime();
						CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

						str.Format("%s==>%s\r\n", tt, "帐号已经使用完成 0");
						CStdioFile file;
						if (file.Open(_T("log.txt"), CFile::typeText | CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate, NULL))
						{
							file.SeekToEnd();
							file.WriteString(str);
							file.Close();
						}
						addLog_important(str);
						addLog("帐号已经使用完成 0");
						Game_state = 100;
						bResult = 100;// 
						one_charactor_fighttime = 0;
					}
					else
					{//正常=1726,y=70,,
						 //正常=1726,y=70,,(42,34,56)
						//异常=1733,y=75    (37,49,55), 43,32,50
						//这里还是要取色一下
						CString strVal;
						strVal.Format("(%0.0lf,%0.0lf,%0.0lf)", tempVal.val[0], tempVal.val[1], tempVal.val[2]);

						if (tempVal.val[0] > 40)
						{
							infor = "检测到游戏还可以再玩 1" + infor + strVal;
							addLog(infor);
							bResult = 200;//检测到游戏还可以再玩
							Game_state = 200;

							one_charactor_fighttime++;
						}
						else
						{
							infor += "未检测到窗口 -1 ";
							addLog(infor);
							//pDlg->MessageBoxA("未检测到窗口,", "error", MB_OK);
							bResult = -1; Game_state = -1;
						}

						//	pDlg->MessageBoxA("检测到游戏还可以再玩,", "error", MB_OK);
					}
				}
				catch (exception & e)
				{
					img.release();
					templ.release();
					infor += "error execptio -1 ";
					addLog(infor);
					return -1;
				}

				img.release();
				return bResult;
			}
			else {
				 	addLog("未检测到游戏结束标志");
			}
		}
	}
	catch (Exception& e)
	{
		addLog("error checkGame_state");
		return -1;
	}
}
int checkGame_state_old()
{
	try
	{
		img = cv::imread(strAppPath.GetBuffer(0) + (string)"s.bmp", IMREAD_COLOR);
		templ = cv::imread(strAppPath.GetBuffer(0) + (string)"fightagain.png", IMREAD_COLOR);
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
		infor.Format("checkGame_state x=%ld,y=%ld,maxVal=%0.2lf,changeX=%ld,changeY=%ld", matchLoc.x, matchLoc.y, maxVal, changeX, changeY);

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
				// 43 31 49
				//21:40 : 04   means : 43 31 50



				strResult.Format("means  : %0.0f %0.0f %0.0f\n", tempVal.val[0], tempVal.val[1], tempVal.val[2]);//RGB三通道，所以均值结果是3行一列
				addLog(strResult);
				//imshow("test", NewImg);
				if (tempVal.val[1] <= 32 && tempVal.val[2] <= 51 || pDlg->bOnlyForTest)
					//if(TRUE)
				{

					CString str;
					CTime t = CTime::GetCurrentTime();
					CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

					str.Format("%s==>%s\r\n", tt, "帐号已经使用完成 0");
					CStdioFile file;
					if (file.Open(_T("log.txt"), CFile::typeText | CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate, NULL))
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
				{//正常=1726,y=70,,
					 //正常=1726,y=70,,(42,34,56)
					//异常=1733,y=75    (37,49,55)
					//这里还是要取色一下
					CString strVal;
					strVal.Format("(%0.0lf,%0.0lf,%0.0lf)", tempVal.val[0], tempVal.val[1], tempVal.val[2]);

					if (tempVal.val[0] > 40)
					{
						infor = "检测到游戏还可以再玩 1" + infor + strVal;
						addLog(infor);
						bResult = 200;//检测到游戏还可以再玩
						Game_state = 200;
					}
					else
					{
						infor += "未检测到窗口 -1 ";
						addLog(infor);
						//pDlg->MessageBoxA("未检测到窗口,", "error", MB_OK);
						bResult = -1; Game_state = -1;
					}

					//	pDlg->MessageBoxA("检测到游戏还可以再玩,", "error", MB_OK);
				}
			}
			catch (exception & e)
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
	catch (Exception & e)
	{
		addLog("error checkGame_state");
		return -1;
	}
}

DWORD WINAPI    duanzao_space(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;


	move_to_relativePos(msdk_handle, 50, 50);
	M_DelayRandom(800, 1000);

	M_LeftClick(msdk_handle, 1);

	while (true)
	{
	M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
	M_DelayRandom(300, 400);
	if (bStop)
		break;
	my_hook_left_Click(msdk_handle, 1);
	M_DelayRandom(300, 400);
	if (bStop)
		break;
	my_hook_left_Click(msdk_handle, 1);
	M_DelayRandom(300, 400);
	if (bStop)
		break;
	my_hook_left_Click(msdk_handle, 1);
	M_DelayRandom(300, 400);
	if (bStop)
		break;
 

	}
	addLog("exit duanzao");
	return 0;
}
DWORD WINAPI    continue_move_right(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp; 
	while (bStop==false)
	{
		int RetSw = M_DelayRandom(800, 1000);
		addLog("moveright");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 2);
		M_KeyDown(msdk_handle, Keyboard_XieGang_WenHao);
		M_DelayRandom(1000, 1290);
	     M_KeyUp(msdk_handle, Keyboard_XieGang_WenHao);

		 //check if finished
		 checkGame_state();
		 CString strInfor;
		 strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
		 addLog(strInfor);

		 if (Game_state == 100 || Game_state == 200)
		 {
			 CString strInfor;
			 strInfor.Format("bFind = %d Keyboard_PageDown\r\n", Game_state);
			 addLog(strInfor);
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
			 if (bStop)break;
			 RetSw = M_DelayRandom(200, 500);
			 RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			 RetSw = M_DelayRandom(200, 500);
			 RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			 RetSw = M_DelayRandom(200, 500);
			 RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			 RetSw = M_DelayRandom(200, 500);
			 RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			 if (bStop)break;
			 RetSw = M_DelayRandom(400, 600);
			 RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			 RetSw = M_DelayRandom(200, 500);
			 RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			 RetSw = M_DelayRandom(400, 600);
			 RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			 RetSw = M_DelayRandom(200, 500);
			 RetSw = M_DelayRandom(200, 500);
		 }

	}
	return 0;
}
DWORD WINAPI    fenjie_zhuangbei(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
 
	 

	int RetSw = M_DelayRandom(2800, 3000);
	CPoint pt = findImage("close.png", 370, 440, 390, 560);
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
	RetSw = M_DelayRandom(1000, 1100);
 //   pt = findImage_in_subrect("safe_mode.png", 200, 150, 600, 500);
	//if (pt.x != 0 && pt.y != 0)
	//{

	//	CString str = "";
	//	CTime t = CTime::GetCurrentTime();
	//	CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
	//	str.Format("%s==>%s (%ld,%ld)\n", tt, "发现safe_mode ", pt.x, pt.y);

	//	//str.Format("%s==>%s (%ld,%ld)\n", tt, "发现在在游戏中 0",cp.x,cp.y);
	//	addLog(str);
	//	bStop = true;
	//	bFullStop = true;
	//	addLog("stop");
	//	return 0;

	//}
	//else
	//{

	//	CString str = "";
	//	CTime t = CTime::GetCurrentTime();
	//	CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

	//	str.Format("%s==>%s (%ld,%ld)\n", tt, "未现safe_mode 0", pt.x, pt.y);
	//	addLog(str);
	//}


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
		RetSw = my_hook_left_Click(msdk_handle, 1001);

		infor.Format("绝对坐标 MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
		addLog(infor);

		RetSw = M_DelayRandom(800, 1000);
		addLog("点击游戏窗口");
		RetSw = move_to_relativePos(msdk_handle, 50, 50);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftClick(msdk_handle, 2);
		RetSw = M_DelayRandom(2800, 3000);

	}


	RetSw = M_DelayRandom(1000, 1100);
	findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1, 1);
	 

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
 
		addLog("按下Keyboard_DanYinHao键");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_DanYinHao, 2);
		//RetSw = M_KeyPress(msdk_handle, Keyboard_DanYinHao, 1);
		 
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
		RetSw = my_hook_left_Click(msdk_handle, 1001);
		RetSw = M_DelayRandom(800, 1000);
	 
	
	

	//全部分解装备
		findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1, 1);

		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_DelayRandom(800, 1000);

		findImage_and_click("select_all.png", 200, 260, 280, 360, msdk_handle, 1, 1);

		 
	//for (int i = 0; i < 1; i++)
	//{
	//	RetSw = M_ResetMousePos(msdk_handle);
	//	RetSw = M_DelayRandom(500, 600);
	//	//RetSw = my_new_MoveTo(msdk_handle, (int)((1371) / rate), (int)((351) / rate));
	//	RetSw = move_to_relativePos(msdk_handle, 240, 350);
	//	RetSw = M_DelayRandom(1500, 2600);
	//}
	//RetSw = my_hook_left_Click(msdk_handle, 1001);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	 
	findImage_and_click("fenjie_button.png", 285, 400, 380, 440, msdk_handle, 1, 1);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_DelayRandom(1800, 2000); 
	RetSw = M_DelayRandom(1800, 2000);

	RetSw = M_DelayRandom(1800, 2000);
	RetSw = M_DelayRandom(1800, 2000);

	addLog("按下Keyboard_ESCAPE键");
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
	RetSw = M_DelayRandom(2200, 3000);


	return 0;
}
CVC_DemoDlg::CVC_DemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVC_DemoDlg::IDD, pParent)
	, m_intMinute(1200)
	, m_edit_keyword(_T("勇士")
	)
	, m_checkTimes(124)
	, m_screenWidth(1920)
	, bHuangLong(FALSE)
	, bOnlyForTest(FALSE)
	, m_matchinename(_T(""))
	, bOnlyDNF(FALSE)
	, m_bSiwanTa(FALSE)
	, b_slowMode(FALSE)
	, m_current_Version(_T(strVserion))
{
	m_rate = 2.5;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	CString strPath;
	GetCurrentDirectory(256, strPath.GetBuffer(256));
	strAppPath.Format("%s\\", strPath);
	 
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
	DDX_Check(pDX, IDC_CHECK4, m_bSiwanTa);
	DDX_Check(pDX, IDC_CHECK5, b_slowMode);
	DDX_Text(pDX, IDC_EDIT9, m_current_Version);
}

BEGIN_MESSAGE_MAP(CVC_DemoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CVC_DemoDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CVC_DemoDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CVC_DemoDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS, &CVC_DemoDlg::OnBnClickedButtonKeypress)
	ON_BN_CLICKED(IDC_BUTTON_MOVER, &CVC_DemoDlg::OnBnClickedButtonMover)
	ON_BN_CLICKED(IDC_BUTTON_MOVETO, &CVC_DemoDlg::OnBnClickedButtonMoveto)
	ON_BN_CLICKED(IDC_BUTTON_GETMOUSEPOS, &CVC_DemoDlg::OnBnClickedButtonGetmousepos)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS3, &CVC_DemoDlg::OnBnClickedButtonKeypress3)
	ON_BN_CLICKED(IDC_BUTTON_OPEN2, &CVC_DemoDlg::OnBnClickedButtonOpen2)
	ON_BN_CLICKED(IDC_BUTTON_KEYPRESS5, &CVC_DemoDlg::OnBnClickedButtonKeyChangeUser)
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
	ON_BN_CLICKED(IDC_BUTTON_OPEN4, &CVC_DemoDlg::OnBnClickedButtonOpen4)
	ON_MESSAGE(WM_HOTKEY, OnHotKey)
	ON_LBN_SELCHANGE(IDC_LIST3, &CVC_DemoDlg::OnLbnSelchangeList3)
	ON_BN_CLICKED(IDC_BUTTON_MOVER3, &CVC_DemoDlg::OnBnClickedButtonMover3)
	ON_BN_CLICKED(IDC_BUTTON_MOVER4, &CVC_DemoDlg::OnBnClickedButtonMover4) 
	ON_BN_CLICKED(IDC_BUTTON_KEY_ON_SCREEN2, &CVC_DemoDlg::OnBnClickedButtonKeyOnScreen2)
	ON_BN_CLICKED(IDC_BUTTON_KEY_ON_SCREEN3, &CVC_DemoDlg::OnBnClickedButtonKeyOnScreen3)
	ON_BN_CLICKED(IDC_CHECK5, &CVC_DemoDlg::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_BUTTON_MOVER5, &CVC_DemoDlg::OnBnClickedButtonMover5)
	ON_BN_CLICKED(IDC_BUTTON_MOVER6, &CVC_DemoDlg::OnBnClickedButtonMover6)
	ON_BN_CLICKED(IDC_BUTTON_MOVER7, &CVC_DemoDlg::OnBnClickedButtonMover7)
	ON_BN_CLICKED(IDC_BUTTON_OPEN5, &CVC_DemoDlg::OnBnClickedButtonOpen5)
END_MESSAGE_MAP()


// CVC_DemoDlg 消息处理程序

BOOL CVC_DemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString strText = GetLocalIP();

	SetWindowText("notepad+++  " + strText);
	//this->SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
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
	 

	bool bok=RegisterHotKey(m_hWnd, 1000, 0, VK_F11);
	if (bok == false)
	{
		AfxMessageBox("hot key error");
	}
	CString infor;

	::GetPrivateProfileString(APP_NAME, "m_rate", "", infor.GetBufferSetLength(256), 256, "keypressDemo.ini");
	if (infor != "")
	{
		m_editRate.SetWindowTextA(infor);
		rate = atof(infor);
		m_rate = rate;
	}
	::GetPrivateProfileString(APP_NAME, "m_screenWidth", "", infor.GetBufferSetLength(256), 256, "keypressDemo.ini");
	if (infor != "")
	{
		SetDlgItemText(IDC_EDIT7, infor);
		m_screenWidth = atol(infor);

	}
	::GetPrivateProfileString(APP_NAME, "bHuangLong", "", infor.GetBufferSetLength(256), 256, "keypressDemo.ini");
	if (infor != "")
	{
		//SetDlgItemText(IDC_EDIT7, infor);
		bHuangLong = atol(infor);
		if (bHuangLong == 1)
			((CButton*)(GetDlgItem(IDC_CHECK1)))->SetCheck(1);

	}
	if (bGonlyDnf == true)
		((CButton*)(GetDlgItem(IDC_CHECK3)))->SetCheck(1);

	::GetPrivateProfileString(APP_NAME, "b_SiWangTa_Dnf", "", infor.GetBufferSetLength(256), 256, "keypressDemo.ini");
	if (infor != "")
	{
		//SetDlgItemText(IDC_EDIT7, infor);
		b_SiWangTa_Dnf = atol(infor);
		if (b_SiWangTa_Dnf == 1)
			((CButton*)(GetDlgItem(IDC_CHECK4)))->SetCheck(1);

	}
	::GetPrivateProfileString(APP_NAME, "bslowmode", "", infor.GetBufferSetLength(256), 256, "keypressDemo.ini");
	if (infor != "")
	{
		//SetDlgItemText(IDC_EDIT7, infor);
		bslowmode = atol(infor);
		if (bslowmode == 1)
			((CButton*)(GetDlgItem(IDC_CHECK5)))->SetCheck(1);

	}
	 
	::GetPrivateProfileString(APP_NAME, "m_matchname", "", infor.GetBufferSetLength(256), 256, "keypressDemo.ini");
	if (infor != "")
	{
		SetDlgItemText(IDC_EDIT8, infor);
		m_matchinename = infor;
		 

	}
	
	SetTimer(0, TIMER_LENGTH, NULL);
	::SetWindowPos((HWND)(this->m_hWnd), HWND_TOP, 0, 0, 800, 600, SWP_SHOWWINDOW | SWP_NOSIZE);
	HANDLE hThread = CreateThread(NULL, 0, StartServer, (LPVOID)this, 0, NULL);// TODO: 在此添加控件通知处理程序代码

	OnBnClickedButtonKeypress4();

	extract_png_files();
	

	

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

void CVC_DemoDlg::OnBnClickedButtonUpdate()
{
	
//	ShellExecute(this->m_hWnd, "open", "https://github.com/sunyuzhe114/KeyPressDemo/blob/master/x64/Release/VC_Demo.exe", NULL, NULL, SW_SHOWMAXIMIZED);
	//CString str = _T("ftp://192.168.1.166/D:/");
	//ShellExecute(this->m_hWnd, "open", "explorer",str, NULL, SW_SHOWMAXIMIZED);
	//ShellExecute(this->m_hWnd, "open", "http://192.168.1.166/D%3A/键盘控制/KeyPressDemo/x64/Release/VC_Demo.exe", NULL, NULL, SW_SHOWMINIMIZED);
	ShellExecute(this->m_hWnd, "open", "d://update.bat", NULL, NULL, SW_SHOWMINIMIZED);

	::PostQuitMessage(0);
}

DWORD WINAPI    changeUser(LPVOID pp)
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
		findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1, 1);
		if (bStop)break;

		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		addLog("按下Keyboard_ESCAPE键");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
		RetSw = M_DelayRandom(2800, 4000);
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle); 
			RetSw = move_to_relativePos(msdk_handle, 380, 460);
			RetSw = M_DelayRandom(500, 600);
		}
		CPoint cp = findImage("sysmenu.png", 320, 80, 480, 200);
		if (cp.x == 0)
		{
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
			RetSw = M_DelayRandom(1000, 1100);
		}

		RetSw = my_hook_left_Click(msdk_handle, 1001);
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
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		cp = findImage("sysmenu.png", 320, 80, 480, 200);
		if (cp.x != 0)
		{
			RetSw = my_hook_left_Click(msdk_handle, 1001);
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
			 
		}
		addLog("点击选定角色");
		if (bStop)break;

		if (bChangeUser == true)
		{
			addLog("按下方向右键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_RightArrow, 3);
			RetSw = M_DelayRandom(2100, 2300);

		}


		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 3);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 2);
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
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
	} while (0);
	
	addLog("changeUser exit");
	return 0;
}
DWORD WINAPI    changeUser_nawawa(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0; 
	RetSw = M_DelayRandom(1800, 2100);
	DWORD m_dTimeBeginPress_F10 = 0;
	move_to_relativePos(msdk_handle, 50, 50);
	RetSw = M_DelayRandom(800, 1000);
	RetSw = M_LeftClick(msdk_handle, 1);
	RetSw = M_DelayRandom(800, 1000);
	 
	for (int i = 0; i < 20; i++)
	{ 
		if (bStop)break;
 

		
		move_to_relativePos(msdk_handle, 320, 480);
		M_LeftDown(msdk_handle);
		M_DelayRandom(550, 600);
		M_LeftUp(msdk_handle);

		RetSw = M_DelayRandom(800, 1000);
		M_DelayRandom(550, 600);
		RetSw = M_LeftDoubleClick(msdk_handle,1);

		if (bStop)break;
		RetSw = my_hook_left_Click(msdk_handle, 2);
		if (bStop)break;

		RetSw = M_DelayRandom(1400, 2000);

		move_to_relativePos(msdk_handle, 420, 360);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		M_LeftDown(msdk_handle);
		M_DelayRandom(550, 600);
		M_LeftUp(msdk_handle);

		RetSw = my_hook_left_Click(msdk_handle, 2);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		
		 
		
	}
	addLog("nawawa  exit");

	addLog("playerlogin  begin");

	pDlg->playerlogin();
	//OnBnClickedButtonKeypress6();
	return 0;
}

DWORD WINAPI    maipiao(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;
	char* inputText = "600";
	char* buyNum_huanglong = "120";
	char* buyNum_qinglong = "85";

	//char* inputText = "1000";
	//char* buyNum_huanglong = "200";
	//char* buyNum_qinglong = "140";
	CString infor;

	do {


		move_to_relativePos(msdk_handle, 50, 50);
		RetSw = M_DelayRandom(800, 1000);

		RetSw = M_LeftClick(msdk_handle, 1);

		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = move_to_relativePos(msdk_handle, 250, 380);
			RetSw = M_DelayRandom(500, 600);
		}
		addLog("点击箱子");
		RetSw = my_hook_left_Click(msdk_handle, 1);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1, 1);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		//点击确认 这
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = move_to_relativePos(msdk_handle, 290, 130);
			RetSw = M_DelayRandom(500, 600);
		}
		addLog("点击共有");
		RetSw = my_hook_left_Click(msdk_handle, 1);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;

		//点击确认 这个位置要计算一下 

		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		RetSw = my_hook_left_Click(msdk_handle, 1);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		//这里要把无尽的永恒放在箱子里

		addLog("点击取出按钮");
		bool click_result = findImage_and_click("quchu.png", 240, 150, 450, 600, msdk_handle, 1, 1, 150, 4);
		if (click_result == true)
		{if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			click_result = findImage_and_click("wujin.png", 180, 135, 440, 435, msdk_handle, 1, 1, 5, 0);
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
//只有小屏幕才输入文字 
			if (dleft <= 600)
			{ 
				addLog("inputText 100");
				RetSw = M_KeyInputString(msdk_handle, inputText, strlen(inputText));
			}
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = my_hook_left_Click(msdk_handle, 1);
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;

		}

		RetSw = M_DelayRandom(1000, 1100);

		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		addLog("按下Keyboard_ESCAPE键");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);

		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;

		//开始移动到小铁柱,买票
		//按5号键，
		addLog("按5号键");
		if (bStop)break;
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 2);
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
		//这里检查一下,是否有5号键已经按下
		CPoint pt = findImage("xuanzeditu.png", 330, 300, 360, 350);
		if (pt.x != 0 && pt.y != 0)
		{
			addLog("faxian xuanzeditu");


		}
		else
		{
			addLog("find  xuanzeditu error 再按5号键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 2);
			RetSw = M_DelayRandom(2000, 2100);
			CPoint pt = findImage("xuanzeditu.png", 330, 300, 360, 350);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("faxian xuanzeditu");


			}
			else
			{
				break;
			}
		}
		//点击确认 这
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			if (pDlg->m_screenWidth <= 1366)
				RetSw = move_to_relativePos(msdk_handle, 405, 325);
			else
				RetSw = move_to_relativePos(msdk_handle, 395, 325);
			RetSw = M_DelayRandom(500, 600);
		}
		addLog("按确定");
		RetSw = my_hook_left_Click(msdk_handle, 2);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;

		pt = findImage("阿德大陆.png", 200, 20, 280, 50);
		if (pt.x != 0 && pt.y != 0)
		{
			addLog("阿德大陆");
		}
		else
		{
			addLog("find  阿德大陆 error");
			break;
		}
		if (bStop)break;
		//滚动3次到了 
		for (int i = 0; i < 4; i++)
		{
			RetSw = M_MouseWheel(msdk_handle, -1);
			if (bStop)break;
			RetSw = M_DelayRandom(1200, 1500);
			if (bStop)break;
			RetSw = M_DelayRandom(1200, 1500);
			if (bStop)break;
			RetSw = M_DelayRandom(1200, 1500);
			addLog("滚轮1次");
		}
		if (bStop)break;



		pt = findImage("素喃.png", 285, 20, 305, 40);
		if (pt.x != 0 && pt.y != 0)
		{
			addLog("素喃");
		}
		else
		{
			addLog("find  素喃 error");
			break;
		}

		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		//走到地点
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1510) / rate), (int)((277) / rate));
			RetSw = move_to_relativePos(msdk_handle, 390, 405);
			RetSw = M_DelayRandom(500, 600);
		}
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftDoubleClick(msdk_handle, 1);
		RetSw = my_hook_left_Click(msdk_handle, 4);


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
		addLog("选定坐标");
		if (bStop)break;

		pt = findImage("传送.png", 340, 310, 365, 330);
		if (pt.x != 0 && pt.y != 0)
		{
			addLog("传送");
		}
		else
		{
			addLog("find  传送 error");
			break;
		}
		//点击确认
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			/*RetSw = move_to_relativePos(msdk_handle, 370, 323);
			RetSw = M_DelayRandom(500, 600);*/
			if (pDlg->m_screenWidth <= 1366)
				RetSw = move_to_relativePos(msdk_handle, 405, 325);
			else
				RetSw = move_to_relativePos(msdk_handle, 395, 325);
		}
		RetSw = my_hook_left_Click(msdk_handle, 4);
		RetSw = M_DelayRandom(500, 1100); 

		addLog("点击确认");
		if (bStop)break;
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		//点击小铁柱
		click_result = findImage_and_click("xiaotiezhu.png", 0, 0, 800, 600,msdk_handle,1001);
		if (click_result == true)
		{
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			click_result=findImage_and_click("shangdian.png", 250, 0, 800,450, msdk_handle, 1001);

			RetSw = M_ResetMousePos(msdk_handle);
			if (pDlg->bHuangLong == true)
			{
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1710) / rate), (int)((405) / rate));
				RetSw = move_to_relativePos(msdk_handle, 354, 175);
				addLog("g购买黄龙");
			}
			else
			{
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
				RetSw = move_to_relativePos(msdk_handle, 177, 175);
				addLog("购买到青龙");
			}
			RetSw = M_DelayRandom(500, 600);

			M_KeyDown(msdk_handle, Keyboard_LeftShift);
			RetSw = M_DelayRandom(500, 600);
			my_hook_left_Click(msdk_handle, 1);
				RetSw = M_DelayRandom(500, 600);
			M_KeyUp(msdk_handle, Keyboard_LeftShift);
			
			if (dleft <= 600)
			{
				if (pDlg->bHuangLong == true)
				{
					RetSw = M_KeyInputString(msdk_handle, buyNum_huanglong, strlen(buyNum_huanglong));
				}
				else
				{
					RetSw = M_KeyInputString(msdk_handle, buyNum_qinglong, strlen(buyNum_qinglong));
				}
			}
			 
			RetSw = M_DelayRandom(1000, 1100);
			click_result = findImage_and_click("confirm.png", 0, 0, 800, 450, msdk_handle, 1001);
			RetSw = M_DelayRandom(1000, 1100);
			my_hook_left_Click(msdk_handle, 1);
			RetSw = M_DelayRandom(1000, 1100);
			my_hook_left_Click(msdk_handle,2);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);


		}


		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);

		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
	} while (0);

	addLog("maipiao exit");
	return 0;
}
DWORD WINAPI    cunqian(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;

	do {


		move_to_relativePos(msdk_handle, 50, 50);
		RetSw = M_DelayRandom(800, 1000);

		RetSw = M_LeftClick(msdk_handle, 1);
	 
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = move_to_relativePos(msdk_handle, 250, 380);
			RetSw = M_DelayRandom(500, 600);
		}
		addLog("点击箱子");
		for (int i = 0; i < 2; i++)
		{
			RetSw = my_hook_left_Click(msdk_handle, 1001);
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
		}
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
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1,1);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		//点击确认 这
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			RetSw = move_to_relativePos(msdk_handle, 290, 130);
			RetSw = M_DelayRandom(500, 600);
		}
		addLog("点击共有");
		RetSw = my_hook_left_Click(msdk_handle, 5);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;

		//点击确认 这个位置要计算一下
		 
		CPoint pt = findImage("savemoney.png", 150, 0, 750, 400);
		if (pt.x != 0 && pt.y != 0)
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
			str.Format("%s==>%s (%ld,%ld)\n", tt, "发现  savemoney", pt.x, pt.y);
			addLog(str);
		}
		else
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
			str.Format("%s==>%s (%ld,%ld)\n", tt, "未现savemoney", pt.x, pt.y);
			addLog(str);
		}

		CString infor;
		infor.Format("查找确定按钮%d,%d", pt.x, pt.y);
		addLog(infor);
		if (pt.x == 0)
		{
			bStop = true;
			addLog("查找 savemoney fail");
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
				RetSw = move_to_relativePos(msdk_handle, pt.x - dleft, pt.y - dtop);
				RetSw = M_DelayRandom(500, 600);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 1001);


		}
		
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		RetSw = my_hook_left_Click(msdk_handle, 1001);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		//这里要把挑战书存一下


		bool click_result = findImage_and_click("cailiao.png", 540, 200, 660, 300, msdk_handle, 1);
		if (click_result == true)
		{
			click_result = findImage_and_click("fangru.png", 300, 200, 480, 480, msdk_handle, 5);
		}
		if (click_result == true)
		{
			click_result = findImage_and_click("tiaozhanshu.png", 460, 200, 720, 480, msdk_handle,5);
		}

		RetSw = M_DelayRandom(1000, 1100);

		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break; 
		addLog("按下Keyboard_ESCAPE键");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);

		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
	} while (0);

	addLog("存钱 exit");
	return 0;
}
DWORD WINAPI    xiuli_fenjieji(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;

	do {

		 
		move_to_relativePos(msdk_handle, 50, 50);
		RetSw = M_DelayRandom(800, 1000);

		RetSw = M_LeftClick(msdk_handle, 1);
		/*RetSw = M_LeftDown(msdk_handle );
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftUp(msdk_handle );*/

		 
		//按5号键，
		 //按5号键，
		addLog("按5号键");
		if (bStop)break;
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 2);
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
		//这里检查一下,是否有5号键已经按下
		CPoint pt = findImage("xuanzeditu.png", 330, 300, 360, 350);
		if (pt.x != 0 && pt.y != 0)
		{
			addLog("faxian xuanzeditu");


		}
		else
		{
			addLog("find  xuanzeditu error 再按5号键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 2);
			RetSw = M_DelayRandom(2000, 2100);
			CPoint pt = findImage("xuanzeditu.png", 330, 300, 360, 350);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("faxian xuanzeditu");


			}
			else
			{
				break;
			}
		}
		//点击确认 这
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			if (pDlg->m_screenWidth <= 1366)
				RetSw = move_to_relativePos(msdk_handle, 405, 325);
			else
				RetSw = move_to_relativePos(msdk_handle, 395, 325);
			RetSw = M_DelayRandom(500, 600);
		}
		addLog("按确定");
		RetSw = my_hook_left_Click(msdk_handle, 2);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;

		pt = findImage("阿德大陆.png", 200, 20, 280, 50);
		if (pt.x != 0 && pt.y != 0)
		{
			addLog("阿德大陆");
		}
		else
		{
			addLog("find  阿德大陆 error");
			break;
		}
		 

		if (bStop)break;
		//滚动1次到了 
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_MouseWheel(msdk_handle, -1);
			if (bStop)break;
			RetSw = M_DelayRandom(1200, 1500);
			if (bStop)break;
			RetSw = M_DelayRandom(1200, 1500);
			if (bStop)break;
			RetSw = M_DelayRandom(1200, 1500);
		}
		if (bStop)break;

		addLog("滚轮1次");
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		//走到地点
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle); 
			RetSw = move_to_relativePos(msdk_handle, 608, 282);
			RetSw = M_DelayRandom(500, 600);
			int x_pos, y_pos;
			RetSw = M_GetCurrMousePos(msdk_handle, &x_pos, &y_pos);
			CString strpos;
			strpos.Format("鼠标坐标%ld,%ld", x_pos, y_pos);
			addLog(strpos);
		}
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		RetSw = my_hook_left_Click(msdk_handle, 1);

		RetSw = my_hook_left_Click(msdk_handle, 4);


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
		addLog("选定坐标");
		if (bStop)break;

		pt = findImage("传送.png", 340, 310, 365, 330);
		if (pt.x != 0 && pt.y != 0)
		{
			addLog("传送");
		}
		else
		{
			addLog("find  传送 error");
			break;
		}
		//点击确认
		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			/*RetSw = move_to_relativePos(msdk_handle, 370, 323);
			RetSw = M_DelayRandom(500, 600);*/
			if (pDlg->m_screenWidth <= 1366)
				RetSw = move_to_relativePos(msdk_handle, 405, 325);
			else
				RetSw = move_to_relativePos(msdk_handle, 395, 325);
		}
		RetSw = my_hook_left_Click(msdk_handle, 4);
		RetSw = M_DelayRandom(500, 1100);
	 

		addLog("点击确认");
		if (bStop)break;
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
		addLog("选定坐标");
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
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);

		 
		pt = findImage("yaluo.png", 150, 0, 750, 400);
		if (pt.x != 0 && pt.y != 0)
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
			str.Format("%s==>%s (%ld,%ld)\n", tt, "发现  yaluo", pt.x, pt.y);
			addLog(str);
		}
		else
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
			str.Format("%s==>%s (%ld,%ld)\n", tt, "未现yaluo", pt.x, pt.y);
			addLog(str);
		}

		CString infor;
		infor.Format("查找 yaluo %d,%d", pt.x, pt.y);
		addLog(infor);
		if (pt.x == 0)
		{
			 
			addLog("查找 yaluo fail");
		}
		else
		{
			pt.x += 18;
			pt.y += 20;
			//确认分析装备
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = M_DelayRandom(500, 800);
				//这里使用的是绝对坐标
				//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
				//这里也可以写个定值
				RetSw = move_to_relativePos(msdk_handle, pt.x - dleft, pt.y - dtop);
				RetSw = M_DelayRandom(500, 600);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 1);

			infor.Format("绝对坐标 MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
			addLog(infor);

			RetSw = M_DelayRandom(800, 1000);

			RetSw = M_DelayRandom(800, 1000);

			pt.x += 18;
			pt.y += 92;
		 
			//确认分析装备
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = M_DelayRandom(500, 800);
				//这里使用的是绝对坐标
				//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
				//这里也可以写个定值
				RetSw = move_to_relativePos(msdk_handle, pt.x - dleft, pt.y - dtop);
				RetSw = M_DelayRandom(500, 600);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 1);

			infor.Format("绝对坐标 MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
			addLog(infor);

			RetSw = M_DelayRandom(800, 1000);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;	
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break; 
			RetSw = my_hook_left_Click(msdk_handle, 1); 

			RetSw = M_DelayRandom(1000, 1100);
			 
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);

			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);





		}

		pt = findImage("yaluo.png", 150, 0, 750, 400);
		if (pt.x != 0 && pt.y != 0)
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
			str.Format("%s==>%s (%ld,%ld)\n", tt, "发现  yaluo", pt.x, pt.y);
			addLog(str);
		}
		else
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
			str.Format("%s==>%s (%ld,%ld)\n", tt, "未现yaluo", pt.x, pt.y);
			addLog(str);
		}
		 
		infor.Format("查找yaluo%d,%d", pt.x, pt.y);
		addLog(infor);
		if (pt.x == 0)
		{

			addLog("查找 yaluo fail");
		}
		else
		{
			pt.x += 18;
			pt.y += 20;
			//确认分析装备
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = M_DelayRandom(500, 800);
				//这里使用的是绝对坐标
				//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
				//这里也可以写个定值
				RetSw = move_to_relativePos(msdk_handle, pt.x - dleft, pt.y - dtop);
				RetSw = M_DelayRandom(500, 600);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 1);

			infor.Format("绝对坐标 MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
			addLog(infor);

			RetSw = M_DelayRandom(800, 1000);
			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);

			pt.x += 18;
			pt.y += 112;
			 
			//确认分析装备
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = M_DelayRandom(500, 800);
				//这里使用的是绝对坐标
				//RetSw = my_hook_MoveTo(msdk_handle, (int)(pt.x / rate), (int)(pt.y / rate));
				//这里也可以写个定值
				RetSw = move_to_relativePos(msdk_handle, pt.x - dleft, pt.y - dtop);
				RetSw = M_DelayRandom(500, 600);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 1);

			infor.Format("绝对坐标 MoveTo %d,%d", (int)(pt.x / rate), (int)(pt.y / rate));
			addLog(infor);

			RetSw = M_DelayRandom(800, 1000);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = my_hook_left_Click(msdk_handle, 1);

			RetSw = M_DelayRandom(1000, 1100);

			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);

			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);


			if (bStop)break;


		}
		if (bStop)break;
	} while (0);

	addLog("xiuli_fenjieji exit");
	return 0;
}
DWORD WINAPI    change_to_first_player(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	RetSw = M_DelayRandom(1800, 2100);
	DWORD m_dTimeBeginPress_F10 = 0;


	do {
		move_to_relativePos(msdk_handle, 50, 50);
		RetSw = M_DelayRandom(800, 1000);
		RetSw = M_LeftClick(msdk_handle, 1);
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1, 1);
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		addLog("按下Keyboard_ESCAPE键");
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
		RetSw = M_DelayRandom(2800, 4000);
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);

		CPoint cp = findImage("sysmenu.png", 320, 80, 480, 200);
		if (cp.x == 0)
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);

		for (int i = 0; i < 1; i++)
		{
			RetSw = M_ResetMousePos(msdk_handle);
			//RetSw = my_new_MoveTo(msdk_handle, (int)((1517) / rate), (int)((454) / rate));
			RetSw = move_to_relativePos(msdk_handle, 380, 460);
			RetSw = M_DelayRandom(500, 600);
		}
		RetSw = my_hook_left_Click(msdk_handle, 1);
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
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);
		addLog("点击选定角色");
		if (bStop)break;

	 //	for (int i = 0; i < 12; i++)
		//{
		//	addLog("按下方向left键");
		//	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_LeftArrow, 1);
		//	if (bStop)break;
		//	RetSw = M_DelayRandom(1000, 1100);

		//}
	 
		RetSw = M_KeyPress (msdk_handle, Keyboard_KongGe, 1);
		RetSw = M_DelayRandom(400, 600);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 1);
		RetSw = M_DelayRandom(1000, 1100);
		RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 4);
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
	} while (0);

	return 0;
}
DWORD WINAPI    changeUser_maipiao(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	RetSw = M_DelayRandom(1800, 2100);
	DWORD m_dTimeBeginPress_F10 = 0;


	change_to_first_player(pp);

	for (int i = 0; i < 16; i++)
	{
		maipiao(pp);
		changeUser(pp);
		if (bStop)break;
	}

	pDlg->GetDlgItem(IDC_BUTTON_MOVER7)->EnableWindow(true);
	addLog("changeUser_cunqian finished");
	return 0;
}
DWORD WINAPI    changeUser_cunqian(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp; 
	unsigned int RetSw = 0;
	RetSw = M_DelayRandom(1800, 2100);
	DWORD m_dTimeBeginPress_F10 = 0;


	change_to_first_player(pp);

	for (int i = 0; i < 16; i++)
	{
		cunqian(pp);
		changeUser(pp);
		if (bStop)break;
	}

	pDlg->GetDlgItem(IDC_BUTTON_MOVETO)->EnableWindow(true);
	addLog("changeUser_cunqian exit");
	return 0;
}
DWORD WINAPI    changeUser_xiuli_fenjieji(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	RetSw = M_DelayRandom(1800, 21000);
	DWORD m_dTimeBeginPress_F10 = 0; 
	move_to_relativePos(msdk_handle, 50, 50);
	RetSw = M_DelayRandom(800, 1000); 
	RetSw = M_LeftClick(msdk_handle, 1);


	//change_to_first_player(pp);

	for (int i = 0; i < 16; i++)
	{
		fenjie_zhuangbei(pp);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		xiuli_fenjieji(pp);
		if (bStop)break;
		RetSw = M_DelayRandom(800, 1000);
		if (bStop)break;
		changeUser(pp);
		if (bStop)break;
	}
	pDlg->GetDlgItem(IDC_BUTTON_MOVER2)->EnableWindow(true);
	addLog("changeUser_fenjie exit");
	return 0;
}
DWORD WINAPI    fighting_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;
	int workKind = 1;
	CString infor; 
		do {

		 
			move_to_relativePos(msdk_handle, 50, 50);
			RetSw = M_DelayRandom(800, 1000);
			RetSw = M_LeftClick(msdk_handle, 1);

			if (workKind == 1)
			{
				RetSw = M_DelayRandom(900, 1000);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_h, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(299, 300);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_z, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(490, 500);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_f, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(290, 300);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_y, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(490, 500);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_e, 2);
				if (bStop)break;
				RetSw = M_DelayRandom(290, 300);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_q, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(1400, 1500);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_f, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(490, 500);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_r, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(490, 500);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_t, 1);
				if (bStop)break;


			}
			


		} while (0);

		addLog("test fight ");
		return 0;
}

DWORD WINAPI    changeUser_And_Login_siwangTa_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;
	bool b_login_ok = false;
	int CHECK_LOGIN_TIME = 10;
	if (pDlg->bOnlyForTest == true)
	{
		CHECK_LOGIN_TIME = 2;
	}
	for (int test = 0; test < CHECK_LOGIN_TIME; test++)
	{
		b_login_ok = false;
		do {
			RetSw = M_ReleaseAllKey(msdk_handle);
			/*for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);;
				RetSw = M_DelayRandom(800, 1000);
			}*/
			move_to_relativePos(msdk_handle, 50, 50);
			RetSw = M_DelayRandom(800, 1000);
			RetSw = M_LeftClick(msdk_handle, 1);

			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);
			//RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
			RetSw = M_DelayRandom(2800, 4000);


			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = move_to_relativePos(msdk_handle, 380, 460);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 1);
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
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("点击选定角色");
			if (bStop)break;

			if (bChangeUser == true)
			{
				addLog("按下方向右键");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_RightArrow, 3);
				RetSw = M_DelayRandom(2100, 2300);
				//bChangeUser =false;//下次不切了?
			}
			if (bChangetofirstUser == true)
			{
				bChangetofirstUser = false;
				for (int i = 0; i < 16; i++)
				{
					addLog("按下方向left键");
					RetSw = my_hook_KeyPress(msdk_handle, Keyboard_LeftArrow, 1);
					if (bStop)break;
					RetSw = M_DelayRandom(1000, 1100);

				}
			}


			RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
			RetSw = M_DelayRandom(1000, 1100);
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
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);

			//按5号键，
			addLog("按5号键");
			if (bStop)break;
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 1);
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
			//这里检查一下,是否有5号键已经按下
			CPoint pt = findImage("xuanzeditu.png", 320, 300, 430, 350);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("faxian xuanzeditu");
			}
			else
			{
				addLog("find  xuanzeditu error");
				break;
			}
			//点击确认 这
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
				RetSw = move_to_relativePos(msdk_handle, 370, 325);
				RetSw = M_DelayRandom(500, 600);
			}
			addLog("按确定");
			RetSw = my_hook_left_Click(msdk_handle, 1);
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;

			pt = findImage("阿德大陆.png", 200, 20, 225, 40);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("阿德大陆");
			}
			else
			{
				addLog("find  阿德大陆 error");
				break;
			}
			if (bStop)break;
			//滚动3次到了 
			 
			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);
			//走到地点
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1510) / rate), (int)((277) / rate));
				RetSw = move_to_relativePos(msdk_handle, 400, 267);
				RetSw = M_DelayRandom(500, 600);
			}
			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);
			RetSw = my_hook_left_Click(msdk_handle, 1);


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
			addLog("选定坐标");
			if (bStop)break;

			pt = findImage("传送.png", 340, 310, 365, 330);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("传送");
			}
			else
			{
				addLog("find  传送 error");
				break;
			}
			//点击确认
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
				RetSw = move_to_relativePos(msdk_handle, 370, 323);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 2);
			RetSw = M_DelayRandom(500, 1100);
			//点击确认二次,保证点上
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
				RetSw = move_to_relativePos(msdk_handle, 370, 326);
				RetSw = M_DelayRandom(500, 600);
			}
			RetSw = my_hook_left_Click(msdk_handle, 2);

			addLog("点击确认");
			if (bStop)break;
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("点击屏幕人物开始移动");

			if (checkDNFWindow() == false)
			{
				addLog("未检测到窗口，停止。。。");
				break;
			}
			if (bStop)break;
			//走到地点
			  
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("键盘向左移动一步");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("向左移动一步");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("向左移动一步");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			RetSw = M_DelayRandom(1000, 1100);
			addLog("键盘向左移动一步");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("向左移动一步");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("向左移动一步");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
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
			 


			if (bStop)break;
			//点击 
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				
					//RetSw = my_new_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
					RetSw = move_to_relativePos(msdk_handle, 421, 140);
					addLog("绝望之");
				 
				RetSw = M_DelayRandom(500, 600);
			}
			b_login_ok = true;

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

			//pDlg->begin_check_game();
			if (bStop)break;
		} while (0);
		if (b_login_ok == true)break;
		if (bFullStop)break;
		CString strLog;
		strLog.Format("登录尝试%ld次", test);
		addLog(strLog);
	}


	if (b_login_ok == true)
	{

		pDlg->begin_check_game();
	}
	else
	{

		//如果登录失败则出现异常,则程序停止,这里要检测一下.

		CPoint pt = findImage("关闭按钮.png", 780, 0, 800, 30);
		if (pt.x != 0 && pt.y != 0)
		{

			addLog("关闭按钮");
			do {
				addLog("按下Keyboard_ESCAPE键");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				if (bStop)break;

			} while (0);



		}

		//如果登录失败则出现异常,则程序停止,这里要检测一下.
		if (bFullStop)
		{
			pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
			pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(true);
			pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(true);
		}
		else
		{
			CString strLog;
			strLog.Format("重新登录尝试 test");
			addLog(strLog);
			pDlg->OnBnClickedButtonKeyChangeUser();
		}


	}


	addLog("changeUser_And_Login_siwangTa_Thread 登录线程停止 exit");

	return 0;
}
DWORD WINAPI    changeUser_backtohome(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;
	bool b_login_ok = false;
	int CHECK_LOGIN_TIME = 1;
	addLog("开始回家");
		b_login_ok = false;
		do {
			RetSw = M_ReleaseAllKey(msdk_handle);
			/*for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);;
				RetSw = M_DelayRandom(800, 1000);
			}*/
			move_to_relativePos(msdk_handle, 50, 50);
			RetSw = M_DelayRandom(800, 1000);
			RetSw = M_LeftClick(msdk_handle, 1);
			RetSw = M_DelayRandom(800, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F12, 2);
			RetSw = M_DelayRandom(2800, 4000);
			addLog("按下F12键");
			
			RetSw = M_DelayRandom(800, 1000);
			//RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
			RetSw = M_DelayRandom(2800, 4000);
			if (bStop)break;

			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = move_to_relativePos(msdk_handle, 380, 460);
				RetSw = move_to_relativePos(msdk_handle, 500, 460);
				RetSw = M_DelayRandom(500, 600);
			}
			addLog("点击返回键");
			RetSw = my_hook_left_Click(msdk_handle, 1);

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
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);

			move_to_relativePos(msdk_handle, 50, 50);
			RetSw = M_DelayRandom(800, 1000);
			RetSw = M_LeftClick(msdk_handle, 1);
			RetSw = M_DelayRandom(800, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F12, 2);
			RetSw = M_DelayRandom(2800, 4000);

			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);
			//RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
			RetSw = M_DelayRandom(2800, 4000);

			CPoint cp = findImage("sysmenu.png", 320, 80, 480, 200);
			if(cp.x==0)
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = move_to_relativePos(msdk_handle, 380, 460);
				//RetSw = move_to_relativePos(msdk_handle, 380, 500);
				RetSw = M_DelayRandom(500, 600);
			}
			addLog("点击选择用户");

			RetSw = my_hook_left_Click(msdk_handle, 1);  
			RetSw = M_DelayRandom(2000, 2100);
			for (int i = 0; i < 16; i++)
			{
				addLog("按下方向left键");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_LeftArrow, 1);
				if (bStop)break;
				RetSw = M_DelayRandom(1000, 1100);

			}
			RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
			RetSw = M_DelayRandom(1000, 1100);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 1);
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
		} while (0);

			 
	return 0;
}
DWORD WINAPI    changeUser_And_Login_Thread(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_F10 = 0;

	CString infor;
	bool b_login_ok = false;
	int CHECK_LOGIN_TIME = 10;
	if (pDlg->bOnlyForTest == true)
	{
		CHECK_LOGIN_TIME = 2;
	}
	for (int test = 0; test < CHECK_LOGIN_TIME; test++)
	{
		b_login_ok = false;
		do {
			RetSw = M_ReleaseAllKey(msdk_handle);
			/*for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				RetSw = my_new_MoveTo(msdk_handle, 1200 / rate, 110 / rate);;
				RetSw = M_DelayRandom(800, 1000);
			}*/
			move_to_relativePos(msdk_handle, 50, 50);
			RetSw = M_DelayRandom(800, 1000); 
			RetSw = M_LeftClick(msdk_handle, 1);
			RetSw = M_DelayRandom(800, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F12, 2);
			RetSw = M_DelayRandom(2800, 4000);

			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1, 1);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);

			RetSw = M_DelayRandom(800, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 2);
			RetSw = M_DelayRandom(2800, 4000);

			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);
			//RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 1);
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
			RetSw = M_DelayRandom(2800, 4000);

			CPoint cp = findImage("sysmenu.png", 320, 80, 480, 200);
			if (cp.x == 0)
			{
				addLog("再次按下Keyboard_ESCAPE键");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
				RetSw = M_DelayRandom(2800, 4000);
			}

			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle); 
				RetSw = move_to_relativePos(msdk_handle, 380, 460);
				RetSw = M_DelayRandom(500, 600);
			}

			RetSw = my_hook_left_Click(msdk_handle, 1001); 
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
			 cp = findImage("sysmenu.png", 320, 80, 480, 200);
			if (cp.x != 0)
			{
				addLog("再次按下选择角色");
				my_hook_left_Click(msdk_handle, 1001);
			}
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
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("点击选定角色");
			if (bStop)break;

			if (bChangeUser == true)
			{
				addLog("按下方向右键");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_RightArrow,3);
				RetSw = M_DelayRandom(2100, 2300);
				//bChangeUser =false;//下次不切了?
			}
		    if (bChangetofirstUser == true)
			{
				bChangetofirstUser = false;
				for (int i = 0; i < 16; i++)
				{
					addLog("按下方向left键");
					RetSw = my_hook_KeyPress(msdk_handle, Keyboard_LeftArrow, 1);
					if (bStop)break;
					RetSw = M_DelayRandom(1000, 1100);

				}
			}


			RetSw = M_KeyPress(msdk_handle, Keyboard_KongGe, 1);
			RetSw = M_DelayRandom(1000, 1100);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 2);
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
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);

			//按5号键，
			addLog("按5号键");
			if (bStop)break;
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 2);
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
			//这里检查一下,是否有5号键已经按下
			CPoint pt = findImage("xuanzeditu.png", 330, 300, 360, 350);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("faxian xuanzeditu");


			}
			else
			{
				addLog("find  xuanzeditu error 再按5号键");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_5, 2);
				RetSw = M_DelayRandom(2000, 2100);
				CPoint pt = findImage("xuanzeditu.png", 330, 300, 360, 350);
				if (pt.x != 0 && pt.y != 0)
				{
					addLog("faxian xuanzeditu");


				}
				else
				{
					break;
				}
			}
			//点击确认 这
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
				if(pDlg->m_screenWidth<=1366)
				RetSw = move_to_relativePos(msdk_handle, 405, 325);
				else
					RetSw = move_to_relativePos(msdk_handle, 395, 325);
				RetSw = M_DelayRandom(500, 600);
			}
			addLog("按确定");
			RetSw = my_hook_left_Click(msdk_handle, 2);

			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			findImage_and_click("xuanzeditu.png", 330, 300, 360, 350,msdk_handle,1,0);
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;

			pt = findImage("阿德大陆.png", 200, 20, 280, 50);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("阿德大陆");
			}
			else
			{
				addLog("find  阿德大陆 error");
				break;
			}
			if (bStop)break;
			//滚动3次到了 
			for (int i = 0; i < 4; i++)
			{
				RetSw = M_MouseWheel(msdk_handle, -1);
				if (bStop)break;
				RetSw = M_DelayRandom(1200, 1500);
				if (bStop)break;
				RetSw = M_DelayRandom(1200, 1500);
				if (bStop)break;
				RetSw = M_DelayRandom(1200, 1500);
				addLog("滚轮1次");
			}
			if (bStop)break;

			

			pt = findImage("素喃.png", 285, 20, 305, 40);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("素喃");
			}
			else
			{
				addLog("find  素喃 error");
				break;
			}

			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);
			//走到地点
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1510) / rate), (int)((277) / rate));
				RetSw = move_to_relativePos(msdk_handle, 385, 272);
				RetSw = M_DelayRandom(500, 600);
			}
			if (bStop)break;
			RetSw = M_DelayRandom(800, 1000);
			RetSw = M_LeftDoubleClick(msdk_handle, 1);
			RetSw = my_hook_left_Click(msdk_handle, 4);


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
			addLog("选定坐标");
			if (bStop)break;

			pt = findImage("传送.png", 340, 310, 365, 330);
			if (pt.x != 0 && pt.y != 0)
			{
				addLog("传送");
			}
			else
			{
				addLog("find  传送 error");
				break;
			}
			//点击确认
			for (int i = 0; i < 1; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
				/*RetSw = move_to_relativePos(msdk_handle, 370, 323);
				RetSw = M_DelayRandom(500, 600);*/
				if (pDlg->m_screenWidth <= 1366)
					RetSw = move_to_relativePos(msdk_handle, 405, 325);
				else
					RetSw = move_to_relativePos(msdk_handle, 395, 325);
			}
			RetSw = my_hook_left_Click(msdk_handle, 4);
			RetSw = M_DelayRandom(500, 1100);
			//点击确认二次,保证点上
			//for (int i = 0; i < 1; i++)
			//{
			//	RetSw = M_ResetMousePos(msdk_handle);
			//	//RetSw = my_new_MoveTo(msdk_handle, (int)((1496) / rate), (int)((325) / rate));
			//	RetSw = move_to_relativePos(msdk_handle, 370, 326);
			//	RetSw = M_DelayRandom(500, 600);
			//}
			//RetSw = my_hook_left_Click(msdk_handle, 4);

			addLog("点击确认");
			if (bStop)break;
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			addLog("点击屏幕人物开始移动");

			if (checkDNFWindow() == false)
			{
				addLog("未检测到窗口，停止。。。");
				break;
			}
			if (bStop)break;
			//走到地点
			for (int j = 0; j < 2; j++)
			{
				for (int i = 0; i < 1; i++)
				{
					RetSw = M_ResetMousePos(msdk_handle);
					//RetSw = my_new_MoveTo(msdk_handle, (int)((1753) / rate), (int)((503) / rate));
					RetSw = move_to_relativePos(msdk_handle, 700, 485);
					RetSw = M_DelayRandom(500, 600);
				}
				RetSw = my_hook_right_Click(msdk_handle, 2);
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
				RetSw = my_hook_right_Click(msdk_handle, 2);
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

			 
			pt = findImage("选择地下城.png", 200, 560, 450, 600);
			if (pt.x != 0 && pt.y != 0)
			{
				b_login_ok = true;
				addLog("选择地下城");
			}
			
			if (b_login_ok == false)
			{

				addLog("键盘向左移动一步");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
				if (bStop)break;
				RetSw = M_DelayRandom(1000, 1100);
				addLog("向左移动一步");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
				if (bStop)break;
				RetSw = M_DelayRandom(1000, 1100);
				addLog("向左移动一步");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_Douhao, 2);
				if (bStop)break;
				RetSw = M_DelayRandom(1000, 1100);
				addLog("向右移动二步");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 4);
			}
			
			pt = findImage("选择地下城.png",   200, 560, 450, 600);
			if (pt.x != 0 && pt.y != 0)
			{
				b_login_ok = true;
				addLog("选择地下城");
			}

			if (b_login_ok == false)
			{
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 4);
				if (bStop)break;
				RetSw = M_DelayRandom(1000, 1100);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_JuHao, 4);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_JuHao, 4);
				if (bStop)break;
			}
			pt = findImage("选择地下城.png", 200, 560, 450, 600);
			if (pt.x != 0 && pt.y != 0)
			{
				b_login_ok = true;
				addLog("选择地下城");
			}
			if (b_login_ok == false)
			{
				RetSw = M_DelayRandom(500, 1000);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 4);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 4);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 4);
				if (bStop)break;
			}
			pt = findImage("选择地下城.png", 200, 560, 450, 600);
			if (pt.x != 0 && pt.y != 0)
			{
				b_login_ok = true;
				addLog("选择地下城");
			}
			if (b_login_ok == false)
			{
			RetSw = M_DelayRandom(500, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 4);
			if (bStop)break;
			RetSw = M_DelayRandom(500, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao, 4);
			if (bStop)break;
			RetSw = M_DelayRandom(500, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_XieGang_WenHao,4);
			}

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


			if (b_login_ok == false)
			{
				pt = findImage("选择地下城.png",  200, 560, 450, 600);
				if (pt.x != 0 && pt.y != 0)
				{
					b_login_ok = true;
					addLog("选择地下城");
				}
				else
				{
					addLog("find  选择地下城 error");
					break;
				}
			}
			
			RetSw = my_hook_left_Click(msdk_handle, 4);
			RetSw = M_LeftDoubleClick(msdk_handle, 1);
			if (bStop)break;
			RetSw = M_DelayRandom(500, 1000);
			RetSw = M_LeftDoubleClick(msdk_handle, 1);
			if (bStop)break;
			RetSw = M_DelayRandom(500, 1000);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 4);
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

			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 4);
			addLog("按下空格准备游戏");

			//第三次点击，是反向的，相当用户一没票就选另外一家
			for (int i = 0; i < 2; i++)
			{
				RetSw = M_ResetMousePos(msdk_handle);
				if (pDlg->bHuangLong == false)
				{
					//RetSw = my_new_MoveTo(msdk_handle, (int)((1710) / rate), (int)((405) / rate));
					RetSw = move_to_relativePos(msdk_handle, 570, 390);
					addLog("移动到黄龙");

					RetSw = my_hook_left_Click(msdk_handle, 4);
					RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 4);
					if (bStop)break;
					RetSw = M_DelayRandom(500, 1000);
					if (bStop)break;
				}
				else
				{
					//RetSw = my_new_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
					RetSw = move_to_relativePos(msdk_handle, 210, 440);
					addLog("移动到青龙");
					RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 4);
					if (bStop)break;
					RetSw = M_DelayRandom(500, 1000);
					if (bStop)break;
					RetSw = my_hook_left_Click(msdk_handle, 4);
				}
				RetSw = M_DelayRandom(500, 600);
			}


			  

			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_KongGe, 4);
			addLog("按下空格准备游戏");



			//pDlg->begin_check_game();
			if (bStop)break;
		} while (0);
		if (b_login_ok == true)break;
		if (bFullStop)break;
		CString strLog;
		strLog.Format("登录尝试%ld次", test);
		addLog(strLog);
	}


	if (b_login_ok == true)
	{

		pDlg->begin_check_game();
	}
	else 
	{
	
		//如果登录失败则出现异常,则程序停止,这里要检测一下.

		CPoint pt = findImage("关闭按钮.png", 780, 0, 800, 30);
		if (pt.x != 0 && pt.y != 0)
		{
			 
			addLog("关闭按钮"); 
			do {
				addLog("按下Keyboard_ESCAPE键");
				RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				if (bStop)break;
				RetSw = M_DelayRandom(500, 1000);
				if (bStop)break;

			} while (0);



		}
		 
		//如果登录失败则出现异常,则程序停止,这里要检测一下.
		if (bFullStop)
		{
			pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
			pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(true);
			pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(true);
		}
		else
		{
			CString strLog;
			strLog.Format("重新登录尝试 test");
			addLog(strLog);
			pDlg->OnBnClickedButtonKeyChangeUser();
		}
		 	
			 
	}


	addLog("changeUser_And_Login_Thread 登录线程停止 exit");

	return 0;
}
DWORD WINAPI    testThread_Game(LPVOID pp)
{
	HANDLE msdk_handle = (HANDLE)pp;
	unsigned int RetSw = 0;
	DWORD m_dTimeBeginPress_BIGSKILL = 0;
	CString text;
	pDlg->m_editLogInfor.GetWindowTextA(text); 
	USES_CONVERSION;
	char* inputText = new char[256];
	bool bOutputChinese = true;
	if (text == "1")
	{
		strcpy(inputText, "红色小晶块");
	}
	else if (text == "2")
	{
		strcpy(inputText, "无尽的永恒");
	}
	else if (text == "3")
	{
		strcpy(inputText, "瞬间移动");
	}
	else if (text == "4")
	{
	 
		strcpy(inputText, "在线敲头");
	}
	else if (text == "0")
	{
		strcpy(inputText, "立即删除");
	}
	else
	{
		bOutputChinese = false;
		strcpy(inputText, text);
	}
	
	if (bOutputChinese = false)
	{
		RetSw = M_KeyInputString(msdk_handle, inputText, strlen(inputText));
	}
	else
	{

		RetSw = M_KeyInputStringGBK(msdk_handle, inputText, strlen(inputText));
	}
	 
 
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
		//RetSw = M_ReleaseAllKey(msdk_handle);
		//RetSw = M_ReleaseAllMouse(msdk_handle);
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		
		if (bStop)break;
		RetSw = M_DelayRandom(1000, 1100);

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
			if (bStop)break;
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			if (bStop)break;
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
		if (bStop)break;
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
			//findImage_and_click("wait_set.png", 420, 340, 520, 380, msdk_handle, 1, 1);
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
			if (bStop)break;
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
		if (bStop)break;
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
			if (bStop)break;
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(200, 500);
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_x, 1);
			RetSw = M_DelayRandom(400, 600);
			if (bStop)break;
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
		CPoint cp = findImage("ingamenew.png", 340, 40, 480, 80);
		if (cp.x != 0 && cp.y != 0)
		{ 

			/*CString str;

			str.Format("%s (%ld,%ld)\n", "发现在游戏中 0", cp.x, cp.y);
			addLog(str);*/
			not_in_game_time = 0;
		}
		else
		{

			CString str;
			str.Format(" %s (%ld,%ld)\n", "未在游戏中 一次", cp.x, cp.y);
			addLog(str);
			not_in_game_time++;
			if (not_in_game_time >= 10)
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
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F12, 2);
	RetSw = M_DelayRandom(600, 1000);
	RetSw = my_hook_KeyPress(msdk_handle, Keyboard_F12, 2);
	RetSw = M_DelayRandom(600, 1000);
	addLog("exit checkThread_Game \r\n");



	RetSw = M_ReleaseAllKey(msdk_handle);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(true);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(true);

	CString infor;
	infor.Format("stop continue remains %ld \r\n", pDlg->m_checkTimes - Global_checkTime);
	addLog(infor);
	RetSw = M_DelayRandom(10000, 15000);
	 
	for (int i = 0; i < 100; i++)
	{
		RetSw = M_DelayRandom(1000, 1500);
		if (bStop)
			break;
	}
	 
	
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

		pDlg->OnBnClickedButtonKeyChangeUser();
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
	CString strFileName = _T(strAppPath+"s.bmp");
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
		
		GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(false);
	}
}
void CVC_DemoDlg::OnBnClickedButtonKeypress()
{
	OnBnClickedButtonKeypress4();

	begin_check_game();


}

void CVC_DemoDlg::OnBnClickedButtonMover()
{
	bStop = false;
	 bChangeUser = true;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	minized_all_the_other_windows();
	 


	HANDLE hThread = CreateThread(NULL, 0, changeUser_nawawa, (LPVOID)msdk_handle, 0, NULL);
}

void CVC_DemoDlg::OnBnClickedButtonMoveto()
{
	bFullStop = false;
	bStop = false;
	bChangeUser = true;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	minized_all_the_other_windows();
	Sleep(3000);

	GetDlgItem(IDC_BUTTON_MOVETO)->EnableWindow(false);
	HANDLE hThread = CreateThread(NULL, 0, changeUser_cunqian, (LPVOID)msdk_handle, 0, NULL);
}

void CVC_DemoDlg::OnBnClickedButtonGetmousepos()
{
	UpdateData(TRUE);
	Global_checkTime = 0;
	 
		SetTimer(0, TIMER_LENGTH, NULL);
		m_listLog.ResetContent();
		CTime t = CTime::GetCurrentTime();
		CString t_day = t.Format("%d");
		//m_list_time_log.ResetContent();
		int find_time_before_6 = -1;
		int count = m_list_time_log.GetCount();
		for (int i = 0; i < count; i++)
		{
			CString strTime;
			m_list_time_log.GetText(i, strTime);
			int first, last;

			first = strTime.Find("2020");
			last = strTime.Find("_");
			CString str_day= strTime.Mid(first + 8,2);
			if (str_day != t_day)
			{
				find_time_before_6 = i;
				break;
			}
			else
			{
				first = strTime.Find(")");
				last = strTime.Find(":");
				CString time = strTime.Mid(first + 1, last - first - 1);
				time.Trim();
				if(atol(time)<6)
				{ 
				find_time_before_6 = i;
				break;
				} 

			}
			
		}
		if (find_time_before_6 != -1)
		{
			int delnum=count - find_time_before_6;
			for (int i = 0; i < delnum; i++)
				m_list_time_log.DeleteString(find_time_before_6);
		}
		m_editLogInfor.SetWindowTextA("reset");
		CString infor;
		infor.Format("full stop  time remains %ld today %ld\r\n", pDlg->m_checkTimes - Global_checkTime, m_list_time_log.GetCount());

		pDlg->m_editLogInfor.SetWindowTextA(infor);
		bStop = true;
		bFullStop = true;
		pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
		pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(true);
		pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(true);
	 
}


void CVC_DemoDlg::OnBnClickedButtonKeypress3()
{
	bStop = true;
	bFullStop = true;
	addLog("stop");
	//pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
	//pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(true);

}


void CVC_DemoDlg::OnBnClickedButtonOpen2()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	HANDLE hThread = CreateThread(NULL, 0, fenjie_zhuangbei, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

	//fenjie_zhuangbei(msdk_handle);
	


}


void CVC_DemoDlg::OnBnClickedButtonKeyChangeUser()
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

	GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(false); 
	GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(false);

	HANDLE hThread = CreateThread(NULL, 0, changeUser_And_Login_Thread, (LPVOID)msdk_handle, 0, NULL);

}


void CVC_DemoDlg::OnBnClickedButtonKeypress4()
{
	minized_all_the_other_windows();
	//HANDLE hThread = CreateThread(NULL, 0, StartServer, (LPVOID)this, 0, NULL);// TODO: 在此添加控件通知处理程序代码

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
	bFullStop = false;
	bChangeUser = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}

	Sleep(3000);
	if (b_SiWangTa_Dnf == TRUE)
	{
		HANDLE hThread = CreateThread(NULL, 0, changeUser_And_Login_siwangTa_Thread, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

	}
	else
	{
	HANDLE hThread = CreateThread(NULL, 0, changeUser_And_Login_Thread, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

	}

}
void CVC_DemoDlg::minized_all_the_other_windows()
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
		::GetClassName(pMainWnd->m_hWnd, strClassName.GetBufferSetLength(100), 100);
		::GetWindowText(pMainWnd->m_hWnd, text.GetBufferSetLength(256), 256);

		if (::IsWindowVisible(pMainWnd->m_hWnd))
		{
			//addLog("window  " + text + " class " + strClassName);
			if (strClassName.Find(m_edit_keyword) != -1 ||strClassName.Find("TMob") != -1 || text.Find(m_edit_keyword) != -1 || text.Find("notepad++") != -1 
				|| text.Find("CPU") != -1 || text.Find("WeGame") != -1 || text.Find("MobaRDP") != -1|| text.Find("Visual Studio") != -1 || text.Find("Moba") != -1 || text.Find("admin") != -1)
			{
				 
				if (text.Find("WeGame") != -1 && m_bMaxmizieWegame_window == true)
				{
					
					// ::PostMessage(pMainWnd->m_hWnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
					
				}
				if (text.Find("WeGame") != -1)
				{
					CRect rect;
					::GetWindowRect(pMainWnd->m_hWnd, &rect);
					CString strWindow;
					strWindow.Format("%lx,%s,%s,(%ld)",pMainWnd->m_hWnd,strClassName, text,rect.Width());

					if (rect.Width() ==1920 || rect.Width() == 1366 )
					{ 
						strWindow.Format(_T("xxxx%lx,(%ld,%ld,%ld,%ld)"), pMainWnd->m_hWnd, rect.left, rect.top, rect.Width(), rect.Height());
						 
					}
					else
					{
						addLog("window  " + text + " infor " + strWindow);
						::PostMessage(pMainWnd->m_hWnd, WM_CLOSE, NULL, 0);
					}
					
					//
				}
			}
			else
			{
				if (text != "")
				{
					//addLog("hide window  " + text + " class " + strClassName);
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

void CVC_DemoDlg::OnBnClickedButtonKeypress6()
{
	if (GetSystemMetrics(SM_CXSCREEN) < 1360)
		ShowWindow(SW_MINIMIZE);
	
	minized_all_the_other_windows();

	
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	OnBnClickedButtonKeypress4();
	if (msdk_handle != INVALID_HANDLE_VALUE)
	{
		GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(false);
		
		GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(false);
		 
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
	::WritePrivateProfileString(APP_NAME, "m_rate", rr, "keypressDemo.ini");
}


void CVC_DemoDlg::OnEnChangeEdit6()
{
	UpdateData();

}


void CVC_DemoDlg::OnBnClickedButtonMover2()
{
	bStop = false;
	bChangeUser = true;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	minized_all_the_other_windows();
	//Sleep(3000);

	GetDlgItem(IDC_BUTTON_MOVER2)->EnableWindow(false);
	HANDLE hThread = CreateThread(NULL, 0, changeUser_xiuli_fenjieji, (LPVOID)msdk_handle, 0, NULL);
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
	bStop = true;
	bFullStop = true;
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS)->EnableWindow(true);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS6)->EnableWindow(true);
	pDlg->GetDlgItem(IDC_BUTTON_KEYPRESS5)->EnableWindow(true);
	minized_all_the_other_windows();
	ShellExecute(this->m_hWnd, "open", "d://立即下线.bat", NULL, "", SW_SHOWMAXIMIZED);
}


void CVC_DemoDlg::OnEnChangeEdit7()
{
	UpdateData();
	CString rr;
	rr.Format("%d", m_screenWidth);
	CWinApp* pApp = AfxGetApp();
	::WritePrivateProfileString(APP_NAME, "m_screenWidth", rr, "keypressDemo.ini");
}


void CVC_DemoDlg::OnBnClickedButtonOpen3()
{ 
	m_bMaxmizieWegame_window = true;
	minized_all_the_other_windows();
	m_bMaxmizieWegame_window = false;

	M_DelayRandom(1450, 1550);
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}

    ShowWindow(SW_SHOWMINIMIZED);
	M_DelayRandom(1450, 1550);
	 
	//这个函数以0,0开头,移动坐标才正确
	findImageInWholeWindow_and_click("home_button.png", 0, 0, 1300, 80, msdk_handle, 5, 0,35);
	 
	M_DelayRandom(2450, 2550);
	findImageInWholeWindow_and_click("dnf_button.png", 0, 0, 200, 768, msdk_handle, 5, 0, 15);
	M_DelayRandom(2450, 2550);
	findImage_and_click("qidong.png", 400, 600, 1920, 1080, msdk_handle, 5, 1);
	return;
	//bStop = false;
	//if (msdk_handle == INVALID_HANDLE_VALUE) {
	//	OnBnClickedButtonOpen();
	//}
	//HANDLE hThread = CreateThread(NULL, 0, continue_move_right, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

	//return;
	unsigned int RetSw = 0;
	 
 
	minized_all_the_other_windows();
	 

	bStop = false;
	bFullStop = false;
	bChangeUser = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	CPoint pt =findImage_in_subrect("safe_mode.png", 300, 130,460, 180);
	if (pt.x != 0 && pt.y != 0)
	{

		CString str = "";
		CTime t = CTime::GetCurrentTime();
		CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
		str.Format("%s==>%s (%ld,%ld)\n", tt, "发现safe_mode ", pt.x, pt.y);

		//str.Format("%s==>%s (%ld,%ld)\n", tt, "发现在在游戏中 0",cp.x,cp.y);
		addLog(str);
		bStop = true;
		bFullStop = true;
		addLog("stop");
	}
	else
	{

		CString str = "";
		CTime t = CTime::GetCurrentTime();
		CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

		str.Format("%s==>%s (%ld,%ld)\n", tt, "未现safe_mode 0", pt.x, pt.y);
		addLog(str);
	}

		pt = findImage("safe_mode.png", 200, 150, 600, 500);//
		if (pt.x != 0 && pt.y != 0)
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
			str.Format("%s==>%s (%ld,%ld)\n", tt, "发现1safe_mode ", pt.x, pt.y);

			//str.Format("%s==>%s (%ld,%ld)\n", tt, "发现在在游戏中 0",cp.x,cp.y);
			addLog(str);
			bStop = true;
			bFullStop = true;
			addLog("stop");
		}
		else
		{

			CString str = "";
			CTime t = CTime::GetCurrentTime();
			CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

			str.Format("%s==>%s (%ld,%ld)\n", tt, "未现safe_mode 0", pt.x, pt.y);
			addLog(str);
		}

	addLog("Path=" + strAppPath); 
	//findImage_and_click("12_close.png", 400, 80, 620, 180, msdk_handle, 1, 0);
	return;
	//checkGame_state();
	  //findImage_and_click("wujin.png", 180, 135, 440, 435, msdk_handle, 1);
	//  findImage_and_click("quchu.png", 240, 150, 450, 600, msdk_handle, 1,1,140,0);
	do {
		char* buyNum_huanglong = "20";
		char* buyNum_qinglong = "14";
		RetSw = M_DelayRandom(1000, 1100);
		if (bStop)break;
		//点击小铁柱
		int click_result = findImage_and_click("xiaotiezhu.png", 0, 0, 800, 600, msdk_handle, 1001);
		if (click_result == true)
		{
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			click_result = findImage_and_click("shangdian.png", 250, 0, 800, 450, msdk_handle, 1001);

			RetSw = M_ResetMousePos(msdk_handle);
			if (pDlg->bHuangLong == true)
			{
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1710) / rate), (int)((405) / rate));
				RetSw = move_to_relativePos(msdk_handle, 354, 175);
				addLog("g购买黄龙");
			}
			else
			{
				//RetSw = my_new_MoveTo(msdk_handle, (int)((1324) / rate), (int)((441) / rate));
				RetSw = move_to_relativePos(msdk_handle, 177, 175);
				addLog("购买到青龙");
			}
			RetSw = M_DelayRandom(500, 600);

			M_KeyDown(msdk_handle, Keyboard_LeftShift);
			RetSw = M_DelayRandom(500, 600);
			my_hook_left_Click(msdk_handle, 1);
			RetSw = M_DelayRandom(500, 600);
			M_KeyUp(msdk_handle, Keyboard_LeftShift);

			if (dleft <= 600)
			{
				if (pDlg->bHuangLong == true)
				{
					RetSw = M_KeyInputString(msdk_handle, buyNum_huanglong, strlen(buyNum_huanglong));
				}
				else
				{
					RetSw = M_KeyInputString(msdk_handle, buyNum_qinglong, strlen(buyNum_qinglong));
				}
			}

			RetSw = M_DelayRandom(1000, 1100);
			//my_hook_left_Click(msdk_handle, 1);
			click_result = findImage_and_click("confirm.png", 0, 0, 800, 450, msdk_handle, 1001);
			RetSw = M_DelayRandom(1000, 1100);

			my_hook_left_Click(msdk_handle, 1);
			RetSw = M_DelayRandom(500, 600);

			if (bStop)break;
			RetSw = M_DelayRandom(1000, 1100);
			if (bStop)break;
			addLog("按下Keyboard_ESCAPE键");
			RetSw = my_hook_KeyPress(msdk_handle, Keyboard_ESCAPE, 2);

		}
	} while (0);
	//CPoint cp = findImage("fightagain.png", 600, 50, 650, 90);
	//CPoint cp = findImage ("sysmenu.png", 320, 80, 480, 200);
	//CPoint cp = findImage("ingamenew.png", 340, 40, 480, 80);
	//if (cp.x == 0)
	//CPoint cp = findImage("xuanzeditu.png", 320, 300, 430, 330);
	/*if (cp.x != 0 && cp.y != 0)
	{

		CString str;
		str.Format("%s (%ld,%ld)\n", "发现sysmenu 0", cp.x, cp.y);
		addLog(str);

	}
	else
	{

		CString str;
		str.Format(" %s (%ld,%ld)\n", "未在sysmenu 0", cp.x, cp.y);
		addLog(str);

	}*/
	//bool click_result=findImage_and_click("cailiao.png", 540, 200, 660, 300, msdk_handle,1);
	//if (click_result == true)
	//{
	//	click_result = findImage_and_click("fangru.png", 300, 200, 480, 480, msdk_handle, 1);
	//}
	//if (click_result == true)
	//{
	//	click_result = findImage_and_click("tiaozhanshu.png", 460, 200, 720, 480, msdk_handle,4);
	//}

	 

	//CString strPath;

	//GetCurrentDirectory(MAX_PATH, strPath.GetBuffer(MAX_PATH));

	//strPath.ReleaseBuffer();

	//AfxMessageBox(strPath);
	
	//RetSw = M_KeyInputStringGBK(msdk_handle,"a你好" , strlen("a你好"));
	//HANDLE hThread = CreateThread(NULL, 0, testThread_Game, (LPVOID)msdk_handle, 0, NULL);


	//HANDLE hThread = CreateThread(NULL, 0, fighting_Thread, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码



	 
	/*CPoint cp = findImage("ingamenew.png", 300, 40, 400, 80);
	if (cp.x != 0 && cp.y != 0)
	{

		 CString str; 
		str.Format("%s (%ld,%ld)\n", "发现在游戏中 0", cp.x, cp.y);
		addLog(str); 
		 
	}
	else
	{

		CString str;
		str.Format(" %s (%ld,%ld)\n", "未在游戏中 0", cp.x, cp.y);
		addLog(str);
		 
	}
	 cp = findImage("ingame.png", 0, 0, 100, 100);
	if (cp.x != 0 && cp.y != 0)
	{

		CString str;
		str.Format("%s (%ld,%ld)\n", "ingame 未在游戏中 0", cp.x, cp.y);
		addLog(str);

	}
	else
	{

		CString str;
		str.Format(" %s (%ld,%ld)\n", "在游戏中 0", cp.x, cp.y);
		addLog(str);

	}*/
	 
	return;
	 
	//if (isDNFWindow() == TRUE)
	//{
	//	addLog("是DNF");
	//}
	//else
	//{
	//	addLog("No DNF");
	//}
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

 



	saveScreen();//390,54
	// checkGame_state();
//	OnBnClickedButtonKeypress4();


	//#测试是否在游戏中
	//CPoint pt = findImage("yaluo.png", 150, 100, 650, 400);
	//发现广告
	// CPoint cp = findImage("close.png", 380, 440, 390, 460);
	 //是否在游戏中
	//CPoint cp = findImage("ingamenew.png", 340, 40, 400, 60);
	//if (cp.x != 0 && cp.y != 0)
	//{

	//	CString str = "";
	//	CTime t = CTime::GetCurrentTime();
	//	CString tt = t.Format("%Y-%m-%d_%H-%M-%S");
	//	str.Format("%s==>%s (%ld,%ld)\n", tt, "发现广告关闭 0", cp.x, cp.y);

	//	//str.Format("%s==>%s (%ld,%ld)\n", tt, "发现在在游戏中 0",cp.x,cp.y);
	//	addLog(str);
	//}
	//else
	//{

	//	CString str = "";
	//	CTime t = CTime::GetCurrentTime();
	//	CString tt = t.Format("%Y-%m-%d_%H-%M-%S");

	//	str.Format("%s==>%s (%ld,%ld)\n", tt, "未在游戏中 0", cp.x, cp.y);
	//	addLog(str);
	//}

 
	//CStdioFile file;
	//if (file.Open(_T("log.txt"), CFile::typeText | CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate, NULL))
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
	::WritePrivateProfileString(APP_NAME, "bHuangLong", rr, "keypressDemo.ini");
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
	if (nIDEvent==0&&time.GetHour() == 6 && time.GetMinute() < 10)
	//if (nIDEvent == 0 && time.GetHour() == 21 && time.GetMinute() < 60)
	{
		if (msdk_handle == INVALID_HANDLE_VALUE) {
			OnBnClickedButtonOpen();
		}
		//这里先点击一下dnf窗口 
		move_to_relativePos(msdk_handle, 50, 50);
		M_DelayRandom(800, 1000);
		M_LeftClick(msdk_handle, 1);
		M_DelayRandom(1800, 2000);
		GetDlgItem(IDC_BUTTON_MOVER4)->EnableWindow(true);
		CWnd* pMainWnd = AfxGetMainWnd()->GetForegroundWindow();
		Global_checkTime = 0;
		CString strClassName;
		CString text;
		CString strCurrentWindow;
		GetClassName(pMainWnd->m_hWnd, strClassName.GetBufferSetLength(100), 100);
		::GetWindowText(pMainWnd->m_hWnd, text.GetBufferSetLength(256), 256);
		if (text.Find(m_edit_keyword) != -1)
		{ 
			bStop = true;
			bFullStop = true;
			Global_checkTime = 0; 
			addLog("早起动");
			KillTimer(0);
			

			SetTimer(1, 60000, NULL);
			
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
	else if(nIDEvent == 0 )
	{
		/*if (time.GetHour() ==0 && time.GetMinute() > 10)
		{
			OnBnClickedButtonKeypress8();
			KillTimer(0);
		}*/
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
	if (nIDEvent == 1)
	{
		KillTimer(nIDEvent);
		bChangetofirstUser = true;
		playerlogin();

	}
	if (nIDEvent == 2)
	{
		KillTimer(nIDEvent);
		OnBnClickedButtonUpdate();

	}if (nIDEvent == 3)
	{
		KillTimer(nIDEvent);
		OnBnClickedButtonKeypress6(); 

	}
	if (nIDEvent == 4)
	{
		KillTimer(nIDEvent);
		OnBnClickedButtonMoveto();

	}if (nIDEvent == 5)
	{
		KillTimer(nIDEvent);
		OnBnClickedButtonMover6();

	}
	if (nIDEvent == 6)
	{
		KillTimer(nIDEvent);
		OnBnClickedButtonMover5();;

	}if (nIDEvent == 7)
	{
		KillTimer(nIDEvent);
		OnBnClickedButtonKeypress8();

	}
	CDialogEx::OnTimer(nIDEvent);
}


void CVC_DemoDlg::OnClose()
{
	KillTimer(0);
	UnregisterHotKey(m_hWnd, 1000);
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
	 if(((CButton*)GetDlgItem(IDC_CHECK2))->GetCheck()==1)
	((CEdit*)GetDlgItem(IDC_EDIT8))->SetReadOnly(TRUE);
	else
		((CEdit*)GetDlgItem(IDC_EDIT8))->SetReadOnly(FALSE);
	 
}


void CVC_DemoDlg::OnEnChangeEdit8()
{
	UpdateData();
	 
	CString rr;
	rr.Format("%s", m_matchinename);
	CWinApp* pApp = AfxGetApp();
	::WritePrivateProfileString(APP_NAME, "m_matchname", rr, "keypressDemo.ini");
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
	bOnlyForTest = true;
	
}


void CVC_DemoDlg::OnBnClickedCheck4()
{UpdateData();
	b_SiWangTa_Dnf= m_bSiwanTa;
	CString strInfor;
	UpdateData();
	CString rr;
	rr.Format("%d", b_SiWangTa_Dnf);
	::WritePrivateProfileString(APP_NAME, "b_SiWangTa_Dnf", rr, "keypressDemo.ini");
	// TODO: 在此添加控件通知处理程序代码
}


void CVC_DemoDlg::OnBnClickedButtonOpen4()
{

	minized_all_the_other_windows();

 
	if (msdk_handle == INVALID_HANDLE_VALUE) {
			OnBnClickedButtonOpen();
		}
		OnBnClickedButtonKeypress4();
		if (msdk_handle != INVALID_HANDLE_VALUE)
		{
			bStop = false;
			HANDLE hThread = CreateThread(NULL, 0, duanzao_space, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

		}
		 
	
}

LRESULT CVC_DemoDlg::OnHotKey(WPARAM wParam, LPARAM lParam)
{
	//wParam是注册热键的ID，lParam是关于按键的信息
	if (wParam == 1000)
	{
		bStop = true;
		bFullStop = true;
		addLog("hotkey stop");
	}
	 
		return 0;
}

void CVC_DemoDlg::OnLbnSelchangeList3()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CVC_DemoDlg::OnBnClickedButtonMover3()
{
	CString str;
	 
	for (int i = m_listLog.GetCount()-1; i >0 ; i--)
	{
		CString strlog;
		 m_listLog.GetText(i, strlog);
		 str = str + strlog + "\r\n";
	} 
	CStdioFile file;
	if (file.Open(_T("logView.txt"), CFile::typeText | CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate, NULL))
	{
		 
		file.WriteString(str);
		file.Close();
	}
	::ShellExecute(this->m_hWnd, "open", "d://log.txt", NULL, NULL, SW_SHOW);

	//::ShellExecute(this->m_hWnd, "open", "d://key.reg", NULL, NULL, SW_SHOW);
	 
}


void CVC_DemoDlg::OnBnClickedButtonMover4()
{
	GetDlgItem(IDC_BUTTON_MOVER4)->EnableWindow(false);
	SetTimer(0, TIMER_LENGTH, NULL);
	 
	m_editLogInfor.SetWindowTextA("reset");
	CString infor;
	infor.Format("full stop  time remains %ld \r\n", pDlg->m_checkTimes - Global_checkTime);

	pDlg->m_editLogInfor.SetWindowTextA(infor);
}


 

CString GetLocalIP()
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (err != 0)
	{
		return "";
	}

	char szHostName[MAX_PATH] = { 0 };
	int nRetCode;
	nRetCode = gethostname(szHostName, sizeof(szHostName));

	char* lpLocalIP;
	PHOSTENT hostinfo;

	if (nRetCode != 0)
	{
		WSACleanup();
		return "";
	}

	hostinfo = gethostbyname(szHostName);
	lpLocalIP = inet_ntoa(*(struct in_addr*) * hostinfo->h_addr_list);

	WSACleanup();

	return lpLocalIP;
}
void CVC_DemoDlg::OnBnClickedButtonKeyOnScreen2()
{
 //ShellExecute(this->m_hWnd, "open", "http://192.168.1.166/D%3A/username.txt", NULL, NULL, SW_SHOW);
	
	//CString sHomeUrl(_T("http://www.baidu.com"));
	//CString sPageUrl(_T("/"));
	//CString sResult(_T(""));

	//HttpRequestGet(sHomeUrl, sPageUrl, sResult); 
	//m_editLogInfor.SetWindowTextA(sResult);

	//初始化
	AfxSocketInit();

	//创建 CSocket 对象
	CSocket aSocket;
	CString strIP = "192.168.1.166";
	//CString strIP = "127.0.0.1";
	CString strPort="5006";
	CString strText = GetLocalIP();
	 

	//初始化 CSocket 对象, 因为客户端不需要绑定任何端口和地址, 所以用默认参数即可
	if (!aSocket.Create())
	{
		char szMsg[1024] = { 0 };
		sprintf(szMsg, "create faild: %d", aSocket.GetLastError());
		AfxMessageBox(szMsg);
		return;
	}

	//转换需要连接的端口内容类型
	int nPort = atoi(strPort);

	//连接指定的地址和端口
	if (aSocket.Connect(strIP, nPort))
	{
		char szRecValue[1024] = { 0 };

		//发送内容给服务器
		aSocket.Send(strText, strText.GetLength());

		//接收服务器发送回来的内容(该方法会阻塞, 在此等待有内容接收到才继续向下执行)
		aSocket.Receive((void*)szRecValue, 1024);

		addLog(szRecValue);
		SetDlgItemText(IDC_EDIT3, szRecValue);
		//AfxMessageBox(szRecValue);
	}
	else
	{
		char szMsg[1024] = { 0 };
		sprintf(szMsg, "create faild: %d", aSocket.GetLastError());
		AfxMessageBox(szMsg);
	}

	//关闭
	aSocket.Close();


}


void CVC_DemoDlg::OnBnClickedButtonKeyOnScreen3()
{
	ShellExecute(this->m_hWnd, "open", "cmd.exe", NULL, NULL, SW_SHOW);
}


void CVC_DemoDlg::OnBnClickedCheck5()
{
	UpdateData();
	bslowmode = b_slowMode;

	CString strInfor;
	UpdateData();
	CString rr;
	rr.Format("%d", bslowmode);
	::WritePrivateProfileString(APP_NAME, "bslowmode", rr, "keypressDemo.ini");
	if (bslowmode)
	{
		strInfor.Format("change to bslowmode %d", bslowmode);
		addLog(strInfor);

	}
	else
	{
		strInfor.Format("change to bslowmode  %d", bslowmode);
		addLog(strInfor);
	}
}
void CVC_DemoDlg::extract_exe_file(DWORD ID, CString filename)
{
	HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(ID), TEXT("BIN"));
	if (NULL == hRsrc) {
		return;
	}
	DWORD dwSize = SizeofResource(NULL, hRsrc);
	if (0 == dwSize) {
		return;
	}
	HGLOBAL gl = LoadResource(NULL, hRsrc);
	if (NULL == gl) {
		return;
	}
	LPVOID lp = LockResource(gl);
	if (NULL == lp) {
		return;
	}
	HANDLE fp = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	DWORD a;
	if (!WriteFile(fp, lp, dwSize, &a, NULL)) {
		return;
	}
	CloseHandle(fp);
	FreeResource(gl);

}
void CVC_DemoDlg::extract_png_file(DWORD ID ,CString filename)
{
	HRSRC hRsrc = FindResource(NULL, MAKEINTRESOURCE(ID), TEXT("PNG"));
	if (NULL == hRsrc) {
		return;
	}
	DWORD dwSize = SizeofResource(NULL, hRsrc);
	if (0 == dwSize) {
		return;
	}
	HGLOBAL gl = LoadResource(NULL, hRsrc);
	if (NULL == gl) {
		return;
	}
	LPVOID lp = LockResource(gl);
	if (NULL == lp) {
		return;
	} 
	HANDLE fp = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	DWORD a;
	if (!WriteFile(fp, lp, dwSize, &a, NULL)) {
		return;
	}
	CloseHandle(fp);
	FreeResource(gl);

}
void CVC_DemoDlg::extract_png_files()
{

	extract_png_file(IDB_PNG1, "d://fangru.png");
	extract_png_file(IDB_PNG2, "d://cailiao.png");
	extract_png_file(IDB_PNG3, "d://tiaozhanshu.png");
	extract_png_file(IDB_PNG4, "d://qidong.png");
	extract_png_file(IDB_PNG5, "d://sysmenu.png");
	extract_png_file(IDB_PNG6, "d://savemoney.png");
	extract_png_file(IDB_PNG7, "d://wait_set.png");
	extract_png_file(IDB_PNG8, "d://quchu.png");
	extract_png_file(IDB_PNG9, "d://wujin.png");
	extract_png_file(IDB_PNGA, "d://xiaotiezhu.png");
	extract_png_file(IDB_PNGB,"d://shangdian.png");
	extract_png_file(IDB_PNGC, "d://选择地下城.png");
	extract_png_file(IDB_PNGD, "d://confirm.png");
	extract_png_file(IDB_PNGH, "d://close.png");
	extract_png_file(IDB_PNGI, "d://select_all.png");
	extract_png_file(IDB_PNGJ, "d://fenjie_button.png");
	extract_png_file(IDB_PNGK, "d://12_close.png");
	extract_exe_file(IDR_BIN1, "d://key.reg");
	extract_png_file(IDB_PNGM, "d://safe_mode.png");
	extract_png_file(IDB_PNGN, "d://dnf_button.png");
	extract_png_file(IDB_PNGO, "d://home_button.png");
	//extract_exe_file(IDR_BIN1, "d://wget.exe");
	extract_exe_file(IDR_BIN2, "d://runwegame.bat");
}


void CVC_DemoDlg::OnBnClickedButtonMover5()
{
	m_bMaxmizieWegame_window = true;
	minized_all_the_other_windows();
	m_bMaxmizieWegame_window = false;

	M_DelayRandom(1450, 1550);
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}

	ShowWindow(SW_SHOWMINIMIZED);
	M_DelayRandom(1450, 1550);
	//if (m_screenWidth == 1920)
	//{
	//	M_ResolutionUsed(msdk_handle, 1920, 1080);
	//}
	//else
	//{
	//	M_ResolutionUsed(msdk_handle, 1366, 768);
	//}'
	findImage_and_click("home_button.png", 500, 0, 1300, 80, msdk_handle, 4, 1);
	findImage_and_click("home_button.png", 0, 0, 1300, 80, msdk_handle, 2, 1);
	M_DelayRandom(1450, 1550);

	M_DelayRandom(1450, 1550);
	findImage_and_click("dnf_button.png", 0, 80, 200, 300, msdk_handle,4, 1);
	findImage_and_click("dnf_button.png", 0, 80, 200, 300, msdk_handle, 2, 1);

	M_DelayRandom(1450, 1550);

	M_DelayRandom(1450, 1550);
	findImage_and_click("qidong.png", 400, 600, 1920, 1080, msdk_handle, 5, 1);
	findImage_and_click("qidong.png", 400, 600, 1920, 1080, msdk_handle, 2, 1);
	return;

	CPoint cp = findImage("qidong.png", 400, 600,1920 , 1080);
	if (cp.x != 0 && cp.y != 0)
	{

		CString str;
		str.Format("%s (%ld,%ld)\n", "发现在qidong中 0", cp.x, cp.y);
		addLog(str);

		//cp.x = 100;
		//cp.y = 100;


		M_ResetMousePos(msdk_handle);
		/*long changeX = cp.x /2*1.15*rate;
		long changeY = cp.y /2*1.59 * rate;*/
		long changeX = cp.x *1.45 ;
		long changeY = cp.y * 1.45;
		/*if (m_screenWidth == 1920)
		{
			  changeX = cp.x / 2 * 1.10;
			  changeY = cp.y / 2 * 1.60;
		}
		else
		{
			 changeX = cp.x / 2 * 1.15 * rate;
			 changeY = cp.y / 2 * 1.59 * rate;
		}*/
	 	CString infor;
		infor.Format("move mouse to %ld,%ld ,传入参数 %ld ,%ld", cp.x, cp.y, changeX, changeY);
		addLog(infor); 
		M_MoveTo3(msdk_handle, changeX, changeY);

		my_hook_left_Click(msdk_handle, 1);
	}
	else
	{

		CString str;
		str.Format(" %s (%ld,%ld)\n", "未在qidong中 0", cp.x, cp.y);
		addLog(str);

	}
}


void CVC_DemoDlg::OnBnClickedButtonMover6()
{
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	bFullStop = false;
	bStop = false;
	HANDLE hThread = CreateThread(NULL, 0, changeUser_backtohome, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

}


void CVC_DemoDlg::OnBnClickedButtonMover7()
{
	bFullStop = false;
	bStop = false;
	bChangeUser = true;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	minized_all_the_other_windows();
	Sleep(3000);

	GetDlgItem(IDC_BUTTON_MOVER7)->EnableWindow(false);
	HANDLE hThread = CreateThread(NULL, 0, changeUser_maipiao, (LPVOID)msdk_handle, 0, NULL);
}


void CVC_DemoDlg::OnBnClickedButtonOpen5()
{
	bStop = false;
	if (msdk_handle == INVALID_HANDLE_VALUE) {
		OnBnClickedButtonOpen();
	}
	HANDLE hThread = CreateThread(NULL, 0, continue_move_right, (LPVOID)msdk_handle, 0, NULL);// TODO: 在此添加控件通知处理程序代码

	return;
}

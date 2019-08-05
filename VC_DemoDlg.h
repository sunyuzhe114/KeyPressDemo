
// VC_DemoDlg.h : 头文件
//

#pragma once


// CVC_DemoDlg 对话框
class CVC_DemoDlg : public CDialogEx
{
	// 构造
public:
	CVC_DemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_VC_DEMO_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	HANDLE msdk_handle;

	// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	static void runinThread();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonKeypress();
	afx_msg void OnBnClickedButtonMover();
	afx_msg void OnBnClickedButtonMoveto();
	afx_msg void OnBnClickedButtonGetmousepos();
	afx_msg void OnBnClickedButtonKeypress3();
	int m_intMinute;
	void saveScreen();
	CEdit m_editLog;
	afx_msg void OnBnClickedButtonOpen2();
	CEdit m_editLogInfor;
	afx_msg void OnBnClickedButtonKeypress5();
	afx_msg void OnBnClickedButtonKeypress4();
	afx_msg void OnEnChangeEdit2();
	CListBox m_listWindow;
	CString m_edit_keyword;
	afx_msg void OnBnClickedButtonKeypress6();
	double m_rate;
	afx_msg void OnEnChangeEdit5();
	CEdit m_editRate;
	int m_checkTimes;
	afx_msg void OnEnChangeEdit6();
	afx_msg void OnBnClickedButtonMover2();
	afx_msg void OnEnChangeEdit1();
};

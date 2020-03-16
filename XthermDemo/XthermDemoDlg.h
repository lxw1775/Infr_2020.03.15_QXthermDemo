// XthermDemoDlg.h : 头文件
//

#pragma once

#include "vfw.h"
// CXthermDemoDlg 对话框
class CXthermDemoDlg : public CDialog
{
// 构造
public:
	CXthermDemoDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_XTHERMDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButRefl();
public:
	afx_msg void OnBnClickedButStart();
public:
	afx_msg void OnBnClickedButStop();
public:
	afx_msg void OnCbnSelchangeComboColor();
public:
	afx_msg void OnBnClickedCheckTmptype();
public:
	afx_msg void OnCbnSelchangeComboSrc();
 
public:
	afx_msg void OnBnClickedButTmp();
public:
	afx_msg void OnBnClickedButNuc();
public:
	afx_msg void OnCbnSelchangeComboCard();
public:
	afx_msg void OnBnClickedButPos1();
public:
	afx_msg void OnBnClickedButTmpset();
public:
	afx_msg void OnBnClickedButSavet();
public:
	afx_msg void OnBnClickedButSaveorg();
public:
	afx_msg void OnBnClickedButTest();
public:
	afx_msg void OnBnClickedCheckRecord();
			afx_msg int ChooseCompressor(COMPVARS *aCOMPVARS, HWND aParent = NULL );
		  afx_msg int GetCompressorName(COMPVARS *aCOMPVARS, CString &aName);

public:
	afx_msg void OnBnClickedCheckTemp();
};

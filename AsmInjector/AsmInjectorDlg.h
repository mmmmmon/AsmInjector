
// AsmInjectorDlg.h: 头文件
//

#pragma once
#include "ProcessHunter.h"

// CAsmInjectorDlg 对话框
class CAsmInjectorDlg : public CDialogEx
{
// 构造
public:
	CAsmInjectorDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ASMINJECTOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	auto CAsmInjectorDlg::PrintLog(CString logstr) -> void;
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	auto CAsmInjectorDlg::GetStringTableStrById(int nIDString)->CString;
	auto CAsmInjectorDlg::OnRclickListFreeMemory() -> void;
	afx_msg void OnBnClickedButtonInject();
	CEdit asmEdit;
	CListCtrl memFreeList;
	ProcessHunter Chick;
	CEdit Log;
	afx_msg void OnBnClickedButtonResumeProcess();
	afx_msg void OnBnClickedButtonSuspendProcess();
	afx_msg void OnRclickListFreeMemory(NMHDR* pNMHDR, LRESULT* pResult);
};

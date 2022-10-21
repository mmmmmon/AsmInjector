
// AsmInjectorDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "AsmInjector.h"
#include "AsmInjectorDlg.h"
#include "afxdialogex.h"
#include "Basic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAsmInjectorDlg 对话框
CAsmInjectorDlg::CAsmInjectorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ASMINJECTOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAsmInjectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ASM, asmEdit);
	DDX_Control(pDX, IDC_LIST_FREE_MEMORY, memFreeList);
	DDX_Control(pDX, IDC_STATIC_CHICK, Chick);
	DDX_Control(pDX, IDC_EDIT_LOG, Log);
}

#pragma warning( push )
#pragma warning( disable : 26454 )
BEGIN_MESSAGE_MAP(CAsmInjectorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_INJECT, &CAsmInjectorDlg::OnBnClickedButtonInject)
	ON_BN_CLICKED(IDC_BUTTON_RESUME_PROCESS, &CAsmInjectorDlg::OnBnClickedButtonResumeProcess)
	ON_BN_CLICKED(IDC_BUTTON_SUSPEND_PROCESS, &CAsmInjectorDlg::OnBnClickedButtonSuspendProcess)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FREE_MEMORY, &CAsmInjectorDlg::OnRclickListFreeMemory)
	ON_COMMAND(ID_FREE, &CAsmInjectorDlg::OnRclickListFreeMemory)
END_MESSAGE_MAP()
#pragma warning( pop )

auto CAsmInjectorDlg::OnRclickListFreeMemory() -> void
{
	auto position = memFreeList.GetFirstSelectedItemPosition();
	if (nullptr != position)
	{
		const auto row = memFreeList.GetNextSelectedItem(position);
		std::wstring processIdStr = memFreeList.GetItemText(row, 0).GetString();
		std::wstring baseStr = memFreeList.GetItemText(row, 1).GetString();
		const auto processId = std::stoi(processIdStr, 0, 10);
		const auto base = reinterpret_cast<void*>(std::stoll(baseStr, 0, 16));

		const auto processHandle = ::OpenProcess(PROCESS_VM_OPERATION, FALSE, processId);
		if (TRUE == ::VirtualFreeEx(processHandle, base, 0, MEM_RELEASE))
		{
			memFreeList.DeleteItem(row);
			std::wstring log = GetStringTableStrById(IDS_STRING_FREEING_MEMORY).GetString();
			log = log + SPACE  + baseStr + COLON + GetStringTableStrById(IDS_STRING_SUCCESS).GetString();
			PrintLog(log.c_str());
		}
		else
		{
			std::wstring log = GetStringTableStrById(IDS_STRING_FREEING_MEMORY).GetString();
			log = log + SPACE + baseStr + COLON + GetStringTableStrById(IDS_STRING_UNSUCCESS).GetString();
			PrintLog(log.c_str());
		}
	}
	return;
}

auto CAsmInjectorDlg::GetStringTableStrById(int nIDString) -> CString
{
	CString strFormat;
	auto bufferSize = strFormat.LoadString(nIDString);
	return strFormat;
}


// CAsmInjectorDlg 消息处理程序

BOOL CAsmInjectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	memFreeList.SetExtendedStyle(memFreeList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	memFreeList.InsertColumn(0, GetStringTableStrById(IDS_STRING_PROCESS_ID), LVCFMT_CENTER, 40);
	memFreeList.InsertColumn(1, GetStringTableStrById(IDS_STRING_MEMORY_ADDR), LVCFMT_CENTER, 100);
	memFreeList.InsertColumn(2, GetStringTableStrById(IDS_STRING_MEMORY_SIZE), LVCFMT_CENTER, 70);

	SetWindowText(GetStringTableStrById(IDS_STRING_TITLE));
	SetDlgItemText(IDC_STATIC_TIPS, GetStringTableStrById(IDS_STRING_TIPS));
	SetDlgItemText(IDC_BUTTON_INJECT, GetStringTableStrById(IDS_STRING_INJECT));
	SetDlgItemText(IDC_BUTTON_SUSPEND_PROCESS, GetStringTableStrById(IDS_STRING_SUSPEND_PROCESS));
	SetDlgItemText(IDC_BUTTON_RESUME_PROCESS, GetStringTableStrById(IDS_STRING_RESUME_PROCESS));
	SetDlgItemText(IDC_CHECK_KERNEL_MODE, GetStringTableStrById(IDS_STRING_KERNEL_MODE));
	SetDlgItemText(IDC_STATIC_LOG, GetStringTableStrById(IDS_STRING_LOG));
	SetDlgItemText(IDC_STATIC_WAIT_FOR_FREEING, GetStringTableStrById(IDS_STRING_WAIT_FOR_FREE_MEM));
	SetDlgItemText(IDC_STATIC_PROCESSID, GetStringTableStrById(IDS_STRING_PROCESS_ID));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CAsmInjectorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAsmInjectorDlg::OnPaint()
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
HCURSOR CAsmInjectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

auto CAsmInjectorDlg::PrintLog(CString logstr) -> void
{
	const auto loglen = Log.GetWindowTextLength();
	logstr += WRAP_LINE;
	Log.SetSel(loglen, loglen);
	Log.ReplaceSel(logstr);
	return;
}

void CAsmInjectorDlg::OnBnClickedButtonInject()
{
	// TODO: 在此添加控件通知处理程序代码
	if (0 == asmEdit.GetWindowTextLengthW())
	{
		MessageBox(GetStringTableStrById(IDS_STRING_EMPTY_WARRING));
		return;
	}
	std::vector<std::wstring> asmCode;
	const auto asmEditLine = asmEdit.GetLineCount();
	const auto processId = GetDlgItemInt(IDC_EDIT_PROCESS_ID);
	for (auto i = 0; i < asmEditLine; i++)
	{
		const auto lineLength = asmEdit.LineLength(asmEdit.LineIndex(i));
		if (0 != lineLength)
		{
			std::vector<wchar_t> lineCode(lineLength);
			asmEdit.GetLine(i, lineCode.data(), lineLength);
			asmCode.push_back(std::wstring(lineCode.data(), lineCode.size()));
		}
	}
	const auto [status,memBlock] = Basic::InjectAsmToProcess(processId, asmCode);
	std::wstring logstr = GetStringTableStrById(IDS_STRING_GENERATE_HEX).GetString();
	logstr += COLON;
	for (auto i = 0; i < std::get<2>(memBlock).size(); i++)
	{
		std::wstring hex;
		std::wstringstream formHexSteam;
		formHexSteam << std::hex << static_cast <unsigned int>(std::get<2>(memBlock)[i]) << " ";
		formHexSteam >> hex;
		std::transform(hex.begin(), hex.end(), hex.begin(), ::toupper);
		logstr += hex;
	}
	PrintLog(logstr.c_str());

	logstr = GetStringTableStrById(IDS_STRING_STATUS).GetString();
	logstr += COLON;
	logstr += Basic::injectStatusStringTable[status];
	PrintLog(logstr.c_str());

	if (Basic::injectStatus::success == status)
	{
		std::stringstream formHexSteam;
		std::string base;
		const auto row = memFreeList.InsertItem(0, std::to_wstring(std::get<0>(memBlock)).c_str());
		formHexSteam  << std::hex  << std::showbase <<std::get<1>(memBlock);
		formHexSteam >> base;
		do
		{
			if ('0' == base[0])
			{
				base.erase(base.begin());
			}
		} while ('0' == base[0]);
		base = "0x0" + base;
		memFreeList.SetItemText(row, 1, Basic::string2wString(base.c_str()).c_str());
		memFreeList.SetItemText(row, 2, std::to_wstring(std::get<2>(memBlock).size()).c_str());
	}
	return;
}


void CAsmInjectorDlg::OnBnClickedButtonResumeProcess()
{
	// TODO: 在此添加控件通知处理程序代码
	std::wstring log; 
	const auto processId = GetDlgItemInt(IDC_EDIT_PROCESS_ID);
	const auto processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	const auto status = Basic::ResumeProcess(processHandle) ? GetStringTableStrById(IDS_STRING_SUCCESS) : GetStringTableStrById(IDS_STRING_UNSUCCESS);
	log = GetStringTableStrById(IDS_STRING_RESUME_PROCESS) + COLON + status;
	PrintLog(log.c_str());
	if (nullptr != processHandle)
	{
		::CloseHandle(processHandle);
	}
}


void CAsmInjectorDlg::OnBnClickedButtonSuspendProcess()
{
	// TODO: 在此添加控件通知处理程序代码
	std::wstring log;
	const auto processId = GetDlgItemInt(IDC_EDIT_PROCESS_ID);
	const auto processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	const auto status = Basic::SuspendProcess(processHandle) ? GetStringTableStrById(IDS_STRING_SUCCESS) : GetStringTableStrById(IDS_STRING_UNSUCCESS);
	log = GetStringTableStrById(IDS_STRING_SUSPEND_PROCESS) + COLON + status;
	PrintLog(log.c_str());
	if (nullptr != processHandle)
	{
		::CloseHandle(processHandle);
	}
}


void CAsmInjectorDlg::OnRclickListFreeMemory(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu popMenu;
	popMenu.LoadMenu(IDR_MENU_FREE_MEM_LIST);
	CPoint posMouse;
	GetCursorPos(&posMouse);
	CMenu* popup = popMenu.GetSubMenu(0);
	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, posMouse.x, posMouse.y, this);
	popMenu.Detach();
	popMenu.DestroyMenu();
	*pResult = 0;
}

#include "pch.h"
#include "Basic.h"
#include "resource.h"
#include "ProcessHunter.h"
#include "AsmInjectorDlg.h"

BEGIN_MESSAGE_MAP(ProcessHunter, CStatic)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()


void ProcessHunter::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SetCapture();
	const auto iconHandle = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
	const auto cursorHandle = OleIconToCursor(nullptr, iconHandle);
	::SetCursor(cursorHandle);
	this->SetIcon(nullptr);
	SetTimer(1, 300, nullptr);
	CStatic::OnLButtonDown(nFlags, point);
}


void ProcessHunter::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ReleaseCapture();
	const auto iconHandle = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
	this->SetIcon(iconHandle);
	KillTimer(1);
	CStatic::OnLButtonUp(nFlags, point);
}



void ProcessHunter::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	const auto desktopHwd= ::GetDesktopWindow();
	const auto deskDC = ::GetWindowDC(desktopHwd);
	const auto oldRop2 = SetROP2(deskDC, R2_NOTXORPEN);
	CPoint cursorPos = {0};
	DWORD processId = 0;
	RECT rect = {0};

	GetCursorPos(&cursorPos);
	const auto hTargetWnd = Basic::SmallestWindowFromPoint(cursorPos);
	if (GetWindowThreadProcessId(GetSafeHwnd(), nullptr) == GetWindowThreadProcessId(hTargetWnd, nullptr)) {
		GetParent()->SetDlgItemText(IDC_STATIC_TIPS, static_cast<CAsmInjectorDlg*>(AfxGetApp()->GetMainWnd())->GetStringTableStrById(IDS_STRING_TIPS));
		return;
	}
	GetWindowThreadProcessId(hTargetWnd, &processId);
	GetParent()->SetDlgItemInt(IDC_EDIT_PROCESS_ID, processId);

	auto titleLen = ::SendMessage(hTargetWnd, WM_GETTEXTLENGTH, 0, 0);
	if (titleLen > 0) {
		titleLen += 1;
		const auto titleBuf = new TCHAR[titleLen];
		ZeroMemory(titleBuf, titleLen);
		::SendMessage(hTargetWnd, WM_GETTEXT, titleLen, (LPARAM)titleBuf);
		GetParent()->SetDlgItemText(IDC_STATIC_TIPS, titleBuf);
		delete[] titleBuf;
	}
	::GetWindowRect(hTargetWnd, &rect);

	if (rect.left < 0) rect.left = 0;
	if (rect.top < 0) rect.top = 0;
	const auto newPen = ::CreatePen(0, 3, RGB(125, 0, 125));
	const auto origPen = ::SelectObject(deskDC, newPen);
	::Rectangle(deskDC, rect.left, rect.top, rect.right, rect.bottom);
	Sleep(200);
	::Rectangle(deskDC, rect.left, rect.top, rect.right, rect.bottom);
	::SetROP2(deskDC, oldRop2);
	::SelectObject(deskDC, origPen);
	::DeleteObject(newPen);
	::ReleaseDC(desktopHwd, deskDC);
	CStatic::OnTimer(nIDEvent);
}

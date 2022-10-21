#pragma once
#include <afxwin.h>
class ProcessHunter :
    public CStatic
{
public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};


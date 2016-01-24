// DateTimeEB.cpp : implementation file
//

#include "stdafx.h"
#include "DateTimeEB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DateTimeEB

BEGIN_MESSAGE_MAP(DateTimeEB, CEdit)
	//{{AFX_MSG_MAP(DateTimeEB)
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DateTimeEB message handlers

bool VAtoi(CString str, int &res)
{
    str.Remove(char(32));
    if (str.IsEmpty()) return false;
    bool error = false;
    for (int i = 0; i < str.GetLength() && !error; i++) {
        int c = ::tolower(str[i]);
        error = !(
            (c >= '0' && c <= '9')
            || ((c == '-' || c == '+') && i == 0)
            );
    }
    if (!error) res = atoi(LPCTSTR(str));
    return !error;
}

template<class TYPE>
void VSetInRange(TYPE &x, TYPE min, TYPE max) 
{
    ASSERT(min<=max);
    if (x < min) x = min; else if (x > max) x = max;
}


DateTimeEB::DateTimeEB( CString strMask )
{
   m_strMask = strMask;
}


//-------------------------------------------------------------------//
// PreSubclassWindow()                                               //
//-------------------------------------------------------------------//
// This allows us to initialize to the current time by default.
// We have to do it after the control is instantiated.
//-------------------------------------------------------------------//
void DateTimeEB::PreSubclassWindow() 
{
    SetDateTime(COleDateTime::GetCurrentTime());
    CEdit::PreSubclassWindow();
}


void DateTimeEB::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) 
{
    CString old;
    GetWindowText(old);
    int pos;
    TCHAR c;
    GetSel(pos, pos);
    switch (nChar) {
        case VK_BACK: pos--;
            break;
        case VK_UP: pos--;
            break;
        case '0':case '1':case '2':case '3':
        case '4':case '5':case '6':case '7':
        case '8':case '9':
            if (pos < old.GetLength()
                  && pos < m_strMask.GetLength()
                  && (UINT)old[pos] != nChar
                  && ((c = m_strMask[pos]) == 'D' || 
                  c == 'M' || c == 'Y' || c == 'h' 
                  || c == 'm' || c == 's')) {
                CString str = old;
                str.SetAt(pos, (TCHAR)nChar);
                COleDateTime dt = GetDateTime(str);
                if (dt.GetStatus() == COleDateTime::valid) SetDateTime(dt);
            }
            if (++pos < m_strMask.GetLength())
                for ( c = m_strMask[pos]; pos < m_strMask.GetLength() 
                    && c != 'D' && 
                    c != 'M' && c != 'Y' && 
                    c != 'h' && c != 'm' && c != 's';
                  c = m_strMask[pos]) pos++;
            break;
        default:
            if (++pos < m_strMask.GetLength())
                for ( c = m_strMask[pos]; pos < m_strMask.GetLength() 
                      && c != 'D' && c != 'M' 
                      && c != 'Y' && c != 'h' 
                      && c != 'm' && c != 's';
                    c = m_strMask[pos]) pos++;
            break;
    }
    SetSel(pos, pos);
}


void DateTimeEB::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    switch (nChar) {
        case VK_DELETE:case VK_INSERT:
            break;
        case VK_UP: case VK_DOWN:
            {
                CString old;
                GetWindowText(old);
                int pos;
                GetSel(pos, pos);
                if (pos < m_strMask.GetLength()) {
                    TCHAR c = m_strMask[pos];
                    if (c != 'D' && c != 'M' && 
                      c != 'Y' && c != 'h' && 
                      c != 'm' && c != 's')
                        if (--pos >= 0) c = m_strMask[pos];
                    tm t;
                    COleDateTime dt = GetDateTime();
                    t.tm_mday = dt.GetDay();
                    t.tm_mon = dt.GetMonth();
                    t.tm_year = dt.GetYear();
                    t.tm_hour = dt.GetHour();
                    t.tm_min = dt.GetMinute();
                    t.tm_sec = dt.GetSecond();
                    switch (c) {
                        case 'D':
                            t.tm_mday += (nChar!=VK_UP) ? -1 : 1;
                            break;
                        case 'M':
                            t.tm_mon += (nChar!=VK_UP) ? -1 : 1;
                            break;
                        case 'Y':
                            t.tm_year += (nChar!=VK_UP) ? -1 : 1;
                            break;
                        case 'h':
                            t.tm_hour += (nChar!=VK_UP) ? -1 : 1;
                            break;
                        case 'm':
                            t.tm_min += (nChar!=VK_UP) ? -1 : 1;
                            break;
                        case 's':
                            t.tm_sec += (nChar!=VK_UP) ? -1 : 1;
                            break;
                        default    :
                             CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
                            break;
                    }
                    dt.SetDateTime(t.tm_year, t.tm_mon, t.tm_mday, 
                                      t.tm_hour, t.tm_min, t.tm_sec);
                    if (dt.GetStatus() == COleDateTime::valid) 
                        SetDateTime(dt);
                } else CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
            }
            break;
        default:
            CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
            break;
    }
}


void DateTimeEB::SetDateTime(COleDateTime dt)
{
    CString str;
    int i, n = m_strMask.GetLength();
    for (i = 0; i < n; i++) {
        CString s;
        int val;
        TCHAR c = m_strMask[i];
        switch (c) {
            case 'D':case 'M':case 'Y':case 'h':case 'm':case 's':
                for (; i < n; i++) {
                    if (m_strMask[i] == c) s += c;
                    else {i--; break;}
                }
                break;
            default :
                break;
        }
        CString format;
        format.Format("%%0%dd", s.GetLength());
        switch (c) {
            case 'D':
                val = dt.GetDay();
                ::VSetInRange(val, 0, 31);
                s.Format(format, val);
                break;
            case 'M':
                val = dt.GetMonth();
                ::VSetInRange(val, 1, 12);
                s.Format(format, val);
                break;
            case 'Y':
                val = dt.GetYear();
                ::VSetInRange(val, 1900, 9990);
                s.Format(format, val);
                break;
            case 'h':
                val = dt.GetHour();
                ::VSetInRange(val, 0, 23);
                s.Format(format, val);
                break;
            case 'm':
                val = dt.GetMinute();
                ::VSetInRange(val, 0, 59);
                s.Format(format, val);
                break;
            case 's':
                val = dt.GetSecond();
                ::VSetInRange(val, 0, 59);
                s.Format(format, val);
                break;
            default :
                s = c;
                break;
        }
        str += s;
    }
    int pos;
    GetSel(pos, pos);
    SetWindowText(str);
    SetSel(pos, pos);
    CWnd *pOwner = GetParent();
    if (pOwner != NULL) 
      pOwner->PostMessage(WM_DATETIME_EDIT, 
                  (WPARAM)DTE_DATETIMECHANGED, (LPARAM)0);
}

COleDateTime DateTimeEB::GetDateTime()
{
    CString str;
    GetWindowText(str);
    return GetDateTime(str);
}

COleDateTime DateTimeEB::GetDateTime(CString str)
{
    tm t;
    COleDateTime dt;
    dt.SetStatus(COleDateTime::invalid);
    CTime::GetCurrentTime().GetLocalTm(&t);
    t.tm_mon += 1;
    t.tm_year += 1900;

    int i, n = m_strMask.GetLength();
    for (i = 0; i < n; i++) {
        CString s;
        TCHAR c = m_strMask[i];
        switch (c) {
            case 'D':case 'M':case 'Y':case 'h':case 'm':case 's':
                for ( ; i < n; i++) {
                    if (str.GetLength() <= s.GetLength()) break;
                    if (m_strMask[i] == c) s += str[s.GetLength()];
                    else {i--; break;}
                }
                break;
            default :
                if (str.GetLength() < 0 || str[0] != (s = c)) return dt;
                break;
        }
        str = str.Right(str.GetLength() - s.GetLength());
        int val;
        switch (c) {
            case 'D':
                if (!::VAtoi(s, val)) return dt;
                ::VSetInRange(val, 1, 31);
                t.tm_mday = val;
                break;
            case 'M':
                if (!::VAtoi(s, val)) return dt;
                ::VSetInRange(val, 1, 12);
                t.tm_mon = val;
                break;
            case 'Y':
                if (!::VAtoi(s, val)) return dt;
                ::VSetInRange(val, 1900, 9990);
                t.tm_year = val;
                break;
            case 'h':
                if (!::VAtoi(s, val)) return dt;
                ::VSetInRange(val, 0, 23);
                t.tm_hour = val;
                break;
            case 'm':
                if (!::VAtoi(s, val)) return dt;
                ::VSetInRange(val, 0, 59);
                t.tm_min = val;
                break;
            case 's':
                if (!::VAtoi(s, val)) return dt;
                ::VSetInRange(val, 0, 59);
                t.tm_sec = val;
                break;
            default :
                break;
        }
    }
    dt.SetStatus(COleDateTime::valid);
    dt.SetDateTime(t.tm_year, t.tm_mon, 
            t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    return dt;
}


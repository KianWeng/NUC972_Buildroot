/*
** $Id: ctrlmisc.c 12039 2009-08-13 01:40:24Z houhuihua $
**
** ctrlmisc.c: the help routines for MiniGUI control.
**
** Copyright (C) 2003 ~ 2008 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** All rights reserved by Feynman Software.
**
** Current maintainer: Yan Xiaowei
**
** Create date: 1999/8/23
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "common.h"
#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "ctrl/ctrlhelper.h"
#include "ctrl/button.h"
#include "ctrlmisc.h"

#define _ID_TIMER   100

static int ToolTipWinProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    static int iBorder;
    switch (message) {
        case MSG_CREATE:
        {
            const WINDOWINFO *info;
            int timeout = (int)GetWindowAdditionalData (hWnd);
            if (timeout >= 10)
                SetTimer (hWnd, _ID_TIMER, timeout / 10);

            info = GetWindowInfo (hWnd);
            iBorder = 
                info->we_rdr->calc_we_area (hWnd, LFRDR_METRICS_BORDER, NULL);
            SetWindowFont (hWnd, 
                (PLOGFONT)GetWindowElementAttr(hWnd, WE_FONT_TOOLTIP));
            break;
        }

        case MSG_TIMER:
            KillTimer (hWnd, _ID_TIMER);
            if(IsWindowVisible(hWnd))
	{
		ShowWindow (hWnd, SW_HIDE);
	}
            break;

        case MSG_PAINT:
        {
            HDC hdc;
            const char* text;
            
            hdc = BeginPaint (hWnd);
 
            text = GetWindowCaption (hWnd);
            SetBkMode (hdc, BM_TRANSPARENT);

            SetTextColor (hdc, GetWindowElementPixel(hWnd, WE_FGC_TOOLTIP));
            TabbedTextOut (hdc, iBorder, iBorder, text);
 
            EndPaint (hWnd, hdc);
            return 0;
        }

        case MSG_DESTROY:
        {
            KillTimer (hWnd, _ID_TIMER);
            return 0;
        }

        case MSG_SETTEXT:
        {
            int timeout = (int)GetWindowAdditionalData (hWnd);
            if (timeout >= 10) {
                KillTimer (hWnd, _ID_TIMER);
                SetTimer (hWnd, _ID_TIMER, timeout / 10);
            }

            break;
        }
    }
 
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

/* ToolTip Win without border or frame. */
HWND CreateToolTipWin (HWND hParentWnd, int x, int y, int timeout_ms, 
                const char* text, ...)
{
    HWND hwnd;
    char* buf = NULL;
    MAINWINCREATE CreateInfo;
    SIZE text_size;

    if (strchr (text, '%')) {
        va_list args;
        int size = 0;
        int i = 0;

        va_start(args, text);
        do {
            size += 1000;
            if (buf) free(buf);
            buf = malloc(size);
            i = vsnprintf(buf, size, text, args);
        } while (i == size);
        va_end(args);
    }
        
    SelectFont (HDC_SCREEN, 
            (PLOGFONT)GetWindowElementAttr(hParentWnd, WE_FONT_TOOLTIP));
    GetTabbedTextExtent (HDC_SCREEN, buf ? buf : text, -1, &text_size);

    if (x + text_size.cx > g_rcScr.right)
        x = g_rcScr.right - text_size.cx;
    if (y + text_size.cy > g_rcScr.bottom)
        y = g_rcScr.bottom - text_size.cy;

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_USEPARENTRDR;
    CreateInfo.spCaption = buf ? buf : text;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor (IDC_ARROW);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ToolTipWinProc;
    CreateInfo.lx = x;
    CreateInfo.ty = y;
    CreateInfo.rx = CreateInfo.lx + text_size.cx;
    CreateInfo.by = CreateInfo.ty + text_size.cy;
    CreateInfo.iBkColor = GetWindowElementPixel (hParentWnd, WE_BGC_TOOLTIP);
    CreateInfo.dwAddData = (DWORD) timeout_ms;
    CreateInfo.hHosting = hParentWnd;

    hwnd = CreateMainWindow (&CreateInfo);

    if (buf)
        free (buf);

    return hwnd;
}

void ResetToolTipWin (HWND hwnd, int x, int y, const char* text, ...)
{
    char* buf = NULL;
    SIZE text_size;

    if (strchr (text, '%')) {
        va_list args;
        int size = 0;
        int i = 0;

        va_start(args, text);
        do {
            size += 1000;
            if (buf) free(buf);
            buf = malloc(size);
            i = vsnprintf(buf, size, text, args);
        } while (i == size);
        va_end(args);
    }

    SelectFont (HDC_SCREEN, 
            (PLOGFONT)GetWindowElementAttr(hwnd, WE_FONT_TOOLTIP));
    GetTabbedTextExtent (HDC_SCREEN, buf ? buf : text, -1, &text_size);

    SetWindowCaption (hwnd, buf ? buf : text);
    if (buf) free (buf);

    if (x + text_size.cx > g_rcScr.right)
        x = g_rcScr.right - text_size.cx;
    if (y + text_size.cy > g_rcScr.bottom)
        y = g_rcScr.bottom - text_size.cy;

    MoveWindow (hwnd, x, y, text_size.cx, text_size.cy, TRUE);
    ShowWindow (hwnd, SW_SHOWNORMAL);
}

void DestroyToolTipWin (HWND hwnd)
{
    DestroyMainWindow (hwnd);
    MainWindowThreadCleanup (hwnd);
}

void GUIAPI DisabledTextOutEx (HDC hdc, HWND hwnd, int x, int y, const char* szText)
{
    SetBkMode (hdc, BM_TRANSPARENT);
    SetTextColor (hdc, GetWindowElementColorEx (hwnd, WE_DISABLED_ITEM));
    TextOut (hdc, x + 1, y + 1, szText);
    SetTextColor (hdc, GetWindowElementColorEx (hwnd, WE_SIGNIFICANT_ITEM));
    TextOut (hdc, x, y, szText);
}

void GUIAPI NotifyParentEx (HWND hwnd, int id, int code, DWORD add_data)
{
    NOTIFPROC notif_proc;

    if (GetWindowExStyle (hwnd) & WS_EX_NOPARENTNOTIFY)
        return;

    notif_proc = GetNotificationCallback (hwnd);

    if (notif_proc) {
        notif_proc (hwnd, id, code, add_data);
    }
    else {
        SendNotifyMessage (GetParent (hwnd), MSG_COMMAND, 
                                 (WPARAM) MAKELONG (id, code), (LPARAM)hwnd);
    }
}

int GUIAPI 
DefaultPageProc (HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    HWND hCurFocus;

    switch (message) {
    case MSG_DLG_GETDEFID:
    {
        HWND hDef;

        hDef = GetDlgDefPushButton (hWnd);
        if (hDef)
            return GetDlgCtrlID (hDef);
        return 0;
    }
    
    case MSG_DESTROY:
        DestroyAllControls (hWnd);
        break;

    case MSG_DLG_SETDEFID:
    {
        HWND hOldDef;
        HWND hNewDef;

        hNewDef = GetDlgItem (hWnd, wParam);
        if (SendMessage (hNewDef, MSG_GETDLGCODE, 0, 0L) & DLGC_PUSHBUTTON) {
            hOldDef = GetDlgDefPushButton (hWnd);
            if (hOldDef) {
                ExcludeWindowStyle (hOldDef, BS_DEFPUSHBUTTON);
                InvalidateRect (hOldDef, NULL, TRUE);
            }
            IncludeWindowStyle (hNewDef, BS_DEFPUSHBUTTON);
            InvalidateRect (hNewDef, NULL, TRUE);

            return (int)hOldDef;
        }
        break;
    }
        
    case MSG_KEYDOWN:
        if ((hCurFocus = GetFocusChild (hWnd))) {
            if (SendMessage (hCurFocus, MSG_GETDLGCODE, 0, 0L) 
                        & DLGC_WANTALLKEYS)
                break;
        }

        switch (LOWORD (wParam)) {
        case SCANCODE_KEYPADENTER:
        case SCANCODE_ENTER:
        {
            HWND hDef;

            if (SendMessage (hCurFocus, MSG_GETDLGCODE, 0, 0L) & DLGC_PUSHBUTTON)
                break;

            hDef = GetDlgDefPushButton (hWnd);
            if (hDef) {
                SendMessage (hWnd, MSG_COMMAND, GetDlgCtrlID (hDef), 0L);
                return 0;
            }
            break;
        }

        case SCANCODE_ESCAPE:
            SendMessage (hWnd, MSG_COMMAND, IDCANCEL, 0L);
            return 0;

        case SCANCODE_TAB:
        {
            HWND hNewFocus;

            if (hCurFocus) {
                if (SendMessage (hCurFocus, MSG_GETDLGCODE, 0, 0L) 
                        & DLGC_WANTTAB)
                    break;
            }

            if (lParam & KS_SHIFT)
                hNewFocus = GetNextDlgTabItem (hWnd, hCurFocus, TRUE);
            else
                hNewFocus = GetNextDlgTabItem (hWnd, hCurFocus, FALSE);

            if (hNewFocus != hCurFocus) {
                SetFocus (hNewFocus);
//                SendMessage (hWnd, MSG_DLG_SETDEFID, GetDlgCtrlID (hNewFocus), 0L);
            }

            return 0;
        }

        case SCANCODE_CURSORBLOCKDOWN:
        case SCANCODE_CURSORBLOCKRIGHT:
        case SCANCODE_CURSORBLOCKUP:
        case SCANCODE_CURSORBLOCKLEFT:
        {
            HWND hNewFocus;
                
            if (hCurFocus) {
                if (SendMessage (hCurFocus, MSG_GETDLGCODE, 0, 0L) 
                        & DLGC_WANTARROWS)
                    break;
            }

            if (LOWORD (wParam) == SCANCODE_CURSORBLOCKDOWN
                    || LOWORD (wParam) == SCANCODE_CURSORBLOCKRIGHT)
                hNewFocus = GetNextDlgGroupItem (hWnd, hCurFocus, FALSE);
            else
                hNewFocus = GetNextDlgGroupItem (hWnd, hCurFocus, TRUE);
            
            if (hNewFocus != hCurFocus) {
                if (SendMessage (hCurFocus, MSG_GETDLGCODE, 0, 0L) 
                        & DLGC_STATIC)
                    return 0;

                SetFocus (hNewFocus);
//                SendMessage (hWnd, MSG_DLG_SETDEFID, GetDlgCtrlID (hNewFocus), 0L);

                if (SendMessage (hNewFocus, MSG_GETDLGCODE, 0, 0L)
                        & DLGC_RADIOBUTTON) {
                    SendMessage (hNewFocus, BM_CLICK, 0, 0L);
                    ExcludeWindowStyle (hCurFocus, WS_TABSTOP);
                    IncludeWindowStyle (hNewFocus, WS_TABSTOP);
                }
            }

            return 0;
        }
        break;
        }
        break;
    }
    
    return DefaultControlProc (hWnd, message, wParam, lParam);
}


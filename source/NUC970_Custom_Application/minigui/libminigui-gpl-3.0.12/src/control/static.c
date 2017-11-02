/*
** $Id: static.c 13736 2011-03-04 06:57:33Z wanzheng $
**
** static.c: the Static Control module.
**
** Copyright (C) 2003 ~ 2008 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** All rights reserved by Feynman Software.
** 
** Current maitainer: Cui Wei
**
** Create date: 1999/5/22
*/

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#ifdef _MGCTRL_STATIC

#include "minigui.h"
#include "gdi.h"
#include "window.h"
#include "ctrl/ctrlhelper.h"
#include "ctrl/static.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"

#include "static_impl.h"

static int 
StaticControlProc (HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
{
    RECT        rcClient;
    HDC         hdc;
    const char* spCaption;
    PCONTROL    pCtrl;
    UINT        uFormat = DT_TOP;
    DWORD       dwStyle;         
    BOOL        needShowCaption = TRUE;
    
    pCtrl = gui_Control (hwnd); 
    switch (message) {
        case MSG_CREATE:
#ifdef __TARGET_MSTUDIO__
            SetWindowBkColor (hwnd, 
                    GetWindowElementPixel (hwnd, WE_MAINC_THREED_BODY)); 
#endif

            pCtrl->dwAddData2 = pCtrl->dwAddData;
            SetWindowBkColor (hwnd, GetWindowElementPixel (hwnd,
                                                WE_MAINC_THREED_BODY));
            /* DK[01/11/10]: For bug 4336 */
            switch (pCtrl->dwStyle & SS_TYPEMASK) {
                case SS_GRAYRECT:
                    pCtrl->iBkColor = PIXEL_lightgray;
                break;
                case SS_BLACKRECT:
                    pCtrl->iBkColor = PIXEL_black;
                break;
                case SS_WHITERECT:
                    pCtrl->iBkColor = PIXEL_lightwhite;
                break;
                default:
                break;
            }

            return 0;
            
        case STM_GETIMAGE:
            return (int)(pCtrl->dwAddData2); 
        
        case STM_SETIMAGE:
        {
            int pOldValue;
            
            pOldValue  = (int)(pCtrl->dwAddData2);
            pCtrl->dwAddData2 = (DWORD)wParam;
            InvalidateRect (hwnd, NULL, 
                    (GetWindowStyle (hwnd) & SS_TYPEMASK) == SS_ICON);
            return pOldValue;
        }
           
        case MSG_GETDLGCODE:
            return DLGC_STATIC;

        case MSG_PAINT:
            hdc = BeginPaint (hwnd);
            SelectFont (hdc, GetWindowFont(hwnd));

            GetClientRect (hwnd, &rcClient);
            dwStyle = GetWindowStyle (hwnd);
            if (dwStyle & WS_DISABLED)
                SetTextColor (hdc, GetWindowElementPixelEx (hwnd, hdc, WE_FGC_DISABLED_ITEM));
            else
                SetTextColor (hdc, GetWindowElementPixelEx (hwnd, hdc, WE_FGC_WINDOW));

			// DK[11/01/2010]: For fix bug 4336
            switch (dwStyle & SS_TYPEMASK)
            {
                case SS_SIMPLE:
                    spCaption = GetWindowCaption (hwnd);
                    if (spCaption && !(dwStyle & SS_ALIGNMASK)) {
                        SetBkMode (hdc, BM_TRANSPARENT);
                        TextOut (hdc, 0, 0, spCaption);
                        needShowCaption = FALSE;
                    }
                    break; 

                case SS_GRAYRECT:
                    SetBrushColor (hdc, PIXEL_lightgray);
                    FillBox (hdc, 0, 0, rcClient.right, rcClient.bottom);
                    break;

                case SS_BLACKRECT:
                    SetBrushColor (hdc, PIXEL_black);
                    FillBox (hdc, 0, 0, rcClient.right, rcClient.bottom);
                    break;

                case SS_WHITERECT:
                    SetBrushColor (hdc, PIXEL_lightwhite);
                    FillBox (hdc, 0, 0, rcClient.right, rcClient.bottom);
                    break;

                case SS_BLACKFRAME:
                    SetPenColor (hdc, PIXEL_black);
                    Rectangle (hdc, 0, 0, rcClient.right - 1, rcClient.bottom - 1); 
                    break;

                case SS_GRAYFRAME:
                    SetPenColor (hdc, PIXEL_lightgray);
                    Rectangle (hdc, 0, 0, rcClient.right - 1, rcClient.bottom - 1); 
                    break;

                case SS_WHITEFRAME:
                    SetPenColor (hdc, PIXEL_lightwhite);
                    Rectangle (hdc, 0, 0, rcClient.right - 1, rcClient.bottom - 1); 
                    break;

                case SS_GROUPBOX: 
                    {
                        WINDOWINFO  *info;
                        DWORD color; 

                        spCaption = GetWindowCaption (hwnd);

                        if (spCaption)
                        {
                            SIZE size;
                            RECT rect;
                            if (GetWindowExStyle (hwnd) & WS_EX_TRANSPARENT)
                                SetBkMode(hdc, BM_TRANSPARENT);
                            else
                                SetBkMode (hdc, BM_OPAQUE);

                            SetBkColor(hdc, GetWindowBkColor (hwnd));

                            TextOut (hdc, pCtrl->pLogFont->size, 2, spCaption);

                            GetTextExtent (hdc, spCaption, -1, &size);
                            rect.left = pCtrl->pLogFont->size - 1;
                            rect.right = rect.left + size.cx + 2;
                            rect.top = 0;
                            rect.bottom = size.cy;
                            ExcludeClipRect (hdc, &rect);
                            needShowCaption = FALSE;
                        }

                        info = (WINDOWINFO*)GetWindowInfo (hwnd);
                        color = GetWindowElementAttr (hwnd, WE_MAINC_THREED_BODY);
                        rcClient.top += (pCtrl->pLogFont->size >> 1); 
                        info->we_rdr->draw_3dbox (hdc, 
                                &rcClient, color, LFRDR_BTN_STATUS_NORMAL); 
                    }
                    break;

                case SS_BITMAP:
                    if (pCtrl->dwAddData2) {
                        int x = 0, y = 0, w, h;
                        PBITMAP bmp = (PBITMAP)(pCtrl->dwAddData2);

                        if (dwStyle & SS_REALSIZEIMAGE) {
                            w = bmp->bmWidth;
                            h = bmp->bmHeight;
                            if (dwStyle & SS_CENTERIMAGE) {
                                x = (rcClient.right - w) >> 1;
                                y = (rcClient.bottom - h) >> 1;
                            }
                        }
                        else {
                            x = y = 0;
                            w = RECTW (rcClient);
                            h = RECTH (rcClient);
                        }

                        FillBoxWithBitmap(hdc, x, y, w, h, bmp);
                        needShowCaption = FALSE;
                    }
                    break;

                case SS_ICON:
                    if (pCtrl->dwAddData2) {
                        int x = 0, y = 0, w, h;
                        HICON hIcon = (HICON)(pCtrl->dwAddData2);

                        if (dwStyle & SS_REALSIZEIMAGE) {
                            GetIconSize (hIcon, &w, &h);
                            if (dwStyle & SS_CENTERIMAGE) {
                                x = (rcClient.right - w) >> 1;
                                y = (rcClient.bottom - h) >> 1;
                            }
                        }
                        else {
                            x = y = 0;
                            w = RECTW (rcClient);
                            h = RECTH (rcClient);
                        }

                        DrawIcon (hdc, x, y, w, h, hIcon);
                        needShowCaption = FALSE;
                    }
                    break;

                default:
                    break;
            }

            if (needShowCaption) {
                uFormat = DT_TOP;
                switch (dwStyle & SS_ALIGNMASK) {
                    case SS_LEFT:
                        uFormat |= DT_LEFT;
                        break;
                    case SS_CENTER:
                        uFormat |= DT_CENTER;
                        break;
                    case SS_RIGHT:
                        uFormat |= DT_RIGHT;
                        break;
                }

                if (dwStyle & SS_LEFTNOWORDWRAP) {
                    uFormat |= DT_SINGLELINE | DT_EXPANDTABS;
                }
                else {
                    uFormat |= DT_WORDBREAK;
                }

                if (dwStyle & SS_NOPREFIX)
                    uFormat |= DT_NOPREFIX;

                spCaption = GetWindowCaption (hwnd);

                if (spCaption) {
                    SetBkMode (hdc, BM_TRANSPARENT);
                    DrawText (hdc, spCaption, -1, &rcClient, uFormat);
                }
            }

            EndPaint (hwnd, hdc);
            return 0;

        case MSG_LBUTTONDBLCLK:
            if (GetWindowStyle (hwnd) & SS_NOTIFY)
                NotifyParent (hwnd, pCtrl->id, STN_DBLCLK);
            break;

        case MSG_LBUTTONDOWN:
            if (GetWindowStyle (hwnd) & SS_NOTIFY)
                NotifyParent (hwnd, pCtrl->id, STN_CLICKED);
            break;

        case MSG_NCLBUTTONDBLCLK:
            break;

        case MSG_NCLBUTTONDOWN:
            break;

        case MSG_HITTEST:
            dwStyle = GetWindowStyle (hwnd);
            if ((dwStyle & SS_TYPEMASK) == SS_GROUPBOX)
                return HT_TRANSPARENT;

            if (GetWindowStyle (hwnd) & SS_NOTIFY)
                return HT_CLIENT;
            else
                return HT_OUT;
        break;

        case MSG_FONTCHANGED:
            InvalidateRect (hwnd, NULL, TRUE);
            return 0;
            
        case MSG_SETTEXT:
            SetWindowCaption (hwnd, (char*)lParam);
            InvalidateRect (hwnd, NULL, TRUE);
            return 0;
            
        default:
            break;
    }

    return DefaultControlProc (hwnd, message, wParam, lParam);
}

BOOL RegisterStaticControl (void)
{
    WNDCLASS WndClass;

    WndClass.spClassName = CTRL_STATIC;
    WndClass.dwStyle     = WS_NONE;
    WndClass.dwExStyle   = WS_EX_NONE;
    WndClass.hCursor     = GetSystemCursor (IDC_ARROW);
    WndClass.iBkColor    = GetWindowElementPixel (HWND_NULL,WE_MAINC_THREED_BODY);
    WndClass.WinProc     = StaticControlProc;

    return AddNewControlClass (&WndClass) == ERR_OK;
}

#endif /* _MGCTRL_STATIC */


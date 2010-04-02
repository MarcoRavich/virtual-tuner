/*  Copyright 2010 Stefan Elmlund

    This file is part of Virtual Tuner.

    Virtual Tuner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Virtual Tuner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Virtual Tuner.  If not, see <http://www.gnu.org/licenses/>.
*/

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "graph.h"
#include "freqmeterctrl.h"

// data for the frequency meter control
typedef struct
{
    double targetFreq;
    double currentFreq;
	// note string
    char note;
    char number;
    char sharp;
    void *notePtr;
    // detecting flag
    BOOL detecting;
    BOOL hideNeedle;
    // cumalated frequnecy and number of samples.
    double cumlFreq;
    size_t samples;
    // font used
    HFONT freqFont;
    HFONT noteFont;
    HFONT numFont;
    // back buffer
    HDC memhdc;
    HBITMAP bmp;
    HBITMAP oldbmp;
    uint32_t *backbuf;

} FreqMeterCtrlData;

LRESULT CALLBACK FreqMeterCtrlProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch( msg ) {
    // first message called to the control
    case WM_NCCREATE: {
        // allocate memory for the controls data
        FreqMeterCtrlData *data = HeapAlloc(GetProcessHeap(), 0, sizeof(FreqMeterCtrlData));
        // set default data
        data->targetFreq = 0.0;
        data->currentFreq = 0.0;
        data->note = '\0';
        data->number = '\0';
        data->sharp = '\0';
        data->notePtr = NULL;
        data->backbuf = NULL;
        data->detecting = FALSE;
        data->hideNeedle = TRUE;
        data->freqFont = CreateFont(14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Arial");
        data->noteFont = CreateFont(27,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Arial");
        data->numFont = CreateFont(15,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Arial");
        // set the data at the window extra
        SetWindowLong(hwnd, 0, (LONG)data);
        } break;
    // last message called
    case WM_NCDESTROY: {
        // get the control data
        FreqMeterCtrlData *data = (FreqMeterCtrlData *)GetWindowLong(hwnd, 0);
        // delete the fonts
        DeleteObject(data->noteFont);
        DeleteObject(data->freqFont);
        DeleteObject(data->numFont);
        // restore the old dc
        if(data->backbuf != NULL) {
            SelectObject(data->memhdc, data->oldbmp);
        }
        // free memory for the control data
        HeapFree(GetProcessHeap(), 0, data);
        } break;
    // called after the window have been displayed (or resized)
    case WM_SIZE:
        {
            // get the control data
            FreqMeterCtrlData *data = (FreqMeterCtrlData *)GetWindowLong(hwnd, 0);
            // check so a backbuffer is not already attached to the control
            if( data->backbuf == NULL )
            {
                // get the controls size
                HDC hdc = GetDC(hwnd);
                RECT rect;
                GetClientRect(hwnd, &rect);
                // bitmap for the back buffer
                BITMAPV4HEADER bi;
                ZeroMemory(&bi,sizeof(BITMAPV4HEADER));
                bi.bV4Size         = sizeof(BITMAPV4HEADER);
                bi.bV4Width        = rect.right - rect.left;
                bi.bV4Height       = -(rect.bottom - rect.top);
                bi.bV4Planes       = 1;
                bi.bV4BitCount     = 32;
                bi.bV4V4Compression  = BI_BITFIELDS;
                bi.bV4RedMask      = 0x00FF0000;
                bi.bV4GreenMask    = 0x0000FF00;
                bi.bV4BlueMask     = 0x000000FF;
                bi.bV4AlphaMask    = 0xFF000000;
                // create the a device context for the back buffer
                data->memhdc = CreateCompatibleDC(hdc);
                // create the back buffer
                data->bmp = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&data->backbuf, NULL, 0);
                // select the back buffer as to draw on
                data->oldbmp = (HBITMAP)SelectObject(data->memhdc, data->bmp);
                ReleaseDC(hwnd, hdc);
            }
        } break;
    // called when the control needs to redraw
    case WM_PAINT: 
        {
            // get the controls data
            FreqMeterCtrlData *data = (FreqMeterCtrlData *)GetWindowLong(hwnd, 0);
            HDC hdc = data->memhdc;

            UINT format = DT_SINGLELINE|DT_CENTER|DT_VCENTER;

            // draw the label text
            RECT rect;
            GetClientRect(hwnd, &rect);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_3DFACE));

            // draw a frame
            FrameRect(hdc,&rect, GetStockObject(BLACK_BRUSH));

            // set the color for the line
            rgb_t color;
            color.r = 150;
            color.g = 150; 
            color.b = 150;
            color.a = 0;

            // draw the arc, center x=131; y=102;
            for(double r=(-3.14159265/2 - 0.6); r < (-3.14159265/2 + 0.55); r+=0.1)
            {
                int x1 = 131 - 180 * cos(r); 
                int y1 = 210 + 180 * sin(r);
                int x2 = 131 - 180 * cos(r+0.1);
                int y2 = 210 + 180 * sin(r+0.1);
                WuLine(hwnd, data->backbuf, x1, y1, x2, y2, color);
            }
            // draw the seperator line marking
            for(double r=(-3.14159265/2 - 0.6); r < (-3.14159265/2 + 0.65); r+=0.1)
            {
                int x1 = 131 - 175 * cos(r); 
                int y1 = 210 + 175 * sin(r);
                int x2 = 131 - 185 * cos(r);
                int y2 = 210 + 185 * sin(r);
                WuLine(hwnd, data->backbuf, x1, y1, x2, y2, color);
            }

            // draw the target text
            // select the label font
            HFONT oldfont = SelectObject(hdc, data->freqFont);

            // draw the target text
            char buf[128];
            if( data->hideNeedle == FALSE) {
                SetTextColor(hdc, RGB(0,0,0));
                if(data->detecting == TRUE) {
                    SetBkColor(hdc, RGB(100,255,100));
                }
                else {
                    SetBkColor(hdc, RGB(255,100,100));
                }
                SetBkMode(hdc, OPAQUE);
                rect.left = 112; rect.right = 150; rect.top = 8; rect.bottom = 22;

                sprintf(buf, "%0.1f", data->targetFreq);
                DrawText(hdc, buf, -1, &rect, format);
            }

            SetTextColor(hdc, RGB(100,100,100));
            SetBkMode(hdc, TRANSPARENT);
            // draw "-10" text
            rect.left = 82; rect.right = 105; rect.top = 14; rect.bottom = 27;
            DrawText(hdc, "-10", -1, &rect, format);
            // draw "-20" text
            rect.left = 47; rect.right = 69; rect.top = 25; rect.bottom = 38;
            DrawText(hdc, "-20", -1, &rect, format);
            // draw "-30" text
            rect.left = 14; rect.right = 34; rect.top = 43; rect.bottom = 56;
            DrawText(hdc, "-30", -1, &rect, format);

            // draw "10" text
            rect.left = 157; rect.right = 179; rect.top = 14; rect.bottom = 27;
            DrawText(hdc, "+10", -1, &rect, format);
            // draw "20" text
            rect.left = 193; rect.right = 215; rect.top = 25; rect.bottom = 38;
            DrawText(hdc, "+20", -1, &rect, format);
            // draw "30" text
            rect.left = 228; rect.right = 250; rect.top = 43; rect.bottom = 56;
            DrawText(hdc, "+30", -1, &rect, format);

            // draw the note char
            SetTextColor(hdc, RGB(0,0,0));
            SelectObject(hdc, data->noteFont);
            rect.left = 117; rect.top = 121; rect.right = 138; rect.bottom = 144;
            buf[0] = data->note; buf[1] = '\0';
            DrawText(hdc, buf, -1, &rect, format);

            // draw sharp sign
            SelectObject(hdc, data->numFont);
            rect.left = 136; rect.top = 116; rect.right = 148; rect.bottom = 131;
            buf[0] = data->sharp; buf[1] = '\0';
            DrawText(hdc, buf, -1, &rect, format);

            // draw the note number
            rect.left = 136; rect.top = 134; rect.right = 148; rect.bottom = 148;
            buf[0] = data->number; buf[1] = '\0';
            DrawText(hdc, buf, -1, &rect, format);

            // restore the old font
            SelectObject(hdc, oldfont);

            if(data->hideNeedle == FALSE) {
                // draw the needle
                // needle color
                color.r = 50;
                color.g = 50; 
                color.b = 50;
                color.a = 0;
                // calculat the needle start and stop point
                double d = (data->targetFreq - data->currentFreq)/50.0;
                double r = -3.14159265/2 + d;
                int x1 = 131 - 100 * cos(r); 
                int y1 = 210 + 100 * sin(r);
                int x2 = 131 - 182 * cos(r);
                int y2 = 210 + 182 * sin(r);
                WuLine(hwnd, data->backbuf, x1, y1, x2, y2, color);
            }

            // copy the back buffer to the front buffer
            PAINTSTRUCT ps;
            GetClientRect(hwnd, &rect);
            HDC frontHdc = BeginPaint(hwnd, &ps);
            BitBlt(frontHdc, 0, 0, rect.right, rect.bottom, hdc, 0, 0, SRCCOPY);
            EndPaint(hwnd, &ps);
        } break;

    // do not draw the background
    case WM_ERASEBKGND:
        break;

    // update the frequency target data
    case FMM_UPDATE:
        {
            // get the pointer to the data
            FreqMeterCtrlData *data = (FreqMeterCtrlData *)GetWindowLong(hwnd, 0);
            // get the message structure
            FreqMeterUpdateMsg *msg = (FreqMeterUpdateMsg *)lparam;
            // have the target note been changed seans last frequency?
            if( data->notePtr == msg->notePtr )
            {
                // add the sample in the message to the cumalated freq.
                if(msg->currentFreq > (msg->targetFreq-25.0) &&
                   msg->currentFreq < (msg->targetFreq+25.0))
                {
                    data->cumlFreq += msg->currentFreq;
                    data->samples++;
                }

                // update the control if ther are more then 3 samples
                if( data->samples >= 3) {
                    data->targetFreq = msg->targetFreq;
                    data->currentFreq = data->cumlFreq / (double)data->samples;
                    data->detecting = TRUE;
                    data->hideNeedle = FALSE;
                    data->note = msg->note;
                    data->number = msg->number;
                    data->sharp = msg->sharp;
                    // redraw the control
                    InvalidateRect(hwnd, NULL, FALSE);
                    // message have been handled
                    return TRUE;
                }
            }
            else
            {
                // redraw the control if not detecting is set the TRUE
                if( data->detecting == TRUE ) {
                    data->detecting = FALSE;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                // set the first sample
                data->samples = 1;
                data->cumlFreq = msg->currentFreq;
                data->notePtr = msg->notePtr;
            }
            // case stament FFM_UPDATE end
        } break;
    // message not handled by this procedure, call the default procedure.
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    // message have been handle by this procedure
    return TRUE;
}

void InitFreqMeterCtrl(void)
{
    // register the control class
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpszClassName = "FREQ_METER";
    wc.hInstance = GetModuleHandle(0);
    wc.lpfnWndProc = FreqMeterCtrlProc;
    wc.hCursor = LoadCursor (NULL, IDC_ARROW);
    wc.hIcon = 0;
    wc.lpszMenuName = 0;
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
    wc.style = 0;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(FreqMeterCtrlData *);
    wc.hIconSm = 0;
    RegisterClassEx(&wc);
}

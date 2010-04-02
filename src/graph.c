/*	Copyright 2010 Stefan Elmlund

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

#include "graph.h"

#define swap(a,b) { int tmp; tmp=a; a=b; b=tmp; }

// draw the wu line (anialized line)
void WuLine(HWND hwnd, uint32_t *buf, int x1, int y1, int x2, int y2, rgb_t color)
{
	RECT rect;
	GetClientRect(hwnd, &rect);

    uint16_t error_adj, error_acc, error_acc_tmp;
    int32_t dx, dy, xstep;
    rgb_t backcol; // background color
    rgb_t rescol; // result color
    uint16_t w; // weight
    int32_t width = (rect.right - rect.left) - 1;
    int32_t height = (rect.bottom - rect.top)- 1;
    rgb_t *offset;
    uint8_t code1;
    uint8_t code2;
    uint8_t done=0;
    int32_t x;
    int32_t y;
    uint32_t y_pitch = rect.right - rect.left;

    // clip the line using Cohen-Sutherland algorithm
    // do not clip line with zero length
    if(x1 == x2 && y1 == y2) {
        return;
    }

    do {
        code1 = code2 = 0;
        // find clip code for point 1
        if      (y1 < 0)      code1 |= 1; // bottom;
        else if (y1 > height) code1 |= 2; // top
        if      (x1 < 0)      code1 |= 4; // left
        else if (x1 > width)  code1 |= 8; // right

        // find clip code for point 2
        if      (y2 < 0)      code2 |= 1; // bottom
        else if (y2 > height) code2 |= 2; // top
        if      (x2 < 0)      code2 |= 4; // left
        else if (x2 > width)  code2 |= 8; // right

		// line not visable
        if((code1 & code2) != 0) {
            return;
        }

        if((code1 | code2) == 0) {
            // line within the client area
            done = 1;
		}
        else {
            // line need clipping
            if(code1 != 0) {
                if(code1 & 1) {
                    // clip against the bottom
                    x = x2 - (y2 * (x1 - x2)) / (y1 - y2);
                    y = 0;
                }
                else if(code1 & 2) {
                    // clip against the top
                    x = x2 + ((x1 - x2) * (height - y2)) / (y1 - y2);
                    y = height;
                }
                if(code1 & 4) {
                    // clip against the left
                    y = y2 - (x2 * (y1 - y2)) / (x1 - x2);
                    x = 0;
                }
                else if(code1 & 8) {
                    // clip against the right
                    y = y2 + ((y1 - y2) * (width - x2)) / (x1 - x2);
                    x = width;
                }
                x1 = x;
                y1 = y;
            }
            else {
                if(code2 & 1) {
                    // clip against the bottom
                    x = x1 - (y1 * (x2 - x1)) / (y2 - y1);
                    y = 0;
                }
                else if(code2 & 2) {
                    // clip against the top
                    x = x1 + ((x2 - x1) * (height - y1)) / (y2 - y1);
                    y = height;
                }
                if(code2 & 4) {
                    // clip against the left
                    y = y1 - (x1 * (y2 - y1)) / (x2 - x1);
                    x = 0;
                }
                else if(code2 & 8) {
                    // clip against the right
                    y = y1 + ((y2 - y1) * (width - x1)) / (x2 - x1);
                    x = width;
                }
                x2 = x;
                y2 = y;
            }
        }
    } while(done != 1);

    // Draw an antialiasing line using Xiaolin Wu's line algotithm
    // do not draw line with zero length
    if(x1 == x2 && y1 == y2) {
        return;
    }

    // draw initial and final pixel
    buf[y1 * y_pitch + x1] = *((uint32_t *)&color);
    buf[y2 * y_pitch + x2] = *((uint32_t *)&color);

    // make sure the line runs from top to bottom
    if(y1 > y2) {
        swap(x1, x2);
        swap(y1, y2);
    }

    if((dx = x2 - x1) >= 0) {
        xstep = 1;
    }
    else {
        xstep = -1;
        dx = -dx; // make delta x positive
    }

    // special cases horizontal, veritcal and diagonal lines
    if((dy = y2 - y1) == 0) {
        // horizontal line
        offset = (rgb_t *)(buf + y1 * y_pitch + x1);
        while(dx--) {
            *offset = color;
            offset += xstep;
        }
        return;
    }
    if(dx == 0) {
        // vertical line
        offset = (rgb_t *)(buf + y1 * y_pitch + x1);
        do {
            *offset = color;
            offset += y_pitch;
        } while(--dy != 0);
        return;
    }

    if(dx == dy) {
        // diagonal line
        offset = (rgb_t *)(buf + y1 * y_pitch + x1);
        xstep = y_pitch + xstep;
        do {
            *offset = color;
            offset += xstep;
        } while(--dy != 0);
        return;
    }

    error_acc = 0;

    if(dy > dx) {
        // "verticalis" line
        error_adj = ((uint32_t)dx << 16) / (uint32_t)dy;
       
        while(--dy) {
            // calculate the new y and x for the pixel
            error_acc_tmp = error_acc;
            error_acc += error_adj;
            if(error_acc <= error_acc_tmp) {
                x1 += xstep;
            }
            y1++;
           
            // draw the "right" pixel
            w = error_acc >> 8; // calculate the weight
            offset = (rgb_t *)(buf + y1 * y_pitch + x1 + xstep);
            // alpha blend line color with its weight and the
            // background color
            backcol = *offset;
            rescol.r = backcol.r + ((w * (color.r - backcol.r)) >> 8);
            rescol.g = backcol.g + ((w * (color.g - backcol.g)) >> 8);
            rescol.b = backcol.b + ((w * (color.b - backcol.b)) >> 8);
            // draw the pixel to the background buffer
            *offset = rescol;

            // draw the "left" pixel
            w = w ^ 255; // invert the weigth
            offset -= xstep;
            // alpha blend the color with its inverted weight and the
            // background color
            backcol = *offset;
            rescol.r = backcol.r + ((w * (color.r - backcol.r)) >> 8);
            rescol.g = backcol.g + ((w * (color.g - backcol.g)) >> 8);
            rescol.b = backcol.b + ((w * (color.b - backcol.b)) >> 8);
            // draw the pixel to the background buffer
            *offset = rescol;
        }
        return;
    }

    // "horizontic" line
    error_adj = ((uint32_t)dy << 16) / (uint32_t)dx;
   
    while(--dx) {
        // calculate the new x and y for the pixel
        error_acc_tmp = error_acc;
        error_acc += error_adj;
        if(error_acc <= error_acc_tmp) {
            y1++;
        }
        x1 += xstep;

        // draw the "upper" pixel
        w = error_acc >> 8; // calculate the weight
        offset = (rgb_t *)(buf + (y1+1) * y_pitch + x1);
        // alpha blend "line" color with its weight and the
        // background color
        backcol = *offset;
        rescol.r = backcol.r + ((w * (color.r - backcol.r)) >> 8);
        rescol.g = backcol.g + ((w * (color.g - backcol.g)) >> 8);
        rescol.b = backcol.b + ((w * (color.b - backcol.b)) >> 8);
        // draw the pixel to the background buffer
        *offset = rescol;


        // draw the "lower" pixel
        w = w ^ 255; // invert the weigth
        offset -= y_pitch;
        // alpha blend the line color with its inverted weight and the
        // background color
        backcol = *offset;
        rescol.r = backcol.r + ((w * (color.r - backcol.r)) >> 8);
        rescol.g = backcol.g + ((w * (color.g - backcol.g)) >> 8);
        rescol.b = backcol.b + ((w * (color.b - backcol.b)) >> 8);
        // draw the pixel to the background buffer'
        *offset = rescol;
    }
}

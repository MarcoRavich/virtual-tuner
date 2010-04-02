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

#ifndef __GRAPH_H__
#define __GRAPH_H__

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include <stdint.h>

typedef struct
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} rgb_t;

/** draw wu line on a background window
  * param hwnd: handler to window
  * param buf: pointer to a background buffer
  * param x1: start coordinate
  * param y1: start coordinate
  * param x2: end coordinate
  * param y2: end coordinate
  * param color: color to draw the line
**/
void WuLine(HWND hwnd, uint32_t *buf, int x1, int y1, int x2, int y2, rgb_t color);

#endif

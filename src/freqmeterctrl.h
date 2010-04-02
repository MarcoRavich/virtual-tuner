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

#ifndef __FREQ_METER_CTRL_H__
#define __FREQ_METER_CTRL_H__

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

// message for setting the target frequency
// lParam: pointer to a update structure
#define FMM_UPDATE  (WM_USER + 0)

typedef struct
{
    double targetFreq;
    double currentFreq;
    char note;
    char number;
    char sharp;
    void *notePtr;
} FreqMeterUpdateMsg;

// Initialize and register the freq meter control
void InitFreqMeterCtrl(void);

#endif

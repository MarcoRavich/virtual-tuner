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

#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "wavein.h"
#include "detfreq.h"

WaveIn *WaveInOpen(HWND hwnd, size_t n)
{
    WaveIn *wavein = malloc(sizeof(WaveIn));
    MMRESULT res;

    // open the wavein
    wavein->format.wFormatTag = WAVE_FORMAT_PCM;
    wavein->format.nChannels = 1;
    wavein->format.nSamplesPerSec = WAVEIN_SAMPLE_RATE;
    wavein->format.nAvgBytesPerSec = WAVEIN_SAMPLE_RATE * 2;
    wavein->format.nBlockAlign = 2;
    wavein->format.wBitsPerSample = 16;
    wavein->format.cbSize = 0;
    wavein->n = n;

    res = waveInOpen(&wavein->handler, WAVE_MAPPER, &wavein->format, (DWORD)hwnd, 0, CALLBACK_WINDOW);  
 
    if(res) {
        return NULL;
    }

    return wavein;
}

void WaveInStart(WaveIn *waveIn, BOOL prep)
{
    MMRESULT res;

    res = waveInReset(waveIn->handler);
    if(res) {
        return;
    }

    // prepare header an add the buffer
    waveIn->header.lpData = (char *) waveIn->data;
    waveIn->header.dwBufferLength = waveIn->n * 2;
    waveIn->header.dwBytesRecorded = 0;
    waveIn->header.dwFlags = 0L;
    waveIn->header.dwLoops = 0L;
    waveIn->header.dwUser = 0L;
    waveInPrepareHeader(waveIn->handler, &waveIn->header, sizeof(WAVEHDR));

    res = waveInAddBuffer(waveIn->handler, &waveIn->header, sizeof(WAVEHDR));
    if(res) {
        return;
    }

    res = waveInStart(waveIn->handler);
    if(res) {
        return;
    }
}

void WaveInClose(WaveIn *waveIn)
{
    waveInUnprepareHeader(waveIn->handler, &waveIn->header, sizeof(WAVEHDR));
    waveInClose(waveIn->handler);
    free(waveIn);	
}

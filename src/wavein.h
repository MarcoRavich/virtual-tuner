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

#ifndef __WAVEIN_H__
#define __WAVEIN_H__

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include <stdint.h>

#define WAVEIN_ABS(a) (((a) < 0) ? -(a) : (a))

#define WAVEIN_SAMPLE_RATE         11025
#define WAVEIN_SAMPLE_SIZE           512
#define WAVEIN_BITS_PER_SAMPLE        16

typedef struct
{
    HWAVEIN handler;
    WAVEFORMATEX format;
    WAVEHDR header;
    int16_t data[WAVEIN_SAMPLE_SIZE];
    size_t n; // number of samples
} WaveIn;

/*! Open a wave in channel
    @param: number of sample to record
    @return: pointer to a structre containg the wavein information
*/
WaveIn *WaveInOpen(HWND hwnd, size_t N);

/*! Start recording from a open channel
    @param: pointer to a wavein structure
*/
void WaveInStart(WaveIn *waveIn, BOOL);

/*! Return the current wave buffer state
    @return: value != 0 if the buffer is full else 0
*/
inline
size_t WaveInBufferFull(WaveIn *waveIn)
{
    return ((waveIn->header.dwFlags & WHDR_DONE) == WHDR_DONE);
}

/*! Close the wave in channel, called this when the program ends
    @param: pointer to an open wavein structure
*/
void WaveInClose(WaveIn *waveIn);

#endif

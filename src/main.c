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

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include "wavein.h"
#include "detfreq.h"

#include "freqmeterctrl.h"

#include "../res/resource.h"

WaveIn *waveIn;

BOOL CALLBACK MainDlgProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch(message) {
	// user have pressed the 'X' button to close the program 
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	// new data in the in the wave input
	case WIM_DATA:
		// check if the wave in buffer is full
		if( waveIn->header.dwFlags & WHDR_DONE ) {
			waveInStop(waveIn->handler);
			// normalize the signal and copy the buffer to signal buffer
		    double signal[WAVEIN_SAMPLE_SIZE];
			size_t bytsRecs = waveIn->header.dwBytesRecorded >> 2;
			WaveInNormalize(waveIn, signal, bytsRecs);
			
			// there is now a copy of the orignal signal so it can be restarted			// restart the wave in
			WaveInStart(waveIn, TRUE);
			WaveInStart(waveIn, TRUE);

			// apply hamming window to the signal
			HammingWindow(signal, WAVEIN_SAMPLE_SIZE);
			// find the closest note frequency
			NoteTblNode *target = FindClosestFrequency(signal, bytsRecs);
			// detect the frequency from the signal
			double detFreq = DetectFrequency(signal, bytsRecs, target->freq);
			// update the frequency meter
			FreqMeterUpdateMsg msg;
			msg.targetFreq = target->freq;
			msg.currentFreq = detFreq;
			msg.note = target->note;
			msg.number = target->num;
			msg.sharp = target->sharp;
			msg.notePtr = (void *)(target);
			SendMessage(GetDlgItem(hwnd, ID_FREQMETER), FMM_UPDATE, 0, (LPARAM)&msg);
		}
		break;
		// message not handled by this function
        default:
            return FALSE;
    }
	// message have been handled by this function
    return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// initialize the frequent control
	InitFreqMeterCtrl();

	// create the dialog window
	HWND hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(ID_MAINDIALOG), NULL, MainDlgProc);

	// set the icon for the dialog
	HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_ICON));
	SendMessage(hwnd, WM_SETICON, ICON_BIG, (WPARAM)icon);

	// opend wave in for recoding of sound
	waveIn = WaveInOpen(hwnd, WAVEIN_SAMPLE_SIZE);
	if(waveIn == NULL)
	{
		MessageBox(hwnd, "Failed to open wavein (do you have a soundcard?)", "Error", MB_OK);
		exit(-1);
	}

	// start the recording
	WaveInStart(waveIn, TRUE);
	
	// display the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

	// handle incomming message for the dialog until the user quits the program
	MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
		if( !IsDialogMessage(hwnd, &msg) )
		{
			TranslateMessage(&msg);
        		DispatchMessage(&msg);
		}
    }

	WaveInClose(waveIn);

    return msg.wParam;
}

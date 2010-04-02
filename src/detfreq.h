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

#ifndef __DETFREQ_H__
#define __DETFREQ_H__

#define _USE_MATH_DEFINES
#include <math.h>

#include "wavein.h"
#include "notetbl.h"

/*! Invoke hamming window on the given signal
	@param: pointer to a normalized signal
	@param: number of sampled in the signal
*/
inline
void HammingWindow(double *signal, size_t n) {
	for(int i=0; i<n; i++) {
		signal[i] = signal[i] * (0.54 - 0.46 * cos(2*M_PI*i/n));
	}
}

/*! Normalize the interger value
    @param: pointer to the wavein structure
	@param: reslut normalized signal
	@param: size of the result signal

	@note: result buffer needs to be >= waveIn->n!
*/
inline
void WaveInNormalize(WaveIn *waveIn, double *result, size_t n) {
	// do the normalization
	int16_t *data = (int16_t *)waveIn->data;
	for(size_t i=0; i<n; i++)
	{
		result[i] = data[i] / 32768.0;
	}
}

/*! Invoke the Goerzel filter on the given signal
	@param: pointer to a normalized signal
	@param: number of sampled in the signal
	@param: target frequency to detect
	@return: nearest value

	@note: nearest value should be as high as possible
*/
double GoertzelFilter(double *signal, size_t n, double targetFreq);

/*! Find the frequent that is closes to the input signal
    @param signal: pointer to input signal
    @param n: number of sample in the signal
    @return: node to the frequency node from the table or
             NULL if no signal was found
*/
NoteTblNode *FindClosestFrequency(double *signal, size_t n);

/*! Detect the frequency around the target frequency
    @param: pointer to the normalize signal
    @param: number of samples in the signal
    @param: target frequency
    @param: detection limit for before return a frequency
    @return: detected frequency
*/
double DetectFrequency(double *signal, size_t n, double targetFreq);

#endif

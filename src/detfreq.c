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

#include "wavein.h"
#include "detfreq.h"
#include "notetbl.h"

extern NoteTblNode NoteTbl[];

double GoertzelFilter(double *signal, size_t n, double targetFreq)
{
    const double k = (n * targetFreq)/WAVEIN_SAMPLE_RATE;
    const double coeff = 2.0*cos((2.0*M_PI*k)/n);
    double q1 = 0;
    double q2 = 0;

    for(size_t i=0; i<n; i++)
    {
        double q0 = signal[i] + coeff*q1 - q2;
        q2 = q1;
        q1 = q0;
    }

    double real = 0.5*coeff*q1 - q2;
    double imag = sin((2.0*M_PI*k)/n)*q1;

    return sqrt(real*real + imag*imag);
}

NoteTblNode *FindClosestFrequency(double *signal, size_t n)
{
    // pointer to the node with the bigest value
    NoteTblNode *node = &NoteTbl[0];
    double max = GoertzelFilter(signal, n, node->freq);

    // iterate the note table to find the closes frequency for the node
    NoteTblNode *itr = &NoteTbl[1];
    while(itr->note != '\0')
    {
        double ret = GoertzelFilter(signal, n, itr->freq);
        if( ret > max )
        {
            max = ret;
            node = itr;
        }
        itr++;
    }
    // return the pointer to the node with the closes frequency
    return node;
}

double DetectFrequency(double *signal, size_t n, double targetFreq)
{
    double sum = 0;

    // scan the frequency around target frequency +-30.0 to detect the frequency with the
    // greatest goertzel filter value
    double farDetFreq = targetFreq-27.5;
    double max = GoertzelFilter(signal, n, farDetFreq);

    for(double freq=(targetFreq-25.0); freq<(targetFreq+28.0);)
    {
        double ret = GoertzelFilter(signal, n, freq);
        if(ret > max) {
            max = ret;
            farDetFreq = freq;
        }
        freq += 2.5;
    }
    // close scan the frequency
    double nearDetFreq = farDetFreq - 2.5;
    max = GoertzelFilter(signal, n, nearDetFreq);
    for(double freq = (farDetFreq - 2.4); freq < (farDetFreq + 2.6); )
    {
        double ret = GoertzelFilter(signal, n, freq);
        if(ret > max) {
            max = ret;
            nearDetFreq = freq;
        }
        freq += 0.1;
    }
    // return the frequency with the closes match
    return nearDetFreq;
}

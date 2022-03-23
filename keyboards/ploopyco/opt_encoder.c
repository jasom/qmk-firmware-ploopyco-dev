/* Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 * Copyright 2020 Ploopy Corporation
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "opt_encoder.h"

#define SCROLL_WHEEL_HYSTERESIS 2

enum State state;

/* Variables used for scroll wheel functionality. */
static int accum;
int  lowA;
int  highA;
bool cLowA;
bool cHighA;
int  lowIndexA;
int  highIndexA;
bool lowOverflowA;
bool highOverflowA;
int  lowB;
int  highB;
bool cLowB;
bool cHighB;
int  lowIndexB;
int  highIndexB;
bool lowOverflowB;
bool highOverflowB;
int  scrollThresholdA;
int  scrollThresholdB;
int  arLowA[SCROLLER_AR_SIZE];
int  arHighA[SCROLLER_AR_SIZE];
int  arLowB[SCROLLER_AR_SIZE];
int  arHighB[SCROLLER_AR_SIZE];

/* Setup function for the scroll wheel. Initializes
   the relevant variables. */
void opt_encoder_init(void) {
    accum            = 0;
    state            = HIHI;
    lowA             = 1023;
    highA            = 0;
    cLowA            = false;
    cHighA           = false;
    lowIndexA        = 0;
    highIndexA       = 0;
    lowOverflowA     = false;
    highOverflowA    = false;
    lowB             = 1023;
    highB            = 0;
    cLowB            = false;
    cHighB           = false;
    lowIndexB        = 0;
    highIndexB       = 0;
    lowOverflowB     = false;
    highOverflowB    = false;
    scrollThresholdA = 0;
    scrollThresholdB = 0;
}

/******************************************************************
 * How this works: there are 4 encoder states:
 *
 *  +---------------HIHI------------+
 * HILO                            LOHI
 *  +---------------LOLO------------+
 *
 *  Travelling around the circle clockwise indicates a positive value,
 *  anti-clockwise indicates a negative value.
 *
 *  Sometimes we miss a state though, e.g. HILO to LOHI.  This gives us no
 *  information about the direction of travel, but is otherwise benign.  The
 *  scroll-wheel (lacking detents) can sit right between two states as well,
 *  causing "fluttering"
 *
 *  So, when we detect a clockwise transition, we increment an accumulator,
 *  when we detect an anti-clockwise transition, we decrement.  Only when the
 *  accumulator hits a threshold (SCROLL_WHEEL_HYSTERESIS), do we signal a
 *  scroll event.
 */

int opt_encoder_handler(int curA, int curB) {
    if (lowOverflowA == false || highOverflowA == false) calculateThresholdA(curA);
    if (lowOverflowB == false || highOverflowB == false) calculateThresholdB(curB);

    bool LO = false;
    bool HI = true;
    bool sA, sB;

    if (curA < scrollThresholdA)
        sA = LO;
    else
        sA = HI;

    if (curB < scrollThresholdB)
        sB = LO;
    else
        sB = HI;

    enum State newState;

    if (sA == LO) {
        if (sB == LO)
            newState = LOLO;
        else
            newState = LOHI;
    } else {
        if (sB == LO)
            newState = HILO;
        else
            newState = HIHI;
    }
    switch (state) {
        case HIHI:
            if (newState ==  LOHI) accum++;
            if (newState == HILO) accum--;
            break;
        case LOHI:
            if (newState == LOLO) accum++;
            if (newState == HIHI) accum--;
            break;
        case LOLO:
            if (newState == HILO) accum++;
            if (newState == LOHI) accum--;
            break;
        case HILO:
            if (newState == HIHI) accum++;
            if (newState == LOLO) accum--;
            break;
    }
    state = newState;
    if (accum >= SCROLL_WHEEL_HYSTERESIS) {
        accum=0;
        return 1;
    }
    if (-accum >= SCROLL_WHEEL_HYSTERESIS) {
        accum=0;
        return -1;
    }
    return 0;


}

void calculateThresholdA(int curA) { scrollThresholdA = calculateThreshold(curA, &lowA, &highA, &cLowA, &cHighA, arLowA, arHighA, &lowIndexA, &highIndexA, &lowOverflowA, &highOverflowA); }

void calculateThresholdB(int curB) { scrollThresholdB = calculateThreshold(curB, &lowB, &highB, &cLowB, &cHighB, arLowB, arHighB, &lowIndexB, &highIndexB, &lowOverflowB, &highOverflowB); }

int calculateThreshold(int cur, int* low, int* high, bool* cLow, bool* cHigh, int arLow[], int arHigh[], int* lowIndex, int* highIndex, bool* lowOverflow, bool* highOverflow) {
    if (cur < *low) *low = cur;
    if (cur > *high) *high = cur;

    int curThresh = thresholdEquation(*low, *high);
    int range     = *high - *low;

    // The range is enforced to be over a certain limit because noise
    // can cause erroneous readings, making these calculations unstable.
    if (range >= SCROLL_THRESH_RANGE_LIM) {
        if (cur < curThresh) {
            if (*cHigh == true) {
                // We were just high, and now we crossed to low.
                // high reflects a sample of a high reading.
                arHigh[*highIndex] = *high;
                incrementIndex(highIndex, highOverflow);
                int midpoint = ((*high - *low) / 2) + *low;
                *low         = midpoint;
                *high        = midpoint;
                *cLow        = false;
                *cHigh       = false;
            } else {
                *cLow = true;
            }
        }
        if (cur > curThresh) {
            if (*cLow == true) {
                // We were just low, and now we crossed to high.
                // low reflects a sample of a low reading.
                arLow[*lowIndex] = *low;
                incrementIndex(lowIndex, lowOverflow);
                int midpoint = ((*high - *low) / 2) + *low;
                *low         = midpoint;
                *high        = midpoint;
                *cLow        = false;
                *cHigh       = false;
            } else {
                *cHigh = true;
            }
        }
    }

    int calcHigh = 0;
    if (*highOverflow == true) {
        for (int i = 0; i < SCROLLER_AR_SIZE; i++) {
            calcHigh += arHigh[i];
        }
        calcHigh = calcHigh / SCROLLER_AR_SIZE;
    } else if (*highIndex != 0) {
        for (int i = 0; i < *highIndex; i++) {
            calcHigh += arHigh[i];
        }
        calcHigh = calcHigh / *highIndex;
    } else {
        calcHigh = *high;
    }

    int calcLow = 0;
    if (*lowOverflow == true) {
        for (int i = 0; i < SCROLLER_AR_SIZE; i++) {
            calcLow += arLow[i];
        }
        calcLow = calcLow / SCROLLER_AR_SIZE;
    } else if (*lowIndex != 0) {
        for (int i = 0; i < *lowIndex; i++) {
            calcLow += arLow[i];
        }
        calcLow = calcLow / *lowIndex;
    } else {
        calcLow = *low;
    }

    return thresholdEquation(calcLow, calcHigh);
}

int thresholdEquation(int lo, int hi) { return ((hi - lo) / 3) + lo; }

void incrementIndex(int* index, bool* ovflw) {
    if (*index < SCROLLER_AR_SIZE - 1)
        (*index)++;
    else {
        *index = 0;
        *ovflw = true;
    }
}

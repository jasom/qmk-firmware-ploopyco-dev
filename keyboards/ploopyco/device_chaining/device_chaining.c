/* Copyright 2021 Colin Lam (Ploopy Corporation)
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

#include "quantum.h"
#include "wait.h"

#include "device_chaining.h"

#define MASTER_DATA_PIN_OUT B5    // pin 29
#define MASTER_DATA_PIN_IN B6     // pin 30
#define MASTER_LED_PIN D6         // pin 26
#define MASTER_BUTTON_PIN D5      // pin 22

#define SLAVE_DATA_PIN_IN B5      // pin 29
#define SLAVE_DATA_PIN_OUT B6     // pin 30
#define SLAVE_LED_PIN D6          // pin 26
#define SLAVE_BUTTON_PIN D5       // pin 22

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = { };

/*** MASTER CODE STARTS HERE ***************************************/

void keyboard_pre_init_kb(void) {
    setPinOutput(MASTER_DATA_PIN_OUT);
    setPinInputHigh(MASTER_DATA_PIN_IN);
    setPinOutput(MASTER_LED_PIN);
    setPinInput(MASTER_BUTTON_PIN);
}

void housekeeping_task_user(void) {
    uint8_t p = readPin(MASTER_BUTTON_PIN);
    if (p == 1)
        writePinHigh(MASTER_DATA_PIN_OUT);
    if (p == 0)
        writePinLow(MASTER_DATA_PIN_OUT);

    uint8_t q = readPin(MASTER_DATA_PIN_IN);
    if (q == 1)
        writePinHigh(MASTER_LED_PIN);
    if (q == 0)
        writePinLow(MASTER_LED_PIN);
}


/*** SLAVE CODE STARTS HERE ****************************************

void keyboard_pre_init_kb(void) {
    setPinOutput(SLAVE_DATA_PIN_OUT);
    setPinInputHigh(SLAVE_DATA_PIN_IN);
    setPinOutput(SLAVE_LED_PIN);
    setPinInput(SLAVE_BUTTON_PIN);
}

void housekeeping_task_user(void) {
    uint8_t p = readPin(SLAVE_BUTTON_PIN);
    if (p == 1)
        writePinHigh(SLAVE_DATA_PIN_OUT);
    if (p == 0)
        writePinLow(SLAVE_DATA_PIN_OUT);

    uint8_t q = readPin(SLAVE_DATA_PIN_IN);
    if (q == 1)
        writePinHigh(SLAVE_LED_PIN);
    if (q == 0)
        writePinLow(SLAVE_LED_PIN);
}
*/


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
#include "lufa.h"

#include "device_chaining.h"

// NOTE: These are NOT SPI pin definitions.
#define MOSI_PIN B5           // pin 29
#define MISO_PIN B6           // pin 30

#define REMOTE_LED_PIN D6     // pin 26
#define REMOTE_BUTTON_PIN D5  // pin 22
#define LOCAL_LED_PIN C7      // pin 32
#define LOCAL_BUTTON_PIN C6   // pin 31

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = { };

bool isMaster = false;

void keyboard_pre_init_kb(void) {
    setPinOutput(REMOTE_LED_PIN);
    setPinOutput(LOCAL_LED_PIN);
    setPinInput(REMOTE_BUTTON_PIN);
    setPinInput(LOCAL_LED_PIN);
}

void keyboard_post_init_user(void) {
    wait_ms(50);
    if (USB_DeviceState == DEVICE_STATE_Configured) {
        isMaster = true;
        setPinOutput(MOSI_PIN);
        setPinInputHigh(MISO_PIN);
    } else {
        setPinOutput(MISO_PIN);
        setPinInputHigh(MOSI_PIN);
    }
}

void housekeeping_task_user(void) {
    uint8_t p = readPin(REMOTE_BUTTON_PIN);
    if (p == 1) {
        if (isMaster)
            writePinHigh(MOSI_PIN);
        else
            writePinHigh(MISO_PIN);
    } else {
        if (isMaster)
            writePinLow(MOSI_PIN);
        else
            writePinLow(MISO_PIN);
    }

    uint8_t q = readPin(LOCAL_BUTTON_PIN);
    if (q == 1)
        writePinHigh(LOCAL_LED_PIN);
    if (q == 0)
        writePinLow(LOCAL_LED_PIN);

    uint8_t r;
    if (isMaster)
        r = readPin(MISO_PIN);
    else
        r = readPin(MOSI_PIN);

    if (r == 1)
        writePinHigh(REMOTE_LED_PIN);
    if (r == 0)
        writePinLow(REMOTE_LED_PIN);
}

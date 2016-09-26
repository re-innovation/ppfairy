#include "Config.h"
#include "Heartbeat.h"
#include "LatchedButton.h"
#include "VoltMode.h"
#include "LoadControl.h"
#include "MemoryFree.h"
#include <Arduino.h>
#include <avr/wdt.h>

// See Config.h for pin and other configuration

// Global variables

Heartbeat* heartbeat;
LatchedButton* resetButton;
LatchedButton* modeButton;
LoadControl* loadControl;
Mode* mode = NULL;

enum eModes {
    Volt,
    Cap
};
eModes nextMode = Volt;

void setNextMode()
{
#ifdef DEBUG
    int freeb4 = freeMemory();
#endif
    if (mode) {
        mode->stop();
        delete mode;
        mode = NULL;
    }
    switch (nextMode) {
    case Volt:
        mode = new VoltMode();
        nextMode = Cap;
        break;
    case Cap:
        mode = new VoltMode();
        nextMode = Volt;
        break;
    }
    mode->start();
#ifdef DEBUG
    Serial.print(F("setNextMode() free b4/now: "));
    Serial.print(freeb4);
    Serial.print(F("/"));
    Serial.println(freeMemory());
#endif
}

void setup()
{
    Serial.begin(115200);

    // Set up the blinker
    heartbeat = new Heartbeat(HEARTBEAT_LED_PIN);

    // Make an object for performing load control
    loadControl = new LoadControl();

    // Construct input buttons (sets pin modes in constructor)
    resetButton = new LatchedButton(RESET_BUTTON_PIN);
    modeButton = new LatchedButton(MODE_BUTTON_PIN);

    // Ensure load is disconnected at start, indicator off
    pinMode(INDICATOR_LED_PIN, OUTPUT);
    digitalWrite(INDICATOR_LED_PIN, LOW);

    // Let things settle
    delay(500);

    // enable watchdog reset at 1/4 sec
    wdt_enable(WDTO_250MS);

    // engage game mode
    setNextMode();

#ifdef DEBUG
    Serial.println(F("setup() E"));
#endif
}

void loop()
{
    // feed the watchdog
    wdt_reset();

    // give a time slice to various peripheral functions
    heartbeat->update();
    loadControl->update();
    resetButton->update();
    modeButton->update();

    // detect button presses and behave appropriately
    if (resetButton->wasPressed()) {
#ifdef DEBUG
        Serial.println(F("BUTTON: resetting mode"));
#endif
        mode->reset();
    }

    if (modeButton->wasPressed()) {
#ifdef DEBUG
        Serial.println(F("BUTTON: switching mode"));
#endif
        setNextMode();
    }

    mode->update();
}


#pragma once

#include <stdint.h>

#define DEBOUNCECOUNT   5
#define DEBOUNCETIME   5

class DebouncedButton {
public:
    DebouncedButton(uint8_t pin, bool pullup=true);
    virtual void update();
    bool isPressed();
    bool valueNow();
    void set(bool on);

private:
    uint8_t _pin;
    bool _pullup;
    uint8_t _count;
    unsigned long _last;
};


#include "bus.h"
#include "joypad.h"
#include "logging.h"

Joypad::Joypad(Bus& bus)
    : bus(bus) {
}

u8 Joypad::Read() {
    buttons_value = 0xFF;

    if (directions_selected) {
        if (down) buttons_value &= ~(1 << 3);
        if (up) buttons_value &= ~(1 << 2);
        if (left) buttons_value &= ~(1 << 1);
        if (right) buttons_value &= ~(1 << 0);
    }
    if (buttons_selected) {
        if (start) buttons_value &= ~(1 << 3);
        if (select) buttons_value &= ~(1 << 2);
        if (b) buttons_value &= ~(1 << 1);
        if (a) buttons_value &= ~(1 << 0);
    }

    return buttons_value;
}

void Joypad::Write(u8 value) {
    // Only bits 4 and 5 can be written.
    // This doesn't actually overwrite the register, but controls if
    // buttons and/or directions are enabled.
    directions_selected = !((value >> 4) & 0b1);
    buttons_selected = !((value >> 5) & 0b1);
}

void Joypad::PressButton(Button button) {
    if (button == Button::Up) up = true;
    if (button == Button::Down) down = true;
    if (button == Button::Left) left = true;
    if (button == Button::Right) right = true;
    if (button == Button::A) a = true;
    if (button == Button::B) b = true;
    if (button == Button::Select) select = true;
    if (button == Button::Start) start = true;
}

void Joypad::ReleaseButton(Button button) {
    if (button == Button::Up) up = false;
    if (button == Button::Down) down = false;
    if (button == Button::Left) left = false;
    if (button == Button::Right) right = false;
    if (button == Button::A) a = false;
    if (button == Button::B) b = false;
    if (button == Button::Select) select = false;
    if (button == Button::Start) start = false;
}

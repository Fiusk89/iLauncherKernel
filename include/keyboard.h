#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <kernel.h>

typedef struct keyboard
{
    char Key;
    char MultiKeys[7];
    bool isLeftShiftPressed;
    bool isRightShiftPressed;
    char keymap_size;
    char base[255];
    char shift[255];
    char press[255];
} keyboard_t;

typedef struct keyboard_keybuffer
{
    char *buffer;
    bool breakline;
    size_t size;
} keyboard_keybuffer_t;

extern keyboard_t en_international;
extern keyboard_t keyboard_ps2;

keyboard_t *keyboard_info_setup_layout(keyboard_t keyboard);
keyboard_keybuffer_t *keyboard_keybuffer_setup(size_t size, bool breakline);
void *keyboard_keybuffer_free(keyboard_keybuffer_t *keybuffer);
void *keyboard_keybuffer_read(keyboard_keybuffer_t *keybuffer);
void *keyboard_keybuffer_scan(keyboard_keybuffer_t *keybuffer, int8_t keycode);
void *keyboard_keybuffer_clear_characters(keyboard_keybuffer_t *keybuffer);
void keyboard_install();
void keyboard_reset();
char keyboard_get_key();
#endif
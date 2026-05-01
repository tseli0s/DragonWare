/**********************************************************************
 * FILE: driver.h
 * PURPOSE: VGA text mode driver utilities, ported from the bootloader and kernel
 * PROJECT: DragonWare Base System
 * DATE: 04-2026
 * AUTHOR: Aggelos Tselios <aggelostselios777@gmail.com>
 * LICENSE: GPL-3.0-or-later (https://spdx.org/licenses/GPL-3.0-or-later.html)
 **********************************************************************/

/** @brief Width of the VGA text mode console, in characters */
#define VGA_CONSOLE_WIDTH        (80)

/** @brief Height of the VGA text mode console, in characters */
#define VGA_CONSOLE_HEIGHT       (25)

/** @brief Size of the VGA text mode buffer in bytes. Each character is 16 bits wide,
 * eight bits for the color/attributes and eight for the character. */
#define VGA_BUFSIZE              ((Size)((VGA_WIDTH * VGA_HEIGHT) * sizeof(u16)))

/** @brief Where will the console be mapped in the virtual address space. (Maybe have a mechanism to
 * allow the kernel to choose this as well?) */
#define VGA_CONSOLE_VIRTUAL_ADDR ((void *)(0xA0000000))

/** @brief Helper to write a single message about the console status. */
#define LogStatus(msg)                          \
        VGAPrintString("[ CONSOLE ] " msg "\n", \
                       VGAGetColorAttribute(VGATEXT_COLOR_WHITE, VGATEXT_COLOR_BLACK));

#include <io.h>
#include <kerneltypes.h>

typedef enum _VGAColor : unsigned char {
        VGATEXT_COLOR_BLACK         = 0,
        VGATEXT_COLOR_BLUE          = 1,
        VGATEXT_COLOR_GREEN         = 2,
        VGATEXT_COLOR_CYAN          = 3,
        VGATEXT_COLOR_RED           = 4,
        VGATEXT_COLOR_MAGENTA       = 5,
        VGATEXT_COLOR_BROWN         = 6,
        VGATEXT_COLOR_LIGHT_GREY    = 7,
        VGATEXT_COLOR_DARK_GREY     = 8,
        VGATEXT_COLOR_LIGHT_BLUE    = 9,
        VGATEXT_COLOR_LIGHT_GREEN   = 10,
        VGATEXT_COLOR_LIGHT_CYAN    = 11,
        VGATEXT_COLOR_LIGHT_RED     = 12,
        VGATEXT_COLOR_LIGHT_MAGENTA = 13,
        VGATEXT_COLOR_LIGHT_BROWN   = 14,
        VGATEXT_COLOR_WHITE         = 15,
} VGAColor;

/**
 * @brief Returns the VGA text mode attribute byte for the given color-background
 * combination.
 * @param fg Foreground color (Color of the character)
 * @param bg Background color (Color around and behind the character)
 * @returns A single byte that is used as the attribute byte when printing a character.
 * @sa VGAGetCharacterWithColor
 */
static inline Byte VGAGetColorAttribute(VGAColor fg, VGAColor bg) {
        return (Byte)(fg | bg << (Byte)4);
}

/**
 * @brief Returns a two-byte pair of a character and an attribute byte that can be written
 * to VGA text mode memory to produce text output on the screen.
 * @param c The character to write. Must be ASCII.
 * @param color Color attributes. See @ref VGAGetColorAttribute
 */
static inline u16 VGAGetCharacterWithColor(char c, Byte color) {
        return (u16)c | (u16)((u16)color << 8);
}

/**
 * @brief Disables the VGA cursor.
 * @sa VGAPrintCharAt
 */
static inline void DisableVGACursor() {
        /* See https://wiki.osdev.org/Text_Mode_Cursor */
        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
}

/**
 * @brief Clears the entire VGA text mode screen by filling it with empty characters.
 * @param colorattr Color to use. See @ref VGAGetColorAttribute
 */
void VGAClearAllText(Byte colorattr);

/**
 * @brief Prints a character at the given position in the VGA text mode memory with the
 * given color attribute (see @ref VGAGetColorAttribute)
 * @param x Horizontal position of the character. Must be between 0 and VGA_CONSOLE_WIDTH.
 * @param y Vertical position of the character. Must be between 0 and VGA_CONSOLE_HEIGHT.
 * @param c Character to write. Must be ASCII.
 * @param color Color attribute of the character.
 */
void VGAPrintCharAt(int x, int y, char c, VGAColor color);

/**
 * @brief Writes a single character with the given color into VGA text mode memory.
 * @param c The character to write
 * @param color The color attribute of the character
 */
void VGAPrintCharacter(char c, VGAColor color);

/**
 * @brief Writes a string into VGA text mode memory.
 * @note Column and row are handled internally.
 * @param[in] str The string to print.
 * @param color Color to use for the string's characters.
 */
[[gnu::nonnull]]
void VGAPrintString(const char *str, VGAColor color);

/**
 * @brief Sets the position of the VGA text mode cursor to the given position.
 * @param x Horizontal position of the cursor.
 * @param y Vertical position of the cursor.
 */
void VGASetCursorPosition(unsigned int x, unsigned int y);

/**
 * @brief Scrolls up the VGA text screen contents by one row.
 * @sa VGAPrintString
 */
void VGAScrollBuffer(void);

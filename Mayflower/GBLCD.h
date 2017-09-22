#pragma once
#include "GBTypes.h"
#include "GBMMU.h"
#include "GameboyScreenPanel.h"
#include "EmulatorEngine.h"

#define SCREEN_WIDTH             160
#define SCREEN_HEIGHT            144
#define SCREEN_BUFFER_HEIGHT     256
#define SCREEN_BUFFER_WIDTH      256
#define CYCLES_TO_DRAW_SCAN_LINE 456 
#define LAST_VISIBLE_SCAN_LINE   144
#define MAX_SCAN_LINE            153
#define NUM_COLUMNS              160
#define TILE_WIDTH                 8
#define TILE_HEIGHT                8
#define SIZE_OF_SPRITE             4
#define NUM_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define LCD_MODE_HBLANK        0
#define LCD_MODE_VBLANK        1
#define LCD_MODE_SPRITE_SEARCH 2
#define LCD_MODE_TRANSFER_DATA 3
#define ABOVE_BACKGROUND       0
#define BEHIND_BACKGROUND      1
#define WHITE_PIXEL            0

#define M_LCDEnabled() (m_MMU->ReadMemory8(LCDC_REGISTER) & (1<<7))


typedef union lcd_control
{
	struct
	{
		byte BGDisplay : 1;               // Bit 0 - BG Display(for CGB see below) (0 = Off, 1 = On)
		byte OBJDisplayEnabled : 1;       // Bit 1 - OBJ(Sprite) Display Enable(0 = Off, 1 = On)
		byte OBJSpriteSizeFlag : 1;       // Bit 2 - OBJ(Sprite) Size(0 = 8x8, 1 = 8x16)
		byte BGTileMapDispSelect : 1;     // Bit 3 - BG Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		byte BGWindowTileDataSelect : 1;  // Bit 4 - BG & Window Tile Data Select(0 = 8800 - 97FF, 1 = 8000 - 8FFF)
		byte WindowDisplayEnabled : 1;    // Bit 5 - Window Display Enable(0 = Off, 1 = On)
		byte WindowTileMapDispSelect : 1; // Bit 6 - Window Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		byte LCDDisplayEnabled : 1;       // Bit 7 - LCD Display Enable(0 = Off, 1 = On)
	};
	byte all;

} lcd_control;

typedef union lcd_status
{
	struct
	{
		unsigned char mode : 2;                     // Bit 0-1
		unsigned char coincidence : 1;              // Bit 2
		unsigned char mode_0_int_selected : 1;      // Bit 3
		unsigned char mode_1_int_selected : 1;      // Bit 4
		unsigned char mode_2_int_selected : 1;      // Bit 5
		unsigned char coincidence_int_selected : 1; // Bit 6
		unsigned char reserved : 1;                 // Bit 7
	};
	unsigned char all;
} lcd_status;

typedef union sprite_attr
{
	struct
	{
		byte reserved : 4;
		byte PaletteNumber : 1;
		byte XFlip : 1;
		byte YFlip : 1;
		byte Priority : 1;
	};
	byte all;
}sprite_attr;

typedef struct sprite
{
	int YPosition;
	int XPosition;
	byte PatternNumber;
	sprite_attr Attributes;
}sprite;

class GameboyScreenPanel;
class EmulatorEngine;
class GBMMU;

class GBLCD
{
private:
	GBMMU *m_MMU;
	byte ReverseByte(byte x);
	byte m_ScreenBuffer[NUM_PIXELS];
	GameboyScreenPanel *m_ScreenPanel;
	EmulatorEngine *m_Emulator;
	int m_ScanLineCounter = 0;

public:
	void DrawScanLine();
	void RenderBackground();
	void RenderWindow();
	void RenderSprites();
	void DrawScreen();
	void UpdateGraphics(int cycles);
	void UpdateLCDStatus();
	byte *GetScreenBuffer();

	GBLCD(GBMMU *MMU, GameboyScreenPanel *ScreenPanel, EmulatorEngine *Emulator);
	~GBLCD();
};


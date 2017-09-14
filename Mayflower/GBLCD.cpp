#include "GBLCD.h"

using namespace std;

GBLCD::GBLCD(GBMMU *MMU, GameboyScreenPanel *ScreenPanel, EmulatorEngine *Emulator)
{
	m_MMU = MMU;
	m_ScreenPanel = ScreenPanel;
	m_Emulator = Emulator;
}

void GBLCD::DrawScanLine()
{
	lcd_control LCDControlFlags = { 0 };
	LCDControlFlags.all = m_MMU->ReadMemory8(LCDC_REGISTER);

	if (LCDControlFlags.BGDisplay)
	{
		RenderBackground();
	}

	if (LCDControlFlags.WindowDisplayEnabled)
	{
		RenderWindow();
	}

	if (LCDControlFlags.OBJDisplayEnabled)
	{
		RenderSprites();
	}
}

void GBLCD::RenderWindow()
{
	lcd_control LCDControlFlags = { 0 };
	LCDControlFlags.all = m_MMU->ReadMemory8(LCDC_REGISTER);
	byte WindowX = m_MMU->ReadMemory8(WX_REGISTER);// -7;
	byte WindowY = m_MMU->ReadMemory8(WY_REGISTER);
	byte CurrentLine = m_MMU->ReadMemory8(LY_REGISTER);
	byte BackgroundPalette = m_MMU->ReadMemory8(BGP_REGISTER);
	word TileMapStartAddr = 0;
	word TileDataStartAddr = 0;
	word ScreenBufferIndex = 0;
	bool SignedTileDataOffset = false;

	if (LCDControlFlags.WindowTileMapDispSelect)
	{
		TileMapStartAddr = 0x9C00;
	}
	else
	{
		TileMapStartAddr = 0x9800;
	}

	if (LCDControlFlags.BGWindowTileDataSelect)
	{
		TileDataStartAddr = 0x8000;
	}
	else
	{
		TileDataStartAddr = 0x9000;
		SignedTileDataOffset = true;
	}

	if (WindowX >= 7)
	{
		WindowX -= 7;
	}
	else
	{
		WindowX = 0;
	}

	if (CurrentLine >= WindowY)
	{
		for (int ColIter = WindowX; ColIter < NUM_COLUMNS; ColIter++)
		{
			int TileMapOffset = (((CurrentLine- WindowY) / TILE_WIDTH) * 32) + (ColIter / TILE_WIDTH);
			word TileMapAddr = TileMapStartAddr + TileMapOffset;
			byte TileNumber = m_MMU->ReadMemory8(TileMapAddr);
			word TileDataAddr = 0;

			if (SignedTileDataOffset)
			{
				sint8 offset = TileNumber;
				TileDataAddr = TileDataStartAddr + (offset * 16);
			}
			else
			{
				TileDataAddr = TileDataStartAddr + (TileNumber * 16);
			}

			byte TileCol = ColIter % 8;
			byte TileRow = CurrentLine % 8;
			byte RowA = m_MMU->ReadMemory8(TileDataAddr + TileRow * 2);
			byte RowB = m_MMU->ReadMemory8(TileDataAddr + TileRow * 2 + 1);
			byte ColBit = (7 - TileCol);
			byte PaletteIndex = (RowA & (1 << ColBit) ? 1 : 0) | (RowB & (1 << ColBit) ? 2 : 0);
			//byte PixelValue = (BackgroundPalette & (0b11 << (PaletteIndex * 2))) >> (PaletteIndex * 2);
			byte PixelValue = (BackgroundPalette & (0b11 << (PaletteIndex * 2))) >> (PaletteIndex * 2);

			ScreenBufferIndex = ((CurrentLine - 1) * 160) + ColIter;
			m_ScreenBuffer[ScreenBufferIndex] = PixelValue;

		}
	}
}

void GBLCD::RenderBackground()
{
	lcd_control LCDControlFlags = { 0 };
	LCDControlFlags.all = m_MMU->ReadMemory8(LCDC_REGISTER);
	byte CurrentLine = m_MMU->ReadMemory8(LY_REGISTER);
	byte ScrollX = m_MMU->ReadMemory8(SCX_REGISTER);
	byte ScrollY = m_MMU->ReadMemory8(SCY_REGISTER);
	word TileMapStartAddr = 0;
	word TileDataStartAddr = 0;
	int AbsoluteY = CurrentLine + ScrollY;
	int TileY = (AbsoluteY % SCREEN_BUFFER_HEIGHT) / TILE_HEIGHT;
	bool SignedTileDataOffset = false;
	byte BackgroundPalette = m_MMU->ReadMemory8(BGP_REGISTER);
	int ScreenBufferIndex = 0;

	//cout << "Palette: " << (unsigned short)BackgroundPalette << endl;

	if (LCDControlFlags.BGTileMapDispSelect)
	{
		TileMapStartAddr = 0x9C00;
	}
	else
	{
		TileMapStartAddr = 0x9800;
	}

	if (LCDControlFlags.BGWindowTileDataSelect)
	{
		TileDataStartAddr = 0x8000;
	}
	else
	{
		TileDataStartAddr = 0x9000;
		SignedTileDataOffset = true;
	}


	for (int ColIter = 0; ColIter < NUM_COLUMNS; ColIter++)
	{
		int AbsoluteX = ScrollX + ColIter;
		int TileX = (AbsoluteX % SCREEN_BUFFER_WIDTH) / TILE_WIDTH;
		int TileMapOffset = (TileY * 32) + TileX;
		short TileMapAddr = TileMapStartAddr + TileMapOffset;
		byte TileNumber = m_MMU->ReadMemory8(TileMapAddr);
		word TileDataAddr = 0;
		//cout << "Tile Number: " << (unsigned short) TileNumber << endl;
		if (SignedTileDataOffset)
		{
			char offset = TileNumber;
			TileDataAddr = TileDataStartAddr + (offset * 16);
		}
		else
		{
			TileDataAddr = TileDataStartAddr + (TileNumber * 16);
		}

		byte TileCol = AbsoluteX % 8;
		byte TileRow = AbsoluteY % 8;
		byte RowA = m_MMU->ReadMemory8(TileDataAddr + TileRow * 2);
		byte RowB = m_MMU->ReadMemory8(TileDataAddr + TileRow * 2 + 1);
		byte ColBit = (7 - TileCol);
		byte PaletteIndex = (RowA & (1 << ColBit) ? 1 : 0) | (RowB & (1 << ColBit) ? 2 : 0);
		//byte PixelValue = (BackgroundPalette & (0b11 << (PaletteIndex * 2))) >> (PaletteIndex * 2);
		byte PixelValue = (BackgroundPalette & (0b11 << (PaletteIndex * 2))) >> (PaletteIndex * 2);

		ScreenBufferIndex = ((CurrentLine - 1) * 160) + ColIter;
		m_ScreenBuffer[ScreenBufferIndex] = PixelValue;
	}

}

void GBLCD::RenderSprites()
{
	int SpritesDrawn = 0;
	int ScreenBufferIndex = 0;
	byte ScanLine = m_MMU->ReadMemory8(LY_REGISTER);
	int SpriteIter = 0;
	byte RowA = 0;
	byte RowB = 0;
	byte VerticalSize = 0;
	byte Palette = 0;
	sprite CurrentSprite = { 0 };
	lcd_control LCDControlFlags = { 0 };
	LCDControlFlags.all = m_MMU->ReadMemory8(LCDC_REGISTER);

	VerticalSize = LCDControlFlags.OBJSpriteSizeFlag ? 16 : 8;

	for (SpriteIter = 0; SpriteIter <= 40; SpriteIter++)
	{
		CurrentSprite.YPosition = m_MMU->ReadMemory8(OAM_START_ADDR + SpriteIter * SIZE_OF_SPRITE) - 16;
		CurrentSprite.XPosition = m_MMU->ReadMemory8(OAM_START_ADDR + SpriteIter * SIZE_OF_SPRITE + 1) - 8;
		CurrentSprite.PatternNumber = m_MMU->ReadMemory8(OAM_START_ADDR + SpriteIter * SIZE_OF_SPRITE + 2);
		CurrentSprite.Attributes.all = m_MMU->ReadMemory8(OAM_START_ADDR + SpriteIter * SIZE_OF_SPRITE + 3);

		// X=0, Y=0 Denotes a hidden sprite
		if (CurrentSprite.XPosition == 0 && CurrentSprite.YPosition == 0)
		{
			continue;
		}

		// Check if sprite intersects with the scan line
		if (CurrentSprite.YPosition <= ScanLine && (CurrentSprite.YPosition + VerticalSize) >= ScanLine)
		{
			// Read sprite out of video ram
			if (CurrentSprite.Attributes.YFlip)
			{
				RowA = m_MMU->ReadMemory8(SPRITE_DATA + CurrentSprite.PatternNumber * 16 + (16 - (ScanLine - CurrentSprite.YPosition) * 2));
				RowB = m_MMU->ReadMemory8(SPRITE_DATA + CurrentSprite.PatternNumber * 16 + (16 - (ScanLine - CurrentSprite.YPosition) * 2) + 1);
			}
			else
			{
				RowA = m_MMU->ReadMemory16(SPRITE_DATA + CurrentSprite.PatternNumber * 16 + (ScanLine - CurrentSprite.YPosition) * 2);
				RowB = m_MMU->ReadMemory16(SPRITE_DATA + CurrentSprite.PatternNumber * 16 + (ScanLine - CurrentSprite.YPosition) * 2 + 1);
			}

			if (CurrentSprite.Attributes.XFlip)
			{
				RowA = ReverseByte(RowA);
				RowB = ReverseByte(RowB);
			}

			Palette = m_MMU->ReadMemory8(OBP0_REGISTER + CurrentSprite.Attributes.PaletteNumber);

			for (int ColIter = 0; ColIter < 8; ColIter++)
			{
				if (CurrentSprite.XPosition + ColIter >= SCREEN_WIDTH)
				{
					break;
				}
				else if (CurrentSprite.XPosition + ColIter < 0)
				{
					continue;
				}
				ScreenBufferIndex = ((ScanLine - 1) * SCREEN_WIDTH) + CurrentSprite.XPosition + ColIter;
				byte ColBit = (7 - ColIter);
				byte PaletteIndex = ((RowA & (1 << ColBit)) ? 1 : 0) | ((RowB & (1 << ColBit)) ? 2 : 0);
				if (PaletteIndex == 0)continue;
				byte PixelValue = (Palette & (0b11 << (PaletteIndex * 2))) >> (PaletteIndex * 2);
				if (((CurrentSprite.Attributes.Priority == ABOVE_BACKGROUND) ||
					m_ScreenBuffer[ScreenBufferIndex] == WHITE_PIXEL) )//&& PixelValue != WHITE_PIXEL )
				{
					m_ScreenBuffer[ScreenBufferIndex] = PixelValue;
				}
			}

			SpritesDrawn++;
			if (SpritesDrawn == 10)
			{
				break;
			}
		}

	}
}


void GBLCD::DrawScreen()
{
	m_ScreenPanel->Refresh();
	//m_NumFramesRendered++;
	//cout << "Drawing Frame: " << m_NumFramesRendered << endl;
}

void GBLCD::UpdateGraphics(int cycles)
{
	UpdateLCDStatus();

	if (M_LCDEnabled())
	{
		// Increment the cycles spent on this scan line
		m_ScanLineCounter += cycles;

		// If the requisite number of cycles have been spent drawing the last
		// scan line. Move on to the next
		if (m_ScanLineCounter >= CYCLES_TO_DRAW_SCAN_LINE)
		{
			// Move on to the next scan line
			m_MMU->WriteMemory8(LY_REGISTER, m_MMU->ReadMemory8(LY_REGISTER) + 1);
			byte CurrentLine = m_MMU->ReadMemory8(LY_REGISTER);

			// Reset amount of time (cycles) that have been spent on the
			// current line
			m_ScanLineCounter = 0;

			// If we have just passed the last visible scan line request
			// V-blank interrupt
			if (CurrentLine == (LAST_VISIBLE_SCAN_LINE + 1))
			{
				m_Emulator->RequestInterrupt(VBLANK_INTERRUPT);
			}
			// If the max scan line has been crossed, loop back around to 0
			else if (CurrentLine > MAX_SCAN_LINE)
			{
				m_MMU->WriteMemory8(LY_REGISTER, 0);
			}
			// If the current line is visible, draw it!
			else if (CurrentLine <= LAST_VISIBLE_SCAN_LINE)
			{
				DrawScanLine();
			}
		}
	}
	else
	{
		//cout << "LCD Disabled" << endl;
	}

}

void GBLCD::UpdateLCDStatus()
{
	// It takes 456 clock cycles to draw one scanline
	// Cycles   0-79 : Mode 2 (SEARCHING SPRITES ATTS)
	// Cycles  80-251: Mode 3 (Transfer data to LCD Driver)
	// Cycles 252-455: Mode 0 (H-Blank)
	lcd_status CurrentLCDStatus = { 0 };
	CurrentLCDStatus.all = m_MMU->ReadMemory8(STAT_REGISTER);
	bool InterruptRequested = false;
	byte OriginalMode = CurrentLCDStatus.mode;

	if (M_LCDEnabled())
	{
		// Set the new LCD status mode. If the mode changes
		// and the interrupt for that mode change is selected
		// request and interrupt.
		byte CurrentScanLine = m_MMU->ReadMemory8(LY_REGISTER);

		if (CurrentScanLine > LAST_VISIBLE_SCAN_LINE)
		{
			CurrentLCDStatus.mode = LCD_MODE_VBLANK;
			InterruptRequested = CurrentLCDStatus.mode_1_int_selected;
		}
		else
		{
			if (m_ScanLineCounter >= 0 && m_ScanLineCounter < 80)
			{
				CurrentLCDStatus.mode = LCD_MODE_SPRITE_SEARCH;
				InterruptRequested = CurrentLCDStatus.mode_2_int_selected;
			}
			else if (m_ScanLineCounter >= 80 && m_ScanLineCounter < 252)
			{
				CurrentLCDStatus.mode = LCD_MODE_TRANSFER_DATA;
			}
			else
			{
				CurrentLCDStatus.mode = LCD_MODE_HBLANK;
				InterruptRequested = CurrentLCDStatus.mode_0_int_selected;
			}
		}

		// If mode has changed
		if ((CurrentLCDStatus.mode != OriginalMode) && InterruptRequested)
		{
			m_Emulator->RequestInterrupt(LCD_STAT_INTERRUPT);
		}

		// Check coincidence flag
		if (CurrentScanLine == m_MMU->ReadMemory8(LYC_REGISTER))
		{
			CurrentLCDStatus.coincidence = 1;
			if (CurrentLCDStatus.coincidence_int_selected)
			{
				m_Emulator->RequestInterrupt(VBLANK_INTERRUPT);
			}
		}
		else
		{
			CurrentLCDStatus.coincidence = 0;
		}

		// Update LCD Status
		m_MMU->WriteMemory8(STAT_REGISTER, CurrentLCDStatus.all);
	}
	else
	{
		// Set LCD mode to 1 (V-Blank) and reset scanline counter
		m_ScanLineCounter = 0;
		m_MMU->WriteMemory8(LY_REGISTER, 0);
		CurrentLCDStatus.mode = LCD_MODE_HBLANK;//LCD_MODE_VBLANK;
		m_MMU->WriteMemory8(STAT_REGISTER, CurrentLCDStatus.all);
	}

}

byte *GBLCD::GetScreenBuffer()
{
	return m_ScreenBuffer;
}

byte GBLCD::ReverseByte(byte x)
{
	static const byte table[] = {
		0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
		0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
		0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
		0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
		0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
		0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
		0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
		0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
		0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
		0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
		0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
		0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
		0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
		0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
		0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
		0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
		0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
		0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
		0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
		0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
		0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
		0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
		0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
		0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
		0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
		0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
		0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
		0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
		0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
		0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
		0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
		0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
	};
	return table[x];
}


GBLCD::~GBLCD()
{
}

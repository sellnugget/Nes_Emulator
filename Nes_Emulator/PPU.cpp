#include "PPU.h"
#include <random>

namespace Hardware {
	PPU::PPU(Bus* cpubus, Cartridge* cartrige)
	{
		_ppuBus = new Bus();
		_cpuBus = cpubus;
		_sprScreen = new Graphics::Image();
		_sprScreen->Create(256, 256);

		_PatternTable[0] = new Graphics::Image();
		_PatternTable[1] = new Graphics::Image();
		_PatternTable[0]->Create(256,256);
		_PatternTable[1]->Create(256, 256);
		_ppuBus->BindDevice("CharRom");
		_ppuBus->BindReadDevice("CharRom", [cartrige](uint16_t address) {return cartrige->_Char_Rom[address]; });
		_ppuBus->BindWriteDevice("CharRom", [cartrige](uint16_t address, uint8_t data) {cartrige->_Char_Rom[address] = data; });
		_ppuBus->MapRegion("CharRom", 0, 0x2000);

		_ppuBus->BindDevice("NameTable");
		_ppuBus->BindReadDevice("NameTable", [this, cartrige](uint16_t address)
			{
	
				address &= 0x0FFF;
				uint8_t data;
				if (cartrige->mirror == Cartridge::MIRROR::VERTICAL)
				{
					// Vertical
					if (address >= 0x0000 && address <= 0x03FF)
						data = tblName[0][address & 0x03FF];
					if (address >= 0x0400 && address <= 0x07FF)
						data = tblName[1][address & 0x03FF];
					if (address >= 0x0800 && address <= 0x0BFF)
						data = tblName[0][address & 0x03FF];
					if (address >= 0x0C00 && address <= 0x0FFF)
						data = tblName[1][address & 0x03FF];
				}
				else if (cartrige->mirror == Cartridge::MIRROR::HORIZONTAL)
				{
					// Horizontal
					if (address >= 0x0000 && address <= 0x03FF)
						data = tblName[0][address & 0x03FF];
					if (address >= 0x0400 && address <= 0x07FF)
						data = tblName[0][address & 0x03FF];
					if (address >= 0x0800 && address <= 0x0BFF)
						data = tblName[1][address & 0x03FF];
					if (address >= 0x0C00 && address <= 0x0FFF)
						data = tblName[1][address & 0x03FF];
				}
				return data;
			});

		_ppuBus->BindWriteDevice("NameTable", [this, cartrige](uint16_t address, uint8_t data)
			{
				address &= 0x0FFF;
				if (cartrige->mirror == Cartridge::MIRROR::VERTICAL)
				{
					// Vertical
					if (address >= 0x0000 && address <= 0x03FF)
						tblName[0][address & 0x03FF] = data;
					if (address >= 0x0400 && address <= 0x07FF)
						tblName[1][address & 0x03FF] = data;
					if (address >= 0x0800 && address <= 0x0BFF)
						tblName[0][address & 0x03FF] = data;
					if (address >= 0x0C00 && address <= 0x0FFF)
						tblName[1][address & 0x03FF] = data;
				}
				else if (cartrige->mirror == Cartridge::MIRROR::HORIZONTAL)
				{
					// Horizontal
					if (address >= 0x0000 && address <= 0x03FF)
						tblName[0][address & 0x03FF] = data;
					if (address >= 0x0400 && address <= 0x07FF)
						tblName[0][address & 0x03FF] = data;
					if (address >= 0x0800 && address <= 0x0BFF)
						tblName[1][address & 0x03FF] = data;
					if (address >= 0x0C00 && address <= 0x0FFF)
						tblName[1][address & 0x03FF] = data;
				}
			});

		_ppuBus->BindDevice( "pallete");
		_ppuBus->BindReadDevice("pallete", [this](uint16_t address)
			{
				address &= 0x001F;
				if (address == 0x0010) address = 0x0000;
				if (address == 0x0014) address = 0x0004;
				if (address == 0x0018) address = 0x0008;
				if (address == 0x001C) address = 0x000C;
				return tblPalete[address] & (mask.grayscale ? 0x30 : 0x3F);
			});
		_ppuBus->BindWriteDevice("pallete", [this](uint16_t address, uint8_t data)
			{
				address &= 0x001F;
				if (address == 0x0010) address = 0x0000;
				if (address == 0x0014) address = 0x0004;
				if (address == 0x0018) address = 0x0008;
				if (address == 0x001C) address = 0x000C;
				tblPalete[address] = data;
			});


		_ppuBus->MapRegion("pallete", 0x3f00, 0xff);
		//defines all possible colors on the nes
		{
			_NTSCScreen[0x00] = Graphics::Vector3(84, 84, 84);
			_NTSCScreen[0x01] = Graphics::Vector3(0, 30, 116);
			_NTSCScreen[0x02] = Graphics::Vector3(8, 16, 144);
			_NTSCScreen[0x03] = Graphics::Vector3(48, 0, 136);
			_NTSCScreen[0x04] = Graphics::Vector3(68, 0, 100);
			_NTSCScreen[0x05] = Graphics::Vector3(92, 0, 48);
			_NTSCScreen[0x06] = Graphics::Vector3(84, 4, 0);
			_NTSCScreen[0x07] = Graphics::Vector3(60, 24, 0);
			_NTSCScreen[0x08] = Graphics::Vector3(32, 42, 0);
			_NTSCScreen[0x09] = Graphics::Vector3(8, 58, 0);
			_NTSCScreen[0x0A] = Graphics::Vector3(0, 64, 0);
			_NTSCScreen[0x0B] = Graphics::Vector3(0, 60, 0);
			_NTSCScreen[0x0C] = Graphics::Vector3(0, 50, 60);
			_NTSCScreen[0x0D] = Graphics::Vector3(0, 0, 0);
			_NTSCScreen[0x0E] = Graphics::Vector3(0, 0, 0);
			_NTSCScreen[0x0F] = Graphics::Vector3(0, 0, 0);

			_NTSCScreen[0x10] = Graphics::Vector3(152, 150, 152);
			_NTSCScreen[0x11] = Graphics::Vector3(8, 76, 196);
			_NTSCScreen[0x12] = Graphics::Vector3(48, 50, 236);
			_NTSCScreen[0x13] = Graphics::Vector3(92, 30, 228);
			_NTSCScreen[0x14] = Graphics::Vector3(136, 20, 176);
			_NTSCScreen[0x15] = Graphics::Vector3(160, 20, 100);
			_NTSCScreen[0x16] = Graphics::Vector3(152, 34, 32);
			_NTSCScreen[0x17] = Graphics::Vector3(120, 60, 0);
			_NTSCScreen[0x18] = Graphics::Vector3(84, 90, 0);
			_NTSCScreen[0x19] = Graphics::Vector3(40, 114, 0);
			_NTSCScreen[0x1A] = Graphics::Vector3(8, 124, 0);
			_NTSCScreen[0x1B] = Graphics::Vector3(0, 118, 40);
			_NTSCScreen[0x1C] = Graphics::Vector3(0, 102, 120);
			_NTSCScreen[0x1D] = Graphics::Vector3(0, 0, 0);
			_NTSCScreen[0x1E] = Graphics::Vector3(0, 0, 0);
			_NTSCScreen[0x1F] = Graphics::Vector3(0, 0, 0);

			_NTSCScreen[0x20] = Graphics::Vector3(236, 238, 236);
			_NTSCScreen[0x21] = Graphics::Vector3(76, 154, 236);
			_NTSCScreen[0x22] = Graphics::Vector3(120, 124, 236);
			_NTSCScreen[0x23] = Graphics::Vector3(176, 98, 236);
			_NTSCScreen[0x24] = Graphics::Vector3(228, 84, 236);
			_NTSCScreen[0x25] = Graphics::Vector3(236, 88, 180);
			_NTSCScreen[0x26] = Graphics::Vector3(236, 106, 100);
			_NTSCScreen[0x27] = Graphics::Vector3(212, 136, 32);
			_NTSCScreen[0x28] = Graphics::Vector3(160, 170, 0);
			_NTSCScreen[0x29] = Graphics::Vector3(116, 196, 0);
			_NTSCScreen[0x2A] = Graphics::Vector3(76, 208, 32);
			_NTSCScreen[0x2B] = Graphics::Vector3(56, 204, 108);
			_NTSCScreen[0x2C] = Graphics::Vector3(56, 180, 204);
			_NTSCScreen[0x2D] = Graphics::Vector3(60, 60, 60);
			_NTSCScreen[0x2E] = Graphics::Vector3(0, 0, 0);
			_NTSCScreen[0x2F] = Graphics::Vector3(0, 0, 0);

			_NTSCScreen[0x30] = Graphics::Vector3(236, 238, 236);
			_NTSCScreen[0x31] = Graphics::Vector3(168, 204, 236);
			_NTSCScreen[0x32] = Graphics::Vector3(188, 188, 236);
			_NTSCScreen[0x33] = Graphics::Vector3(212, 178, 236);
			_NTSCScreen[0x34] = Graphics::Vector3(236, 174, 236);
			_NTSCScreen[0x35] = Graphics::Vector3(236, 174, 212);
			_NTSCScreen[0x36] = Graphics::Vector3(236, 180, 176);
			_NTSCScreen[0x37] = Graphics::Vector3(228, 196, 144);
			_NTSCScreen[0x38] = Graphics::Vector3(204, 210, 120);
			_NTSCScreen[0x39] = Graphics::Vector3(180, 222, 120);
			_NTSCScreen[0x3A] = Graphics::Vector3(168, 226, 144);
			_NTSCScreen[0x3B] = Graphics::Vector3(152, 226, 180);
			_NTSCScreen[0x3C] = Graphics::Vector3(160, 214, 228);
			_NTSCScreen[0x3D] = Graphics::Vector3(160, 162, 160);
			_NTSCScreen[0x3E] = Graphics::Vector3(0, 0, 0);
			_NTSCScreen[0x3F] = Graphics::Vector3(0, 0, 0);
		}
		

		cpubus->BindDevice("PPUCTRL");
		cpubus->BindDevice("PPUMASK");
		cpubus->BindDevice("PPUSTATUS");
		cpubus->BindDevice("OAMADDR");
		cpubus->BindDevice("OAMDATA");
		cpubus->BindDevice("PPUSCROLL");
		cpubus->BindDevice("PPUADDR");
		cpubus->BindDevice("PPUDATA");
		cpubus->BindDevice("OAMDMA");

		//Controller($2000) > write
		//common name: PPUCTRL
		//Description: PPU control register
		//Access: Write
		//function: various flags controlling ppu operation

		cpubus->BindWriteDevice("PPUCTRL", [this](uint16_t address, uint8_t data)
		{
				control.reg = data;
				tram_addr.nametable_x = control.nametable_x;
				tram_addr.nametable_y = control.nametable_y;
		});

		//Mask($2001) > write
		//common name: PPUMASK
		//Description: PPU mask register
		//Access: write
		//function: this register controls the rendering of sprites and backgrounds, as well as colour effects

		cpubus->BindWriteDevice("PPUMASK", [this](uint16_t address, uint8_t data)
			{
				mask.reg = data;
			});
		cpubus->BindReadDevice("PPUSTATUS", [this](uint16_t address)
			{
				uint8_t rtn = (status.reg & 0xe0) | (ppu_data_buffer & 0x1f);
				status.vertical_blank = 0;
				address_latch = 0;
				return rtn;
			});

		//for sprites
		cpubus->BindReadDevice("OAMDATA", [this](uint16_t address) {
			return 0;
			});

		cpubus->BindWriteDevice("PPUSCROLL", [this](uint16_t address, uint8_t data) {

			if (address_latch == 0) {
				fine_x = data & 0x07;
				tram_addr.coarse_x = data >> 3;
				address_latch = 1;
			}
			else {
				tram_addr.fine_y = data & 0x07;
				tram_addr.fine_y = data >> 3;
				address_latch = 0;
			}
			});
		cpubus->BindWriteDevice("OAMDATA", [this](uint16_t address, uint8_t data) {
		});
		cpubus->BindWriteDevice("PPUADDR", [this](uint16_t address, uint8_t data)
			{
				if (address_latch == 0) {
					tram_addr.reg = (tram_addr.reg & 0x00ff) | (data << 8);
					address_latch = 1;
				}
				else {
					tram_addr.reg = (tram_addr.reg & 0xff00) | data;
					vram_addr = tram_addr;
					address_latch = 0;
				}
			});

		cpubus->BindReadDevice("PPUDATA", [this](uint16_t address)
			{
				uint8_t rtn = ppu_data_buffer;
				ppu_data_buffer = _ppuBus->ReadByte(vram_addr.reg);
				if (vram_addr.reg > 0x3f00) rtn = ppu_data_buffer;
				vram_addr.reg += (control.increment_mode ? 32 : 1);
				return rtn;
			});
		cpubus->BindWriteDevice("PPUDATA", [this](uint16_t address, uint8_t data)
			{
				_ppuBus->WriteByte(vram_addr.reg, data);
				vram_addr.reg += (control.increment_mode ? 32 : 1);
			
			});
		cpubus->MapRegion("PPUCTRL", 0x2000, 1);
		cpubus->MapRegion("PPUMASK", 0x2001, 1);
		cpubus->MapRegion("PPUSTATUS", 0x2002, 1);
		cpubus->MapRegion("OAMADDR", 0x2003, 1);
		cpubus->MapRegion("OAMDATA", 0x2004, 1);
		cpubus->MapRegion("PPUSCROLL", 0x2005, 1);
		cpubus->MapRegion("PPUADDR", 0x2006, 1);
		cpubus->MapRegion("PPUDATA", 0x2007, 1);

	}
	

	

	void PPU::OnUpdate()
	{
		GenerateTableImages();
		
	}
	void PPU::Clock()
	{
	
		
		if (scanline == 241 && cycle == 1) {
			status.vertical_blank = 1;
			if (control.enable_nmi) {
				_cpuBus->NMI = true;
			}
			
		}
		//only renders if this is true
		if (scanline >= -1 && cycle < 256) {
			
			if (scanline == -1 && cycle == 1)
			{
				//start of new frame 
				status.vertical_blank = 0;
			}
			//gets next tile for 2 cycles
			if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338)) {
				if (mask.render_background == true) {

				}
				if((cycle - 1))
			}
		}
		cycle++;
		if (cycle == 341) {
			cycle = 1;
			scanline++;
		}
		if (scanline == 260) {
			scanline = -1;
		}


		
	}
	Graphics::Vector3 PPU::GetColorFromPalletRam(uint8_t palette, uint8_t pixel)
	{
		//normal
		uint8_t rtn = _ppuBus->ReadByte(0x3F00 + (palette << 2) + pixel) & 0x3F;
		return _NTSCScreen[rtn];

	}
	void PPU::GenerateTableImages()
	{
		for (int i = 0; i < 2; i++) {
			for (uint16_t nTileY = 0; nTileY < 16; nTileY++)
			{
				for (uint16_t nTileX = 0; nTileX < 16; nTileX++)
				{
					// Convert the 2D tile coordinate into a 1D offset into the pattern
					// table memory.
					uint16_t nOffset = nTileY * 256 + nTileX * 16;

					// Now loop through 8 rows of 8 pixels
					for (uint16_t row = 0; row < 8; row++)
					{
						// For each row, we need to read both bit planes of the character
						// in order to extract the least significant and most significant 
						// bits of the 2 bit pixel value. in the CHR ROM, each character
						// is stored as 64 bits of lsb, followed by 64 bits of msb. This
						// conveniently means that two corresponding rows are always 8
						// bytes apart in memory.
						uint8_t tile_lsb = _ppuBus->ReadByte(i * 0x1000 + nOffset + row + 0x0000);
						uint8_t tile_msb = _ppuBus->ReadByte(i * 0x1000 + nOffset + row + 0x0008);


						// Now we have a single row of the two bit planes for the character
						// we need to iterate through the 8-bit words, combining them to give
						// us the final pixel index
						for (uint16_t col = 0; col < 8; col++)
						{
							// We can get the index value by simply adding the bits together
							// but we're only interested in the lsb of the row words because...
							uint8_t pixel = (tile_lsb & 0x01) + (tile_msb & 0x01);

							// ...we will shift the row words 1 bit right for each column of
							// the character.
							tile_lsb >>= 1; tile_msb >>= 1;

							// Now we know the location and NES pixel value for a specific location
							// in the pattern table, we can translate that to a screen colour, and an
							// (x,y) location in the sprite
							_PatternTable[i]->WritePixel
							(
								nTileX * 8 + (7 - col), // Because we are using the lsb of the row word first
														// we are effectively reading the row from right
														// to left, so we need to draw the row "backwards"
								nTileY * 8 + row,
								GetColorFromPalletRam(2, pixel)
							);
						}
					}
				}
			}
		}
	
	}
}


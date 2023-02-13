#pragma once
#include "Bus.h"
#include "Renderer.h"
#include "RamModual.h"
#include<windows.h>
#include "Cartridge.h"
namespace Hardware {
	class PPU
	{
		
	private:
		struct Tile {
			Graphics::Vector3 Image[64];
		};
		Graphics::Vector3 _NTSCScreen[0x40];

		uint8_t tblPalete[32];
		uint8_t tblName[2][1024];
		Graphics::Vector3 GetColorFromPalletRam(uint8_t palette, uint8_t pixel);
		void GenerateTableImages();
	public:
		PPU(Bus* cpubus, Cartridge* cartrige);
		Graphics::Image* _sprScreen;
		Graphics::Image* _PatternTable[2];
		void OnUpdate();
		void Clock();
	private:
		Bus* _ppuBus;
		Bus* _cpuBus;


		long cycle;
		long scanline;

		uint8_t address_latch = 0x00;
		uint8_t ppu_data_buffer = 0x00;
	   

		union loopy_register {
			struct {
				uint16_t coarse_x : 5;
				uint16_t coarse_y : 5;
				uint16_t nametable_x : 1;
				uint16_t nametable_y : 1;
				uint16_t fine_y : 3;
				uint16_t unused : 1;
			};
			uint16_t reg = 0x0000;
		};

		loopy_register vram_addr;
		loopy_register tram_addr;

		uint8_t fine_x = 0x00;

		uint8_t bg_next_tile_id = 0x00;
		uint8_t bg_next_tile_attrib = 0x00;
		uint8_t bg_next_tile_lsb = 0x00;
		uint8_t bg_next_tile_msb = 0x00;
		uint16_t bg_shifter_pattern_lo = 0x0000;
		uint16_t bg_shifter_pattern_hi = 0x0000;
		uint16_t bg_shifter_attrib_lo = 0x0000;
		uint16_t bg_shifter_attrib_hi = 0x0000;

		union {
			struct {
				uint8_t unused : 5;
				uint8_t sprite_overflow : 1;
				uint8_t sprite_zero_hit : 1;
				uint8_t vertical_blank : 1;
			};
			uint8_t reg;
		} status;

		union {
			struct {
				uint8_t grayscale : 1;
				uint8_t render_background_left : 1;
				uint8_t render_sprites_left : 1;
				uint8_t render_background : 1;
				uint8_t render_sprites : 1;
				uint8_t enhance_red : 1;
				uint8_t enhance_green;
				uint8_t enhance_blue : 1;
			};
			uint8_t reg;
		} mask;

		union PPUCTRL {
			struct {
				uint8_t nametable_x : 1;
				uint8_t nametable_y : 1;
				uint8_t increment_mode : 1;
				uint8_t pattern_sprite : 1;
				uint8_t pattern_background : 1;
				uint8_t sprite_size : 1;
				uint8_t slave_mode : 1;
				uint8_t enable_nmi : 1;
			};
			uint8_t reg;
		} control;

	};


}

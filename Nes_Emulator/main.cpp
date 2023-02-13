
#include <iostream>
#include "Renderer.h"
#include "EMU_6502.h"
#include "Cartridge.h"
#include "RamModual.h"
#include "PPU.h"

int main(void)
{
	Graphics::Renderer render;
	Hardware::Bus bus;
	Hardware::EMU_6502 _cpu(&bus);
	Hardware::RamModual ram(&bus, 0x800, 0, 3);
	std::string input;
	std::getline(std::cin, input);
	if (input[0] == '\"') {
		input = input.substr(1, input.size() - 2);
	}
	Hardware::Cartridge rom(input);
	Hardware::PPU ppu(&bus, &rom);
	rom.MapToBus(&bus, 0x6000, false);

	//_cpu.StartCPU(&bus);



	render.AddImage("screen", ppu._sprScreen);
	render.AddImage("nametable0", ppu._PatternTable[0]);
	render.AddImage("nametable1", ppu._PatternTable[1]);
	render.DrawObject("screen",0,0, 2, 2);
	render.DrawObject("nametable0", 512, 0, 2, 2);
	render.DrawObject("nametable1", 512 + 256, 0, 2, 2);
	_cpu.Reset();
	while (!glfwWindowShouldClose(render._window)) {
		render.Resize(960, 720);
		int c;
		while (_cpu._total_cycles < 29780) {
			c = _cpu._total_cycles;
			_cpu.Execute(true);
			int diff = _cpu._total_cycles - c;
			for (int i = 0; i < diff * 3; i++) {
				ppu.Clock();
			}
			
		}
		ppu.OnUpdate();
		_cpu._total_cycles = 0;
		render.UpdateGL(0);



	}
	glfwTerminate();
}

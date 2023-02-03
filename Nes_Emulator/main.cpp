
#include <iostream>
#include "Renderer.h"
#include "EMU_6502.h"
#include "Cartridge.h"

int main(void)
{
	Graphics::Renderer render;
	Graphics::Image* image = new Graphics::Image();

	Hardware::Bus bus;
	Hardware::EMU_6502 _cpu;
	Hardware::Cartridge rom("C:\\Users\\naomi\\Downloads\\SuperMarioBros.nes", &bus);
	//load test binary into ram
	FILE* file;
	fopen_s(&file, "C:\\Users\\naomi\\Downloads\\TestPPUWrite.bin", "r");
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	fread(bus.internalRam, 1, size, file);

	_cpu.StartCPU(&bus);



	image->LoadImageData("C:\\Users\\naomi\\Downloads\\TestTreeImage.png");
	
	render.Resize(1920, 1080);
	render.AddImage("Tree", image);
	render.DrawObject("Tree", 19,19);
	render.StartPPU();
}

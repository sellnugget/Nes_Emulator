
#include <iostream>
#include "Renderer.h"
int main(void)
{
	Graphics::Renderer render;
	Graphics::Image* image = new Graphics::Image();
	image->LoadImageData("C:\\Users\\naomi\\Downloads\\TestTreeImage.png");
	
	render.Resize(1920, 1080);
	render.AddImage("Tree", image);
	render.DrawObject("Tree", 19,19);
	render.StartPPU();
}

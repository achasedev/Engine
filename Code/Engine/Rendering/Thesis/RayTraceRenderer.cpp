#include "Engine/Core/Rgba.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Rendering/Thesis/RayTraceRenderer.hpp"

#include <cstdlib>

//#include "ThirdParty/stb/stb_image.h"
#include "ThirdParty/stb/stb_image_write.h"

// Console commands
void Command_Draw(Command& cmd);

RayTraceRenderer* RayTraceRenderer::s_instance = nullptr;

RayTraceRenderer::RayTraceRenderer()
	: m_pixelDimensions(1920, 1080)
{
	m_colorData = (Rgba*)malloc(sizeof(Rgba) * 1920 * 1080);
}

void RayTraceRenderer::RegisterConsoleCommands()
{
	Command::Register("ray_draw", "Creates a ray traced image", Command_Draw);
}

RayTraceRenderer::~RayTraceRenderer()
{

}

void RayTraceRenderer::Draw()
{
	ProfileScoped test("RayTraceRenderer::Draw"); UNUSED(test);

	for (int y = 0; y < m_pixelDimensions.y; ++y)
	{
		for (int x = 0; x < m_pixelDimensions.x; ++x)
		{
			int index = y * m_pixelDimensions.x + x;
			m_colorData[index] = Rgba::GetRandomColor();
		}
	}
}

void RayTraceRenderer::WriteToFile(const char* filename)
{
	// Write the image to file (image will be upsidedown, so flip on write)
	//stbi_flip_vertically_on_write(1);

	ProfileScoped test("RayTraceRenderer::WriteToFile"); UNUSED(test);

	stbi_write_png(filename, m_pixelDimensions.x, m_pixelDimensions.y, 4, m_colorData, 0);
}

void RayTraceRenderer::Initialize()
{
	s_instance = new RayTraceRenderer();

	RegisterConsoleCommands();
}

void RayTraceRenderer::ShutDown()
{
	delete s_instance;
	s_instance = nullptr;
}

RayTraceRenderer* RayTraceRenderer::GetInstance()
{
	return s_instance;
}


//////////////////////////////////////////////////////////////////////////
// CONSOLE COMMANDS
//////////////////////////////////////////////////////////////////////////

void Command_Draw(Command& cmd)
{
	std::string name = "Data/Images/Test.png";
	cmd.GetParam("n", name, &name);

	RayTraceRenderer::GetInstance()->Draw();
	ConsolePrintf(Rgba::GREEN, "Draw completed");

	RayTraceRenderer::GetInstance()->WriteToFile(name.c_str());
	ConsolePrintf(Rgba::GREEN, "Write completed");
}

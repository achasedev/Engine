#include "Engine/Rendering/Resources/Texture3D.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Core/Rgba.hpp"

Texture3D::Texture3D()
{
}

Texture3D::~Texture3D()
{
}

bool Texture3D::CreateFromFile(const char* filename)
{
	File* file = new File();
	bool opened = file->Open(filename, "r");

	if (!opened)
	{
		return false;
	}

	file->LoadFileToMemory();

	FileWriteFromBuffer("Data/test.qef", file->GetData(), file->GetSize());

	std::string currLine;

	// "Qubicle Exchange Format"
	file->GetNextLine(currLine);

	if (currLine != "Qubicle Exchange Format")
	{
		ERROR_RECOVERABLE(Stringf("Error: Texture3D::CreateFromFile() only supports QEF files"));
		return false;
	}

	// Version number
	file->GetNextLine(currLine);

	// Website
	file->GetNextLine(currLine);

	// Dimensions
	file->GetNextLine(currLine);
	
	if (!SetFromText(currLine, m_dimensions))
	{
		ERROR_RECOVERABLE(Stringf("Error: Texture3D::CreateFromFile() couldn't get the dimensions of the texture."));
		return false;
	}

	// Number of colors
	file->GetNextLine(currLine);

	int numColors;
	SetFromText(currLine, numColors);

	Rgba* colorPallette = (Rgba*)malloc(sizeof(Rgba) * numColors);
	memset(colorPallette, 0, sizeof(Rgba) * numColors);

	// Get the colors
	for (int i = 0; i < numColors; ++i)
	{
		file->GetNextLine(currLine);
		SetFromText(currLine, colorPallette[i]);
	}

	// Set up the texture colors
	if (m_colorData != nullptr)
	{
		free(m_colorData);
	}

	m_colorData = (Rgba*)malloc(sizeof(Rgba) * m_dimensions.x * m_dimensions.y * m_dimensions.z);
	memset(m_colorData, 0, sizeof(Rgba) * m_dimensions.x * m_dimensions.y * m_dimensions.z);

	// Now get all the voxel colors
	while (!file->IsAtEndOfFile())
	{
		int lineNumber = file->GetNextLine(currLine);

		if (currLine.size() == 0)
		{
			break;
		}

		std::vector<std::string> voxelTokens = Tokenize(currLine, ' ');

		// Get the voxel coords
		int xCoord = StringToInt(voxelTokens[0]);
		int yCoord = StringToInt(voxelTokens[1]);
		int zCoord = StringToInt(voxelTokens[2]);

		int index = yCoord * (m_dimensions.x * m_dimensions.z) + zCoord * m_dimensions.x + xCoord;

		int colorIndex = StringToInt(voxelTokens[3]);

		m_colorData[index] = colorPallette[colorIndex];
	}

	// Done!
	free(colorPallette);
}

Texture3D* Texture3D::Copy() const
{
	Texture3D* newTexture = new Texture3D();
	newTexture->m_dimensions = m_dimensions;

	size_t byteSize = sizeof(Rgba) * m_dimensions.x * m_dimensions.y * m_dimensions.z;
	newTexture->m_colorData = (Rgba*)malloc(byteSize);

	memcpy(newTexture->m_colorData, m_colorData, byteSize);

	return newTexture;
}

Rgba* Texture3D::GetColorData() const
{
	return m_colorData;
}

Rgba Texture3D::GetColorAtCoords(const IntVector3& coords) const
{
	int index = coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;
	return m_colorData[index];
}

IntVector3 Texture3D::GetDimensions() const
{
	return m_dimensions;
}

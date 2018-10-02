#include "Engine/Rendering/Resources/VoxelTexture.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Core/Rgba.hpp"

VoxelTexture::VoxelTexture()
{
}

VoxelTexture::~VoxelTexture()
{
}

bool VoxelTexture::CreateFromFile(const char* filename)
{
	File* file = new File();
	bool opened = file->Open(filename, "r");

	if (!opened)
	{
		return false;
	}

	file->LoadFileToMemory();

	FileWriteFromBuffer("Data/test.qef", file->GetData(), (int) file->GetSize());

	std::string currLine;

	// "Qubicle Exchange Format"
	file->GetNextLine(currLine);

	if (currLine != "Qubicle Exchange Format")
	{
		ERROR_RECOVERABLE(Stringf("Error: VoxelTexture::CreateFromFile() only supports QEF files"));
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
		ERROR_RECOVERABLE(Stringf("Error: VoxelTexture::CreateFromFile() couldn't get the dimensions of the texture."));
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
		file->GetNextLine(currLine);

		if (currLine.size() == 0)
		{
			break;
		}

		std::vector<std::string> voxelTokens = Tokenize(currLine, ' ');

		// Get the voxel coords
		int xCoord = StringToInt(voxelTokens[0]);

		// *Flip from right handed to left handed basis
		xCoord = m_dimensions.x - xCoord - 1;

		int yCoord = StringToInt(voxelTokens[1]);
		int zCoord = StringToInt(voxelTokens[2]);

		int index = yCoord * (m_dimensions.x * m_dimensions.z) + zCoord * m_dimensions.x + xCoord;

		int colorIndex = StringToInt(voxelTokens[3]);

		m_colorData[index] = colorPallette[colorIndex];
	}

	// Done!
	free(colorPallette);

	return true;
}

bool VoxelTexture::CreateFromColorStream(const Rgba* colors, const IntVector3& dimensions)
{
	if (m_colorData != nullptr)
	{
		free(m_colorData);
	}

	int numVoxels = dimensions.x * dimensions.y * dimensions.z;

	m_colorData = (Rgba*)malloc(sizeof(Rgba) * numVoxels);
	memcpy(m_colorData, colors, numVoxels * sizeof(Rgba));

	m_dimensions = dimensions;

	return true;
}

VoxelTexture* VoxelTexture::Clone() const
{
	VoxelTexture* newTexture = new VoxelTexture();
	newTexture->m_dimensions = m_dimensions;

	size_t byteSize = sizeof(Rgba) * m_dimensions.x * m_dimensions.y * m_dimensions.z;
	newTexture->m_colorData = (Rgba*)malloc(byteSize);

	memcpy(newTexture->m_colorData, m_colorData, byteSize);

	return newTexture;
}

void VoxelTexture::SetColorAtIndex(unsigned int index, const Rgba& color)
{
	m_colorData[index] = color;
}

Rgba VoxelTexture::GetColorAtCoords(const IntVector3& coords) const
{
	int index = coords.y * (m_dimensions.x * m_dimensions.z) + coords.z * m_dimensions.x + coords.x;
	return m_colorData[index];
}

Rgba VoxelTexture::GetColorAtIndex(unsigned int index) const
{
	return m_colorData[index];
}

IntVector3 VoxelTexture::GetDimensions() const
{
	return m_dimensions;
}

unsigned int VoxelTexture::GetVoxelCount() const
{
	return m_dimensions.x * m_dimensions.y * m_dimensions.z;
}

/************************************************************************/
/* File: TextureCube.cpp
/* Author:
/* Date: May 7th, 2017
/* Description: Implementation of the TextureCube class
/************************************************************************/
#include "Engine/Core/Image.hpp"
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Renderer/TextureCube.hpp"
#include "Engine/Renderer/glFunctions.hpp"

// Texture Data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// |-------------------------------------------------------------------------------------------------------| //
// |      Image Format       |    Internal Format      |        Channels         |      Pixel Layout       | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |     R8 (1 channel)      |          GL_R8          |         GL_RED          |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RG8 (2 Channel)      |         GL_RG8          |         GL_RG           |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RGB8 (3 Channel)     |         GL_RGB8         |         GL_RGB          |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |    RGBA8 (4 Channel)    |        GL_RGBA8         |         GL_RGBA         |     GL_UNSIGNED_BYTE    | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
// |D24S8 (Depth24/Stencil8) |   GL_DEPTH24_STENCIL8   |     GL_DEPTH_STENCIL    |   GL_UNSIGNED_INT_24_8  | //
// |-------------------------|-------------------------|-------------------------|-------------------------| //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------------------------
// Constructor - just sets the type enum
//
TextureCube::TextureCube()
{
	m_textureType = TEXTURE_TYPE_CUBE_MAP;
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
TextureCube::~TextureCube()
{
}


//-----------------------------------------------------------------------------------------------
// Loads the cube from a file, overrides the base Texture::CreateFromFile() to avoid flipping the image
//
void TextureCube::CreateFromFile(const std::string& filename)
{
	Image* loadedImage = AssetDB::CreateOrGetImage(filename);

	// Construct the Texture from the image
	CreateFromImage(loadedImage);

	// Free up the image data
	delete loadedImage;
}


//-----------------------------------------------------------------------------------------------
// Initializes the TextureCube from a tiled image
//
void TextureCube::CreateFromImage(const Image* image)
{
	if (m_textureHandle == NULL)
	{
		glGenTextures(1, &m_textureHandle);
		GL_CHECK_ERROR();
	}

	unsigned int totalWidth = image->GetDimensions().x;
	unsigned int tileSize = (totalWidth / 4);
	m_dimensions = image->GetDimensions();

	// Set the format based on the image components
	m_textureFormat = static_cast<TextureFormat>(image->GetNumComponentsPerTexel() - 1);

	GLenum internalFormat = ToGLInternalFormat(m_textureFormat);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureHandle);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, internalFormat, tileSize, tileSize ); 
	GL_CHECK_ERROR();

	glPixelStorei(GL_UNPACK_ROW_LENGTH, totalWidth);
	GL_CHECK_ERROR();

	// Bind the images to the sides
	BindImageToSide(TEXCUBE_RIGHT,	*image, tileSize * 2, tileSize * 1);
	BindImageToSide(TEXCUBE_LEFT,	*image, tileSize * 0, tileSize * 1);
	BindImageToSide(TEXCUBE_TOP,	*image, tileSize * 1, tileSize * 0); 
	BindImageToSide(TEXCUBE_BOTTOM, *image, tileSize * 1, tileSize * 2); 
	BindImageToSide(TEXCUBE_FRONT,	*image, tileSize * 1, tileSize * 1); 
	BindImageToSide(TEXCUBE_BACK,	*image, tileSize * 3, tileSize * 1); 

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}


//-----------------------------------------------------------------------------------------------
// Binds the given side of the texture cube to the part of the image specified by xOffset and yOffset
//
void TextureCube::BindImageToSide(TexCubeSide side, const Image& image, unsigned int xOffset, unsigned int yOffset)
{
	// Get the start of the image data at the current tile offset
	unsigned int byteOffset = image.GetNumComponentsPerTexel() * (image.GetDimensions().x * yOffset + xOffset);
	unsigned char* imageData = (unsigned char*)image.GetImageData();
	imageData = &imageData[byteOffset];

	unsigned int tileSize = (unsigned int) ((float) (m_dimensions.x * 0.25f));
	GLenum channel = ToGLChannel(m_textureFormat);
	GLenum pixelLayout = ToGLPixelLayout(m_textureFormat);

	glTexSubImage2D(
		ToGLType(side),
		0,
		0, 0,
		tileSize, tileSize,
		channel,
		pixelLayout,
		imageData);

	GL_CHECK_ERROR();
}


// FOR REFERENCE
//------------------------------------------------------------------------
// make a cube map from 6 images;
//bool TextureCube::make_from_images( Image const *images )
//{
//#if defined(_DEBUG)
//	for (uint i = 0; i < 6; ++i) {
//		ASSERT( images[i].get_width() == images[i].get_height() ); 
//		ASSERT( images[i].get_width() == images[0].get_width() ); 
//		ASSERT( images[i].get_format() == images[0].get_format() );
//	}
//#endif   
//
//	if (m_handle == NULL) {
//		glGenTextures( 1, &m_handle ); 
//		ASSERT_RETURN_VALUE( is_valid(), false ); 
//	}
//
//	m_size = images[0].get_width(); 
//	m_format = TextureFormatFromImageFormat( images[0].get_format() ); 
//
//	GLenum internal_format; 
//	GLenum channels; 
//	GLenum pixel_layout; 
//	TextureGetInternalFormat( &internal_format, &channels, &pixel_layout, m_format ); 
//
//	// bind it; 
//	glBindTexture( GL_TEXTURE_CUBE_MAP, m_handle ); 
//
//	glTexStorage2D( GL_TEXTURE_CUBE_MAP, 1, internal_format, m_size, m_size ); 
//	glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 ); 
//
//	// bind the image to the side; 
//	FlipAndBindImage( TEXCUBE_RIGHT,  images[0], channels, pixel_layout ); 
//	FlipAndBindImage( TEXCUBE_LEFT,   images[1], channels, pixel_layout ); 
//	FlipAndBindImage( TEXCUBE_TOP,    images[2], channels, pixel_layout ); 
//	FlipAndBindImage( TEXCUBE_BOTTOM, images[3], channels, pixel_layout ); 
//	FlipAndBindImage( TEXCUBE_FRONT,  images[4], channels, pixel_layout ); 
//	FlipAndBindImage( TEXCUBE_BACK,   images[5], channels, pixel_layout ); 
//
//	return GLSucceeded(); 
//}
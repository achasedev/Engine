#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
// To disable audio entirely (and remove requirement for fmod.dll / fmod64.dll) for any game,
//	#define ENGINE_DISABLE_AUDIO in your game's Code/Game/EngineBuildPreferences.hpp file.
//
// Note that this #include is an exception to the rule "engine code doesn't know about game code".
//	Purpose: Each game can now direct the engine via #defines to build differently for that game.
//	Downside: ALL games must now have this Code/Game/EngineBuildPreferences.hpp file.
//
#include "Game/Framework/EngineBuildPreferences.hpp"
#if !defined( ENGINE_DISABLE_AUDIO )


//-----------------------------------------------------------------------------------------------
// Link in the appropriate FMOD static library (32-bit or 64-bit)
//
#if defined( _WIN64 )
#pragma comment( lib, "ThirdParty/fmod/fmod64_vc.lib" )
#else
#pragma comment( lib, "ThirdParty/fmod/fmod_vc.lib" )
#endif

// Singleton AudioSystem instance
AudioSystem* AudioSystem::s_instance = nullptr;


// Console Commands
void Command_ShowFFT(Command& cmd)
{
	UNUSED(cmd);

	AudioSystem::SetShouldRender(true);
}

void Command_HideFFT(Command& cmd)
{
	UNUSED(cmd);

	AudioSystem::SetShouldRender(false);
}

void Command_SetYMaxValue(Command& cmd)
{
	float newValue = 0.f;
	bool specified = cmd.GetParam("v", newValue);

	if (!specified)
	{
		ConsoleErrorf("No value specified with -v flag");
		return;
	}

	if (newValue <= 0.f || newValue > 2.0f)
	{
		ConsoleErrorf("Value must be between (0.0, 2.0]");
		return;
	}

	AudioSystem::GetInstance()->SetFFTMaxYValue(newValue);
	ConsolePrintf("Set the FFT max Y value to %.2f", newValue);
}

void Command_SetXMaxValue(Command& cmd)
{
	float maxX = 0.f;
	bool specified = cmd.GetParam("v", maxX);

	if (!specified)
	{
		ConsoleErrorf("No value specified with -v flag");
		return;
	}

	if (maxX <= 0.f || maxX > 20000.f) // Just not too high
	{
		ConsoleErrorf("Value must be between (0.0, 20000.0]");
		return;
	}

	AudioSystem::GetInstance()->SetFFTMaxXValue(maxX);
	ConsolePrintf("Set the FFT max X value to %.2f", maxX);
}

void Command_SetWindowSize(Command& cmd)
{
	int numWindowSegments;
	bool success = cmd.GetParam("n", numWindowSegments);

	if (!success)
	{
		ConsoleErrorf("Window size must be specified with -n flag and be a power of two <= 4096");
		return;
	}

	AudioSystem::GetInstance()->SetWindowSize(numWindowSegments);
	ConsolePrintf(Rgba::GREEN, "Set window size to %i", numWindowSegments);
}


//-----------------------------------------------------------------------------------------------
// Initialization code based on example from "FMOD Studio Programmers API for Windows"
//
AudioSystem::AudioSystem()
	: m_fmodSystem( nullptr )
{
	FMOD_RESULT result;
	result = FMOD::System_Create( &m_fmodSystem );
	ValidateResult( result );

	result = m_fmodSystem->init( 512, FMOD_INIT_NORMAL, nullptr );
	ValidateResult( result );
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::InitializeConsoleCommands()
{
#ifdef FFT_ENABLED
	Command::Register("show_fft", "Shows the FFT audio graph", Command_ShowFFT);
	Command::Register("hide_fft", "Hides the FFT audio graph", Command_HideFFT);
	Command::Register("set_fft_max_x", "Sets the max X value on the FFT graph", Command_SetXMaxValue);
	Command::Register("set_fft_max_y", "Sets the max Y value on the FFT graph", Command_SetYMaxValue);
	Command::Register("set_window_size", "Sets the window size for the FFT function", Command_SetWindowSize);
#endif
}


//-----------------------------------------------------------------------------------------------
// Calculates all the bound locations for rendering the graph
//
void AudioSystem::SetupUIBounds()
{
	Renderer* renderer = Renderer::GetInstance();
	AABB2 uiBounds = renderer->GetUIBounds();

	m_totalBounds = uiBounds;
	m_totalBounds.mins = m_screenEdgePadding;
	m_totalBounds.maxs = Vector2(uiBounds.maxs.x - m_screenEdgePadding.x, m_totalBounds.mins.y + m_graphHeight);

	// Max Value
	m_maxValueBounds.mins = m_totalBounds.mins;
	m_maxValueBounds.maxs = Vector2(m_maxValueBounds.mins.x + m_totalBounds.GetDimensions().x * 0.08f, m_totalBounds.maxs.y);

	// Axes
	m_yAxisBounds.mins = m_maxValueBounds.GetBottomRight();
	m_yAxisBounds.maxs = Vector2(m_yAxisBounds.mins.x + m_totalBounds.GetDimensions().x * 0.05f, m_totalBounds.maxs.y);

	m_xAxisBounds.mins = m_yAxisBounds.GetBottomRight();
	m_xAxisBounds.maxs = Vector2(m_totalBounds.maxs.x - m_totalBounds.GetDimensions().x * 0.05f, m_xAxisBounds.mins.y + 2.f * m_fontHeight);

	// Graph
	m_graphBounds = AABB2(m_xAxisBounds.GetTopLeft(), Vector2(m_xAxisBounds.maxs.x, m_totalBounds.maxs.y));
	m_maxValueBounds.mins.y = m_graphBounds.mins.y;

	// Heading
	m_headingBounds.mins = m_maxValueBounds.GetTopLeft();
	m_headingBounds.maxs = uiBounds.GetTopRight();
	m_headingBounds.maxs -= Vector2(m_screenEdgePadding.x); // Only pad on y by x padding, so it's not so much

	// Right Side Panel
	m_rightSidePanel.mins = m_xAxisBounds.GetBottomRight();
	m_rightSidePanel.maxs = m_totalBounds.GetTopRight();
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::UpdateFFTGraph()
{
	FMOD::ChannelGroup* masterChannelGroup = nullptr;
	m_fmodSystem->getMasterChannelGroup(&masterChannelGroup);

	// Get the fft data
	void* spectrumData = nullptr;
	m_fftDSP->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&spectrumData, 0, 0, 0);
	m_spectrumData = (FMOD_DSP_PARAMETER_FFT*)spectrumData;

	Renderer* renderer = Renderer::GetInstance();

	if (m_spectrumData != nullptr)
	{
		float boxWidth = m_graphBounds.GetDimensions().x / (float)m_segmentsToDisplay;
		AABB2 baseBoxBounds = AABB2(m_graphBounds.mins, m_graphBounds.mins + Vector2(boxWidth, m_graphBounds.GetDimensions().y));

		MeshBuilder mb;
		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

		m_maxValueLastFrame = 0.f;

		float oneOverMaxYValue = 1.0f / m_fftMaxYAxis;
		for (unsigned int i = 0; i < m_segmentsToDisplay; ++i)
		{
			// Get the sum of all channels
			float value = 0.f;
			for (int j = 0; j < m_spectrumData->numchannels; ++j)
			{
				value += m_spectrumData->spectrum[j][i];
			}

			m_maxValueLastFrame = MaxFloat(value, m_maxValueLastFrame);

			AABB2 currBoxBounds = baseBoxBounds;
			currBoxBounds.maxs.y = oneOverMaxYValue * value * baseBoxBounds.GetDimensions().y + baseBoxBounds.mins.y;

			// Texture coords
			AABB2 texCoords;
			texCoords.mins = Vector2::ZERO;
			texCoords.maxs = Vector2(1.0f, value);

			mb.Push2DQuad(currBoxBounds, texCoords);

			baseBoxBounds.Translate(Vector2(baseBoxBounds.GetDimensions().x, 0.f));
		}

		mb.FinishBuilding();
		mb.UpdateMesh(m_barMesh);

		mb.Clear();
		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

		// Push in background first to avoid overdrawing
		mb.Push2DQuad(m_totalBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_backgroundColor);

		// Push a grid
		int baseThickness = 1;

		float width = m_graphBounds.GetDimensions().x / (float)m_gridSegmentCount.x;
		for (int i = 0; i <= m_gridSegmentCount.x; ++i)
		{
			int lineThickness = baseThickness;
			if (i % 2 == 0)
			{
				lineThickness *= 2;
			}

			float x = m_graphBounds.mins.x + i * width;

			Vector2 min = Vector2(x - lineThickness, m_graphBounds.mins.y);
			Vector2 max = Vector2(x + lineThickness, m_graphBounds.maxs.y);

			AABB2 line = AABB2(min, max);

			mb.Push2DQuad(line, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
		}

		float height = m_graphBounds.GetDimensions().y / (float)(m_gridSegmentCount.y);
		for (int i = 0; i <= m_gridSegmentCount.y; ++i)
		{
			float thickness = 1;
			if (i % 2 == 0)
			{
				thickness *= 2;
			}

			float y = m_graphBounds.mins.y + i * height;

			Vector2 min = Vector2(m_graphBounds.mins.x, y - thickness);
			Vector2 max = Vector2(m_graphBounds.maxs.x, y + thickness);

			AABB2 line = AABB2(min, max);

			mb.Push2DQuad(line, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
		}

		// Line for max value
		AABB2 maxValueLine;
		maxValueLine.mins = Vector2(m_graphBounds.mins.x, m_graphBounds.GetDimensions().y * m_maxValueLastFrame * oneOverMaxYValue + m_graphBounds.mins.y);
		maxValueLine.maxs = Vector2(m_graphBounds.maxs.x, m_graphBounds.GetDimensions().y * m_maxValueLastFrame * oneOverMaxYValue + m_graphBounds.mins.y);
		maxValueLine.AddPaddingToSides(0.f, baseThickness);

		mb.Push2DQuad(maxValueLine, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(255, 255, 0, 100));

		// Push the background panels
		mb.Push2DQuad(m_headingBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
		mb.Push2DQuad(m_yAxisBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
		mb.Push2DQuad(m_xAxisBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
		mb.Push2DQuad(m_rightSidePanel, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);

		mb.FinishBuilding();
		mb.UpdateMesh(m_gridMesh);
	}
}


//-----------------------------------------------------------------------------------------------
AudioSystem::~AudioSystem()
{
	FMOD_RESULT result = m_fmodSystem->release();
	ValidateResult( result );

	m_fmodSystem = nullptr; // #Fixme: do we delete/free the object also, or just do this?
}


#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
//-----------------------------------------------------------------------------------------------
void AudioSystem::Initialize()
{
	GUARANTEE_OR_DIE(s_instance == nullptr, "Error: AudioSystem::Initialize() called with an existing instance.");
	s_instance = new AudioSystem();

#ifdef FFT_ENABLED
	s_instance->AddFFTDSPToMasterChannel();
	s_instance->SetupUIBounds();
#endif

	InitializeConsoleCommands();
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::Shutdown()
{
	if (s_instance != nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}

//-----------------------------------------------------------------------------------------------
AudioSystem* AudioSystem::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::BeginFrame()
{
	m_fmodSystem->update();

	if (m_fftDSP != nullptr)
	{
		UpdateFFTGraph();
	}
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	int windowType;
	m_fftDSP->getParameterInt(FMOD_DSP_FFT_WINDOWTYPE, &windowType, 0, 0);
	
	int oldType = windowType;

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_LEFT_ARROW))
	{
		windowType--;
	}
	
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_RIGHT_ARROW))
	{
		windowType++;
	}

	// There are only 6 window types, 0 through 5
	if (windowType > 5)
	{
		windowType = 0;
	}
	else if (windowType < 0)
	{
		windowType = 5;
	}

	if (oldType != windowType)
	{
		SetFFTWindowType((FMOD_DSP_FFT_WINDOW)windowType);
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the string version of the given window type enumeration
//
std::string GetStringForWindowType(FMOD_DSP_FFT_WINDOW windowType)
{
	switch (windowType)
	{
	case FMOD_DSP_FFT_WINDOW_RECT:
		return "Rectangle";
		break;
	case FMOD_DSP_FFT_WINDOW_TRIANGLE:
		return "Triangle";
		break;
	case FMOD_DSP_FFT_WINDOW_HAMMING:
		return "Hamming";
		break;
	case FMOD_DSP_FFT_WINDOW_HANNING:
		return "Hanning";
		break;
	case FMOD_DSP_FFT_WINDOW_BLACKMAN:
		return "Blackman";
		break;
	case FMOD_DSP_FFT_WINDOW_BLACKMANHARRIS:
		return "Blackman-Harris";
		break;
	default:
		break;
	}

	return "";
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::RenderFFTGraph() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	AABB2 bounds = renderer->GetUIBounds();
	BitmapFont* font = AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	renderer->DrawMeshWithMaterial(&m_gridMesh, AssetDB::GetSharedMaterial("UI"));
	renderer->DrawMeshWithMaterial(&m_barMesh, AssetDB::GetSharedMaterial("Gradient"));
	
	std::string text = Stringf("Number of Channels: %i\n", m_spectrumData->numchannels);
	text += Stringf("Number of intervals displayed: %i (out of %i)\n", m_segmentsToDisplay, m_numWindowSegments);
	text += Stringf("Frequency resolution: %f hz\n", m_nyquistFreq / (float)m_numWindowSegments);
	text += Stringf("Sample Rate: %.0f hz\n", m_sampleRate);

	int windowType;
	m_fftDSP->getParameterInt(FMOD_DSP_FFT_WINDOWTYPE, &windowType, 0, 0);

	std::string windowTypeText = GetStringForWindowType((FMOD_DSP_FFT_WINDOW)windowType);

	text += Stringf("[Left, Right] Window Type: %s", windowTypeText.c_str());

	renderer->DrawTextInBox2D(text, m_headingBounds, Vector2::ZERO, m_fontHeight, TEXT_DRAW_SHRINK_TO_FIT, font, m_fontColor);

	// Draw x axis labels
	float maxFrequencyOnGraph = m_nyquistFreq * ((float) m_segmentsToDisplay / (float) m_numWindowSegments);

	float graphWidth = m_graphBounds.GetDimensions().x;
	float axisFontHeight = m_fontHeight * 0.5f;
	Vector2 xTextPos = Vector2(0.f, m_graphBounds.mins.y - axisFontHeight - 10.f);

	for (int i = 0; i <= m_gridSegmentCount.x; ++i)
	{
		float normalizedOffsetIntoXRange = ((float) i / (float) m_gridSegmentCount.x);
		float frequencyValue = normalizedOffsetIntoXRange * maxFrequencyOnGraph;
		std::string frequencyText = Stringf("%.0f", frequencyValue);

		float textWidth = font->GetStringWidth(frequencyText, axisFontHeight, 1.0f);

		xTextPos.x = m_graphBounds.mins.x + graphWidth * ((float)i / (float)m_gridSegmentCount.x) - (0.5f * textWidth);

		renderer->DrawText2D(Stringf("%.0f", frequencyValue), xTextPos, axisFontHeight, font, m_fontColor);
	}

	// Draw y axis labels
	float graphHeight = m_graphBounds.GetDimensions().y;

	for (int i = 0; i <= m_gridSegmentCount.y; ++i)
	{
		float value = (float)i / (float)m_gridSegmentCount.y;
		std::string labelText = Stringf("%.2f", value);
		float textWidth = font->GetStringWidth(labelText, axisFontHeight, 1.0f);

		Vector2 yTextPos;
		yTextPos.x = m_yAxisBounds.maxs.x - textWidth - 10.f;
		yTextPos.y = m_graphBounds.mins.y + (value * graphHeight) - (0.5f * axisFontHeight);
		renderer->DrawText2D(Stringf("%.2f", m_fftMaxYAxis * value), yTextPos, axisFontHeight, font, m_fontColor);
	}

	renderer->DrawTextInBox2D("Frequency (hz)", m_xAxisBounds, Vector2(0.5f, 1.f), m_fontHeight, TEXT_DRAW_OVERRUN, font, m_fontColor);

	float yPosition = RangeMapFloat(m_maxValueLastFrame, 0.f, m_fftMaxYAxis, 1.f, 0.0f);
	renderer->DrawTextInBox2D(Stringf("%.3f", m_maxValueLastFrame), m_maxValueBounds, Vector2(0.f, yPosition), m_fontHeight, TEXT_DRAW_SHRINK_TO_FIT, font, m_fontColor);
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::EndFrame()
{
}


//-----------------------------------------------------------------------------------------------
SoundID AudioSystem::CreateOrGetSound( const std::string& soundFilePath )
{
	std::map< std::string, SoundID >::iterator found = m_registeredSoundIDs.find( soundFilePath );
	if( found != m_registeredSoundIDs.end() )
	{
		return found->second;
	}
	else
	{
		FMOD::Sound* newSound = nullptr;
		m_fmodSystem->createSound( soundFilePath.c_str(), FMOD_DEFAULT, nullptr, &newSound );
		if( newSound )
		{
			SoundID newSoundID = m_registeredSounds.size();
			m_registeredSoundIDs[ soundFilePath ] = newSoundID;
			m_registeredSounds.push_back( newSound );
			return newSoundID;
		}
	}

	return MISSING_SOUND_ID;
}


//-----------------------------------------------------------------------------------------------
SoundPlaybackID AudioSystem::PlaySound( SoundID soundID, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	size_t numSounds = m_registeredSounds.size();
	if( soundID < 0 || soundID >= numSounds )
		return MISSING_SOUND_ID;

	FMOD::Sound* sound = m_registeredSounds[ soundID ];
	if( !sound )
		return MISSING_SOUND_ID;

	FMOD::Channel* channelAssignedToSound = nullptr;
	m_fmodSystem->playSound( sound, nullptr, isPaused, &channelAssignedToSound );
	if( channelAssignedToSound )
	{
		int loopCount = isLooped ? -1 : 0;
		unsigned int playbackMode = isLooped ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		float frequency;
		channelAssignedToSound->setMode(playbackMode);
		channelAssignedToSound->getFrequency( &frequency );
		channelAssignedToSound->setFrequency( frequency * speed );
		channelAssignedToSound->setVolume( volume );
		channelAssignedToSound->setPan( balance );
		channelAssignedToSound->setLoopCount( loopCount );
	}

	return (SoundPlaybackID) channelAssignedToSound;
}


//-----------------------------------------------------------------------------------------------
SoundPlaybackID AudioSystem::PlaySoundFromAudioGroup(const std::string& groupName, bool isLooped/*=false*/, float volume/*=1.f*/, float balance/*=0.0f*/, float speed/*=1.0f*/, bool isPaused/*=false */)
{
	bool groupExists = m_audioGroups.find(groupName) != m_audioGroups.end();

	if (!groupExists)
	{
		ERROR_AND_DIE(Stringf("Error: AudioSystem::PlaySoundFromAudioGroup received non-existant group name, name was \"%s\"", groupName.c_str()));
	}

	AudioGroup* group = m_audioGroups[groupName];
	SoundID soundToPlay = group->GetRandomSound();

	return PlaySound(soundToPlay, isLooped, volume, balance, speed, isPaused);
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::StopSound( SoundPlaybackID soundPlaybackID )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set volume on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->stop();
}


//-----------------------------------------------------------------------------------------------
// Volume is in [0,1]
//
void AudioSystem::SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set volume on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setVolume( volume );
}


//-----------------------------------------------------------------------------------------------
// Balance is in [-1,1], where 0 is L/R centered
//
void AudioSystem::SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set balance on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	channelAssignedToSound->setPan( balance );
}


//-----------------------------------------------------------------------------------------------
// Speed is frequency multiplier (1.0 == normal)
//	A speed of 2.0 gives 2x frequency, i.e. exactly one octave higher
//	A speed of 0.5 gives 1/2 frequency, i.e. exactly one octave lower
//
void AudioSystem::SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed )
{
	if( soundPlaybackID == MISSING_SOUND_ID )
	{
		ERROR_RECOVERABLE( "WARNING: attempt to set speed on missing sound playback ID!" );
		return;
	}

	FMOD::Channel* channelAssignedToSound = (FMOD::Channel*) soundPlaybackID;
	float frequency;
	FMOD::Sound* currentSound = nullptr;
	channelAssignedToSound->getCurrentSound( &currentSound );
	if( !currentSound )
		return;

	int ignored = 0;
	currentSound->getDefaults( &frequency, &ignored );
	channelAssignedToSound->setFrequency( frequency * speed );
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::ValidateResult( FMOD_RESULT result )
{
	if( result != FMOD_OK )
	{
		ERROR_RECOVERABLE( Stringf( "Engine/Audio SYSTEM ERROR: Got error result code %i - error codes listed in fmod_common.h\n", (int) result ) );
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the FMOD system itself
//
FMOD::System* AudioSystem::GetFMODSystem() const
{
	return m_fmodSystem;
}


//-----------------------------------------------------------------------------------------------
// Adds an FFT DSP to the main channel group, for equalizer rendering
//
void AudioSystem::AddFFTDSPToMasterChannel()
{
	FMOD::ChannelGroup* masterChannelGroup = nullptr;
	m_fmodSystem->getMasterChannelGroup(&masterChannelGroup);

	// Create and setup the FFT DSP, assigning it to the master channel group
	FMOD_RESULT result = m_fmodSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &m_fftDSP);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't create the DSP");

	SetFFTWindowType(FMOD_DSP_FFT_WINDOW_BLACKMANHARRIS);
	SetWindowSize(m_numWindowSegments);

	result = masterChannelGroup->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, m_fftDSP);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't ADD the DSP to the master channel group");
}


//-----------------------------------------------------------------------------------------------
// Loads the given file defining a set of audio groups and attempts to construct them
//
void AudioSystem::LoadAudioGroupFile(const std::string& filepath)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(filepath.c_str());

	if (error != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE(Stringf("Error: Couldn't load AudioGroup file \"%s\"", filepath.c_str()));
	}

	// I keep a root around just because I like having a single root element
	const XMLElement* rootElement = document.RootElement();
	
	const XMLElement* groupElement = rootElement->FirstChildElement();

	while (groupElement != nullptr)
	{
		AudioGroup* group = new AudioGroup(*groupElement);
		s_instance->m_audioGroups[group->GetName()] = group;

		groupElement = groupElement->NextSiblingElement();
	}
}



// AudioGroup class


//-----------------------------------------------------------------------------------------------
// Constructor from XML
//
AudioGroup::AudioGroup(const XMLElement& groupElement)
{
	m_name = ParseXmlAttribute(groupElement, "name", "");

	const XMLElement* clipElement = groupElement.FirstChildElement();

	AudioSystem* audio = AudioSystem::GetInstance();
	while (clipElement != nullptr)
	{
		std::string clipSourcePath = ParseXmlAttribute(*clipElement, "source", "");

		if (IsStringNullOrEmpty(clipSourcePath))
		{
			ERROR_RECOVERABLE(Stringf("Error: AudioGroup has clip with no source path specified, clip name was \"%s\"", m_name.c_str()));
			clipElement = clipElement->NextSiblingElement();
			continue;
		}

		SoundID clipID = audio->CreateOrGetSound(clipSourcePath);
		m_sounds.push_back(clipID);

		clipElement = clipElement->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the name of the group
//
std::string AudioGroup::GetName() const
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
// Returns a sound ID from the list of sounds, ensuring the same sound isn't played twice in a row
//
SoundID AudioGroup::GetRandomSound()
{
	// Ensure we have sounds to return!
	ASSERT_OR_DIE(m_sounds.size() > 0, Stringf("Error: AudioGroup::GetRandomSound called on group with no sounds, group name was \"%s\"", m_name.c_str()));

	// Iterate until we find a sound we haven't just played, or if we only have 1 sound
	bool done = false;
	SoundID soundToReturn = 0;

	while (!done)
	{
		unsigned int index = GetRandomIntLessThan((int) m_sounds.size());

		soundToReturn = m_sounds[index];

		if (soundToReturn != m_lastSoundPlayed || (int) m_sounds.size() < 2)
		{
			done = true;
		}
	}

	m_lastSoundPlayed = soundToReturn;
	return soundToReturn;
}


//-----------------------------------------------------------------------------------------------
// Updates the max X value to the one specified, clamping to avoid going over Nyquist Frequency
//
void AudioSystem::SetFFTMaxXValue(float maxFrequency)
{
	float frequencyPerSegment = m_nyquistFreq / (float)m_numWindowSegments;
	m_segmentsToDisplay = Ceiling(maxFrequency / frequencyPerSegment);

	SetupUIBounds();
}


//-----------------------------------------------------------------------------------------------
// Updates the max Y value for the graph and rebuilds the UI bounds elements
//
void AudioSystem::SetFFTMaxYValue(float newValue)
{
	m_fftMaxYAxis = newValue;

	SetupUIBounds();
}


//-----------------------------------------------------------------------------------------------
// Sets the window size for the FFT
//
void AudioSystem::SetWindowSize(int windowSize)
{
	m_numWindowSegments = windowSize;

	FMOD_RESULT result = m_fftDSP->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 2 * m_numWindowSegments);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window size parameter");
}


//-----------------------------------------------------------------------------------------------
// Sets the FFT window type for the current FFT DSP
//
void AudioSystem::SetFFTWindowType(FMOD_DSP_FFT_WINDOW windowType)
{
	FMOD_RESULT result = m_fftDSP->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, windowType);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window type parameter");
}


//-----------------------------------------------------------------------------------------------
void AudioSystem::SetShouldRender(bool newState)
{
	s_instance->m_renderFFTGraph = newState;
}


//-----------------------------------------------------------------------------------------------
bool AudioSystem::ShouldRender()
{
	return s_instance->m_renderFFTGraph;
}


#endif // !defined( ENGINE_DISABLE_AUDIO )
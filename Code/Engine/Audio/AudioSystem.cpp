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
#endif
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
	m_detailsBounds = renderer->GetUIBounds();

	m_detailsBounds.AddPaddingToSides(-20.f, 0.f);
	m_detailsBounds.mins.y += 40.f;
	m_detailsBounds.maxs.y -= m_fontHeight * 3.f;

	m_borderBounds = m_detailsBounds;

	m_borderBounds.mins.x += 100.f;
	m_detailsBounds.maxs.x = m_detailsBounds.mins.x + 100.f;

	m_graphBounds = m_borderBounds;
	m_graphBounds.AddPaddingToSides(-20.f, -20.f);
	m_detailsBounds.AddPaddingToSides(0.f, -20.f);

	if (m_spectrumData != nullptr)
	{
		m_numSegmentsToRender = m_spectrumData->length / 2 /  m_fractionOfSegmentsToShow;

		float boxWidth = m_graphBounds.GetDimensions().x / (float)m_numSegmentsToRender;
		AABB2 baseBoxBounds = AABB2(m_graphBounds.mins, m_graphBounds.mins + Vector2(boxWidth, m_graphBounds.GetDimensions().y));

		MeshBuilder mb;
		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

		m_maxValueLastFrame = 0.f;

		float oneOverNumChannels = 1.0f / (float) m_spectrumData->numchannels;
		for (unsigned int i = 0; i < m_numSegmentsToRender; ++i)
		{
			// Get the average of all channels
			float value = 0.f;
			for (int j = 0; j < m_spectrumData->numchannels; ++j)
			{
				value += m_spectrumData->spectrum[j][i];
			}

			value *= oneOverNumChannels;

			m_maxValueLastFrame = MaxFloat(value, m_maxValueLastFrame);

			AABB2 currBoxBounds = baseBoxBounds;
			currBoxBounds.maxs.y = value * baseBoxBounds.GetDimensions().y + baseBoxBounds.mins.y;

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

		// Push a grid
		int numLines = 11;
		int baseThickness = 1;

		float width = m_graphBounds.GetDimensions().x / (float)numLines;
		for (int i = 0; i <= numLines; ++i)
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

			mb.Push2DQuad(line, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::GRAY);
		}

		float height = m_graphBounds.GetDimensions().y / (float)numLines;
		for (int i = 0; i <= numLines; ++i)
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

			mb.Push2DQuad(line, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::GRAY);
		}

		// Line for max value
		AABB2 maxValueLine;
		maxValueLine.mins = Vector2(m_graphBounds.mins.x, m_detailsBounds.GetDimensions().y * m_maxValueLastFrame + m_detailsBounds.mins.y);
		maxValueLine.maxs = Vector2(m_graphBounds.maxs.x, m_detailsBounds.GetDimensions().y * m_maxValueLastFrame + m_detailsBounds.mins.y);
		maxValueLine.AddPaddingToSides(0.f, baseThickness);

		mb.Push2DQuad(maxValueLine, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(64, 64, 64, 200));

		mb.Push2DQuad(m_borderBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(0, 0, 0, 100));
		mb.Push2DQuad(m_detailsBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::GRAY);

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
void AudioSystem::RenderFFTGraph() const
{

	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	AABB2 bounds = renderer->GetUIBounds();
	BitmapFont* font = AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	renderer->DrawMeshWithMaterial(&m_gridMesh, AssetDB::GetSharedMaterial("UI"));
	renderer->DrawMeshWithMaterial(&m_barMesh, AssetDB::GetSharedMaterial("Gradient"));
	
	renderer->DrawTextInBox2D(Stringf("Number of Channels: %i", m_spectrumData->numchannels), bounds, Vector2::ZERO, m_fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -m_fontHeight));

	renderer->DrawTextInBox2D(Stringf("Number of intervals displayed: %i (out of %i)", m_numSegmentsToRender, m_numWindowSegments), bounds, Vector2::ZERO, m_fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -m_fontHeight));

	renderer->DrawTextInBox2D(Stringf("Frequency resolution: %f hz", 22050.f / (float)m_numWindowSegments), bounds, Vector2::ZERO, m_fontHeight, TEXT_DRAW_OVERRUN, font);
	bounds.Translate(Vector2(0.f, -m_fontHeight));

	renderer->DrawTextInBox2D(Stringf("%.3f", m_maxValueLastFrame), m_detailsBounds, Vector2(0.f, 1.0f - m_maxValueLastFrame), m_fontHeight, TEXT_DRAW_SHRINK_TO_FIT, font);
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

	result = m_fftDSP->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_RECT);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window type parameter");

	result = m_fftDSP->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 2 * m_numWindowSegments);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window size parameter");

	result = masterChannelGroup->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, m_fftDSP);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't ADD the DSP to the master channel group");
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

#endif // !defined( ENGINE_DISABLE_AUDIO )
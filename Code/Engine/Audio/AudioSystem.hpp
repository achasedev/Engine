#pragma once


//-----------------------------------------------------------------------------------------------
#include <map>
#include <string>
#include <vector>
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "ThirdParty/fmod/fmod.hpp"

//-----------------------------------------------------------------------------------------------
typedef size_t SoundID;
typedef size_t SoundPlaybackID;
constexpr size_t MISSING_SOUND_ID = (size_t)(-1); // for bad SoundIDs and SoundPlaybackIDs


//-----------------------------------------------------------------------------------------------
class AudioGroup;
class AudioSystem;

/////////////////////////////////////////////////////////////////////////////////////////////////
class AudioSystem
{
public:
	
	static void					Initialize();
	static void					Shutdown();
	static AudioSystem*			GetInstance();
	static void					LoadAudioGroupFile(const std::string& filepath);

	virtual void				BeginFrame();
	virtual void				RenderFFTGraph() const;
	virtual void				EndFrame();

	virtual SoundID				CreateOrGetSound( const std::string& soundFilePath );
	virtual SoundPlaybackID		PlaySound( SoundID soundID, bool isLooped=false, float volume=1.f, float balance=0.0f, float speed=1.0f, bool isPaused=false );
	virtual SoundPlaybackID		PlaySoundFromAudioGroup(const std::string& groupName, bool isLooped=false, float volume=1.f, float balance=0.0f, float speed=1.0f, bool isPaused=false );
	virtual void				StopSound( SoundPlaybackID soundPlaybackID );
	virtual void				SetSoundPlaybackVolume( SoundPlaybackID soundPlaybackID, float volume );	// volume is in [0,1]
	virtual void				SetSoundPlaybackBalance( SoundPlaybackID soundPlaybackID, float balance );	// balance is in [-1,1], where 0 is L/R centered
	virtual void				SetSoundPlaybackSpeed( SoundPlaybackID soundPlaybackID, float speed );		// speed is frequency multiplier (1.0 == normal)

	virtual void				ValidateResult( FMOD_RESULT result );

	FMOD::System*				GetFMODSystem() const;
	void						AddFFTDSPToMasterChannel();
	void						SetFFTMaxXValue(float maxFrequency);
	void						SetFFTMaxYValue(float newValue);
	static void					SetShouldRender(bool newState);
	static bool					ShouldRender();
	

protected:
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;

	std::map<std::string, AudioGroup*>	m_audioGroups;

	// For FFT rendering
	FMOD::DSP*							m_fftDSP = nullptr;
	FMOD_DSP_PARAMETER_FFT*				m_spectrumData = nullptr;
	bool								m_renderFFTGraph = false;
	mutable Mesh						m_barMesh;
	mutable Mesh						m_gridMesh;
	unsigned int						m_numWindowSegments = 4096;
	unsigned int						m_segmentsToDisplay = 512;
	float								m_maxValueLastFrame = 0.f;
	float								m_sampleRate = 48000.f;
	float								m_nyquistFreq = m_sampleRate * 0.5f;

	// UI Settings
	int									m_graphHeight = 800.f;
	float								m_fontHeight = 30.f;
	Vector2								m_screenEdgePadding = Vector2(30.f, 100.f);
	float								m_fftMaxYAxis = 1.0f;

	AABB2 m_totalBounds;
	AABB2 m_maxValueBounds;
	AABB2 m_graphBounds;
	AABB2 m_headingBounds;

	AABB2 m_xAxisBounds;
	AABB2 m_yAxisBounds;
	AABB2 m_rightSidePanel;

	Rgba m_lineAndPanelColor = Rgba(15, 60, 120, 200);
	Rgba m_backgroundColor = Rgba(0, 0, 0, 100);
	Rgba m_fontColor = Rgba(200, 200, 200, 200);

	IntVector2 m_gridSegmentCount = IntVector2(20, 10);


private:
	//-----Private Methods-----

	// Use Initialize/Destroy instead
	AudioSystem();
	virtual ~AudioSystem();
	AudioSystem(const AudioSystem& copy) = delete;

	// Console Commands
	static void InitializeConsoleCommands();

	// For FFT rendering
	void SetupUIBounds();
	void UpdateFFTGraph();


private:
	//-----Private Data-----

	static AudioSystem* s_instance;	// The singleton AudioSystem instance

};


class AudioGroup
{
public:
	//-----Public Methods-----

	AudioGroup(const XMLElement& groupElement);

	// Accessors
	std::string GetName() const;
	SoundID		GetRandomSound();


private:
	//-----Private Data-----

	std::string m_name;
	std::vector<SoundID> m_sounds;

	SoundID m_lastSoundPlayed;

};


#pragma once


//-----------------------------------------------------------------------------------------------
#include <map>
#include <string>
#include <vector>
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
	

protected:
	FMOD::System*						m_fmodSystem;
	std::map< std::string, SoundID >	m_registeredSoundIDs;
	std::vector< FMOD::Sound* >			m_registeredSounds;

	std::map<std::string, AudioGroup*>	m_audioGroups;


protected:
	//-----Protected Methods-----

	// Use Initialize/Destroy instead
	AudioSystem();
	virtual ~AudioSystem();
	AudioSystem(const AudioSystem& copy) = delete;


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


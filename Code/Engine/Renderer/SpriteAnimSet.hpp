/************************************************************************/
/* File: SpriteAnimSet.hpp
/* Author: Andrew Chase
/* Date: November 13th, 2017
/* Bugs: None
/* Description: Class to represent a collection of animations used by one
/*				Entity
/************************************************************************/
#pragma once
#include <map>
#include <vector>


class SpriteAnim;
class SpriteAnimSetDef;

class SpriteAnimSet
{
public:
	//-----Public Methods-----

	SpriteAnimSet(const SpriteAnimSetDef* setDefinition);
	~SpriteAnimSet();	// For freeing the map of animation pointers

	void Update(float deltaTime);

	// Accessors
	SpriteAnim*		GetCurrentAnimation() const;
	SpriteAnim*		GetAnimationByName(const std::string& animationName) const;

	// Mutators
	void StartAnimation(const std::string& animationName);
	void SetCurrentAnimation(const std::string& animationName);

	void AddAnimation(const std::string& animationName, SpriteAnim* animationToAdd);
	void SetCurrentTimeElapsed(float timeToSet);

	void PauseCurrentAnimation();
	void PlayCurrentAnimation();

	// Producers
	const Texture& GetTexture() const;
	AABB2 GetCurrentUVs() const;


private:
	//-----Private Data-----

	SpriteAnim* m_currentAnimation = nullptr;
	const SpriteAnimSetDef* m_setDefinition = nullptr;
	std::map<std::string, SpriteAnim*> m_animations;
};

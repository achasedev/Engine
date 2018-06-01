/************************************************************************/
/* File: HeatMap.hpp
/* Author: Andrew Chase
/* Date: November 29th, 2017
/* Bugs: None
/* Description: Class to represent a 2D grid of float values
/************************************************************************/
#pragma once
#include <vector>
#include <queue>
#include "Engine/Math/IntVector2.hpp"

class HeatMap
{
public:
	//-----Public Methods-----

	HeatMap(const IntVector2& dimensions, float initialHeatValuePerCell);

	// Mutators
	void SetHeat(const IntVector2& cellCoords, float newHeatValue);
	void AddHeat(const IntVector2& cellCoords, float addAmount);
	void RunDijkstraFromTarget(const IntVector2& targetCoords);

	// Accessors
	float GetHeat(const IntVector2& cellCoords) const;

	// Producers
	void		GetGreedyShortestPath(const IntVector2& pathStartCoords, const IntVector2& pathEndCoords, std::vector<IntVector2>& path) const;
	IntVector2	GetMinNeighborCoords(const IntVector2& currCoords) const;
	bool		AreCoordsInBounds(const IntVector2& coords) const;

	static HeatMap* ConstructDijkstraMap(const IntVector2& dimensions, IntVector2& targetTileCoords);

private:
	//-----Private Methods-----

	void UpdateNeighborDistance(const IntVector2& currCoords, const IntVector2& stepDirection, std::queue<IntVector2>& distanceQueue);


private:
	//-----Private Data-----

	std::vector<float> m_heatPerGridCell;	// Ordered from bottom-left, across rows then up
	IntVector2 m_dimensions;				// Width x height of the grid

};
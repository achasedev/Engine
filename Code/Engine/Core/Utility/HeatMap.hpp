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
	HeatMap(const HeatMap& copy);

	// Mutators
	void Clear(float clearValue);
	void SetHeat(const IntVector2& cellCoords, float newHeatValue);
	void SetHeat(int index, float newHeatValue);
	void AddHeat(const IntVector2& cellCoords, float addAmount);
	void Seed(float seedValue, const IntVector2& seedLocation);
	void Seed(float seedValue, const std::vector<IntVector2>& seedCoords);
	void SolveMapUpToDistance(float maxDistance, const HeatMap* costs = nullptr);

	// Accessors
	float GetHeat(const IntVector2& cellCoords) const;
	float GetHeat(int index) const;

	// Producers
	unsigned int GetCellCount() const;
	int			GetIndex(int x, int y) const;
	IntVector2  GetCoordsForIndex(unsigned int index) const;
	void		GetGreedyShortestPath(const IntVector2& pathStartCoords, const IntVector2& pathEndCoords, std::vector<IntVector2>& path) const;
	IntVector2	GetMinNeighborCoords(const IntVector2& currCoords) const;
	bool		AreCoordsValid(const IntVector2& coords) const;


private:
	//-----Private Methods-----

	bool UpdateCurrFromNeighbor(int currIndex, int neighborIndex, float maxDistance, const HeatMap* costs);


private:
	//-----Private Data-----

	std::vector<float> m_heatPerGridCell;	// Ordered from bottom-left, across rows then up
	IntVector2 m_dimensions;				// Width x height of the grid

	int m_maxDistanceSolved;				// Set when the HeatMap is solved up to a certain distance from seeds
	HeatMap* m_costMap;						// Costs associated with a solve
};
/************************************************************************/
/* File: HeatMap.cpp
/* Author: Andrew Chase
/* Date: November 29th, 2017
/* Bugs: None
/* Description: Implementation of the HeatMap class
/************************************************************************/
#include <queue>
#include "Engine/Core/HeatMap.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - makes the map of dimensions and initializes all cells to the initial heat value
//
HeatMap::HeatMap(const IntVector2& dimensions, float initialHeatValuePerCell)
	: m_dimensions(dimensions)
{
	int numCells = (m_dimensions.x * m_dimensions.y);

	m_heatPerGridCell.reserve(numCells);
	for (int i = 0; i < numCells; i++)
	{
		m_heatPerGridCell.push_back(initialHeatValuePerCell);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the float value at cellCoords to the newHeatValue
//
void HeatMap::SetHeat(const IntVector2& cellCoords, float newHeatValue)
{
	GUARANTEE_OR_DIE(AreCoordsInBounds(cellCoords), Stringf("Error: HeatMap::SetHeat received bad coords, coords were (%d,%d)", cellCoords.x, cellCoords.y));

	int index = (cellCoords.y * m_dimensions.x) + cellCoords.x;

	m_heatPerGridCell[index] = newHeatValue;
}


//-----------------------------------------------------------------------------------------------
// Adds addAmount to the float value at cellCoords
//
void HeatMap::AddHeat(const IntVector2& cellCoords, float addAmount)
{
	GUARANTEE_OR_DIE(AreCoordsInBounds(cellCoords), Stringf("Error: HeatMap::AddHeat received bad coords, coords were (%d,%d)", cellCoords.x, cellCoords.y));

	int index = (cellCoords.y * m_dimensions.x) + cellCoords.x;
	m_heatPerGridCell[index] += addAmount;
}


//-----------------------------------------------------------------------------------------------
// Runs the Dijkstra's algorithm on this map given the target coordinates
//
void HeatMap::RunDijkstraFromTarget(const IntVector2& targetCoords)
{
	// Initialize the target to zero
	SetHeat(targetCoords, 0.f);

	// Create the queue and place the target in it
	std::queue<IntVector2> distanceQueue;
	distanceQueue.push(targetCoords);

	// Iterate until there are no more updates
	while (static_cast<int>(distanceQueue.size()) > 0)
	{
		IntVector2 currTileCoords = distanceQueue.front();
		distanceQueue.pop();

		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_NORTH, distanceQueue);	// North
		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_SOUTH, distanceQueue);	// South
		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_EAST, distanceQueue);	// East
		UpdateNeighborDistance(currTileCoords, IntVector2::STEP_WEST, distanceQueue);	// West
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the float value at cellCoords
//
float HeatMap::GetHeat(const IntVector2& cellCoords) const
{
	GUARANTEE_OR_DIE(AreCoordsInBounds(cellCoords), Stringf("Error: HeatMap::GetHeat received bad coords, coords were (%d,%d)", cellCoords.x, cellCoords.y));

	int index = (cellCoords.y * m_dimensions.x) + cellCoords.x;
	return m_heatPerGridCell[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the greedy min-cost path from startCoords to endCoords
// Assumes a path exists
//
void HeatMap::GetGreedyShortestPath(const IntVector2& pathStartCoords, const IntVector2& pathEndCoords, std::vector<IntVector2>& path) const
{
	IntVector2 currCoords = pathStartCoords;

	while (currCoords != pathEndCoords)
	{
		path.push_back(currCoords);
		currCoords = GetMinNeighborCoords(currCoords);
	}
	
	path.push_back(pathEndCoords);
}


//-----------------------------------------------------------------------------------------------
// Returns the coords of the min neighbor to this tile
//
IntVector2 HeatMap::GetMinNeighborCoords(const IntVector2& currCoords) const
{
	// Get the distances of the neighbors

	IntVector2 northCoords	= currCoords + IntVector2::STEP_NORTH;
	IntVector2 southCoords	= currCoords + IntVector2::STEP_SOUTH;
	IntVector2 eastCoords	= currCoords + IntVector2::STEP_EAST;
	IntVector2 westCoords	= currCoords + IntVector2::STEP_WEST;

	float northDistance = GetHeat(northCoords);
	float southDistance = GetHeat(southCoords);
	float eastDistance	= GetHeat(eastCoords);
	float westDistance	= GetHeat(westCoords);

	float minDistance = MinFloat(northDistance, southDistance, eastDistance, westDistance);

	// Return the appropriate coords
	if		(minDistance == eastDistance) { return eastCoords; }
	else if (minDistance == westDistance) { return westCoords; }
	else if (minDistance == northDistance) { return northCoords; }
	else { return southCoords; }
}


//-----------------------------------------------------------------------------------------------
// Returns true if the coords are in bounds, false otherwise
bool HeatMap::AreCoordsInBounds(const IntVector2& coords) const
{
	return (coords.x >= 0 && coords.x < m_dimensions.x && coords.y >= 0 && coords.y < m_dimensions.y);
}


//-----------------------------------------------------------------------------------------------
// Runs Dijskra's algorithm to generate a distance field on this map, given the target coords
//
HeatMap* HeatMap::ConstructDijkstraMap(const IntVector2& dimensions, IntVector2& targetTileCoords)
{
	// Initialize the heat map - 9999 everywhere except 0 on the target tile
	HeatMap* distanceMap = new HeatMap(dimensions, 9999.f);
	distanceMap->RunDijkstraFromTarget(targetTileCoords);

	return distanceMap;
}


//-----------------------------------------------------------------------------------------------
// Updates the neighbor of currCoords in the stepDirection to be the min of its existing heat
// and the heat at currCoords + 1
//
void HeatMap::UpdateNeighborDistance(const IntVector2& currCoords, const IntVector2& stepDirection, std::queue<IntVector2>& distanceQueue)
{
	IntVector2 neighborCoords = currCoords + stepDirection;

	if (AreCoordsInBounds(neighborCoords))
	{
		float currHeat		= GetHeat(currCoords);
		float neighborHeat	= GetHeat(neighborCoords);

		if (currHeat + 1.f < neighborHeat)
		{
			SetHeat(neighborCoords, currHeat + 1.f);
			distanceQueue.push(neighborCoords);
		}
	}
}

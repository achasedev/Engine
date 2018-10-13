/************************************************************************/
/* File: HeatMap.cpp
/* Author: Andrew Chase
/* Date: November 29th, 2017
/* Bugs: None
/* Description: Implementation of the HeatMap class
/************************************************************************/
#include "Engine/Core/Utility/HeatMap.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - makes the map of dimensions and initializes all cells to the initial heat value
//
HeatMap::HeatMap(const IntVector2& dimensions, float initialHeatValuePerCell)
	: m_dimensions(dimensions)
{
	int numCells = (m_dimensions.x * m_dimensions.y);

	m_heatPerGridCell.resize(numCells);
	Clear(initialHeatValuePerCell);
}


//-----------------------------------------------------------------------------------------------
// Sets all cells to have the given clear value
//
void HeatMap::Clear(float clearValue)
{
	int numCells = (m_dimensions.x * m_dimensions.y);
	for (int i = 0; i < numCells; i++)
	{
		m_heatPerGridCell[i] = clearValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the float value at cellCoords to the newHeatValue
//
void HeatMap::SetHeat(const IntVector2& cellCoords, float newHeatValue)
{
	GUARANTEE_OR_DIE(AreCoordsValid(cellCoords), Stringf("Error: HeatMap::SetHeat received bad coords, coords were (%d,%d)", cellCoords.x, cellCoords.y));

	int index = (cellCoords.y * m_dimensions.x) + cellCoords.x;

	m_heatPerGridCell[index] = newHeatValue;
}


//-----------------------------------------------------------------------------------------------
// Sets the heat at the cell at index to newHeatValue
//
void HeatMap::SetHeat(int index, float newHeatValue)
{
	GUARANTEE_OR_DIE(index >= 0 && index < (int) m_heatPerGridCell.size(), Stringf("Error: HeatMap::SetHeat() received bad index, index was %i.", index));

	m_heatPerGridCell[index] = newHeatValue;
}


//-----------------------------------------------------------------------------------------------
// Adds addAmount to the float value at cellCoords
//
void HeatMap::AddHeat(const IntVector2& cellCoords, float addAmount)
{
	GUARANTEE_OR_DIE(AreCoordsValid(cellCoords), Stringf("Error: HeatMap::AddHeat received bad coords, coords were (%d,%d)", cellCoords.x, cellCoords.y));

	int index = (cellCoords.y * m_dimensions.x) + cellCoords.x;
	m_heatPerGridCell[index] += addAmount;
}


//-----------------------------------------------------------------------------------------------
// Sets the given locations to the seeded heat value
//
void HeatMap::Seed(float seedValue, const std::vector<IntVector2>& seedCoords)
{
	for (int seedIndex = 0; seedIndex < (int) seedCoords.size(); ++seedIndex)
	{
		Seed(seedValue, seedCoords[seedIndex]);
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the heat at the given location to the heat value
//
void HeatMap::Seed(float seedValue, const IntVector2& seedLocation)
{
	if (AreCoordsValid(seedLocation))
	{
		int index = (seedLocation.y * m_dimensions.x) + seedLocation.x;
		m_heatPerGridCell[index] = seedValue;
	}
}


//-----------------------------------------------------------------------------------------------
// Solves this distance map up to a set distance
//
void HeatMap::SolveMapUpToDistance(float maxDist, const HeatMap* costs/*= nullptr*/)
{
	// No cost specified, so each step costs 1 by default
	if (costs == nullptr)
	{
		costs = new HeatMap(m_dimensions, 1.f);
	}

	// Keep iterating until we no longer notice a change in the map
	bool valueChanged = true;
	while (valueChanged)
	{
		valueChanged = false;

		// Iterate across the map
		for (int xIndex = 0; xIndex < m_dimensions.x; xIndex++)
		{
			for (int yIndex = 0; yIndex < m_dimensions.y; yIndex++)
			{
				int currIndex = GetIndex(xIndex, yIndex);

				// Update the current index by checking all 4 neighbors
				int southIndex      = GetIndex(xIndex, yIndex - 1);
				bool southChanged   = UpdateCurrFromNeighbor(currIndex, southIndex, maxDist, costs);

				int northIndex      = GetIndex(xIndex, yIndex + 1);
				bool northChanged   = UpdateCurrFromNeighbor(currIndex, northIndex, maxDist, costs);

				int westIndex       = GetIndex(xIndex - 1, yIndex);
				bool westChanged    = UpdateCurrFromNeighbor(currIndex, westIndex, maxDist, costs);

				int eastIndex       = GetIndex(xIndex + 1, yIndex);
				bool eastChanged    = UpdateCurrFromNeighbor(currIndex, eastIndex, maxDist, costs);

				// Update if we changed or not
				valueChanged = valueChanged || northChanged || southChanged || eastChanged || westChanged;
			}
		}
	}
}


//----------------------------------------------------------------------------------
// Updates the distance value at currIndex based on the distance value of neighborIndex
//
bool HeatMap::UpdateCurrFromNeighbor(int currIndex, int neighborIndex, float maxDistance, const HeatMap* costs)
{
	// Bad index, just return false
	if (neighborIndex == -1) { return false; }

	float currDistance = m_heatPerGridCell[currIndex];
	float newDistance = m_heatPerGridCell[neighborIndex] + costs->GetHeat(currIndex);

	bool isWithinMaxDist = (newDistance < maxDistance);

	if (newDistance < currDistance && isWithinMaxDist)
	{
		m_heatPerGridCell[currIndex] = newDistance;
		return true;
	} 

	return false;
}


//-----------------------------------------------------------------------------------------------
// Returns the float value at cellCoords
//
float HeatMap::GetHeat(const IntVector2& cellCoords) const
{
	if (!AreCoordsValid(cellCoords))
	{
		return 9999999999.f;
	}

	//GUARANTEE_OR_DIE(AreCoordsValid(cellCoords), Stringf("Error: HeatMap::GetHeat() received bad coords, coords were (%d,%d)", cellCoords.x, cellCoords.y));

	int index = (cellCoords.y * m_dimensions.x) + cellCoords.x;
	return m_heatPerGridCell[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the heat value associated with the cell at index
//
float HeatMap::GetHeat(int index) const
{
	GUARANTEE_OR_DIE(index >= 0 && index < m_dimensions.x * m_dimensions.y, Stringf("Error: HeatMap::GetHeat() received bad index, index was %i.", index));
	return m_heatPerGridCell[index];
}


//----------------------------------------------------------------------------------
// Gets the index for the position (x, y)
// Returns -1 for coords out of bounds
//
int HeatMap::GetIndex(int x, int y) const
{
	if ((x < 0) || (x >= m_dimensions.x) || (y < 0) || (y >= m_dimensions.y)) 
	{
		return -1; 
	} 
	else 
	{
		return y * m_dimensions.x + x; 
	}
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

	// Return the appropriate coords, randomizing ties
	float tieCount = 1.f;
	IntVector2 returnValue;

	if (minDistance == eastDistance && CheckRandomChance(1.f / tieCount)) 
	{
		returnValue = eastCoords;
		tieCount += 1.0f;
	}

	if (minDistance == westDistance && CheckRandomChance(1.f / tieCount))
	{ 
		returnValue = westCoords; 
		tieCount += 1.0f;
	}
	
	if (minDistance == northDistance && CheckRandomChance(1.f / tieCount))
	{ 
		returnValue = northCoords; 
		tieCount += 1.0f;
	}
	
	if (minDistance == southDistance && CheckRandomChance(1.f / tieCount))
	{ 
		returnValue = southCoords; 
	}

	return returnValue;
}


//-----------------------------------------------------------------------------------------------
// Returns true if the coords are in bounds, false otherwise
bool HeatMap::AreCoordsValid(const IntVector2& coords) const
{
	return (coords.x >= 0 && coords.x < m_dimensions.x && coords.y >= 0 && coords.y < m_dimensions.y);
}

/************************************************************************/
/* File: CubicSpline.cpp
/* Author: Andrew Chase
/* Date: December 9th, 2017
/* Bugs: None
/* Description: Implementation of the CubicSpline class
/************************************************************************/
#include "Engine/Math/CubicSpline.hpp"


//-----------------------------------------------------------------------------------------------
// Constructs a cubic spline give the position and number of points (velocities optional)
// If velocities aren't specified, they are defaulted to (0,0)
//
CubicSpline2D::CubicSpline2D(const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray/*=nullptr */)
{
	AppendPoints(positionsArray, numPoints, velocitiesArray);
}


//-----------------------------------------------------------------------------------------------
// Adds the given point params to the ends of the list
//
void CubicSpline2D::AppendPoint(const Vector2& position, const Vector2& velocity/*=Vector2::ZERO */)
{
	m_positions.push_back(position);
	m_velocities.push_back(velocity);
}


//-----------------------------------------------------------------------------------------------
// Adds the given list of points params to the ends of the appropriate lists, intializing
// velocities to (0,0) if not specified
//
void CubicSpline2D::AppendPoints(const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray/*=nullptr */)
{
	// Push back the positions and velocities
	for (int i = 0; i < numPoints; ++i)
	{
		if (velocitiesArray != nullptr)
		{
			AppendPoint(positionsArray[i], velocitiesArray[i]);
		}
		else
		{
			AppendPoint(positionsArray[i]);	// Velocity defaults to Vector2::ZERO			
		}
		
	}
}


//-----------------------------------------------------------------------------------------------
// Appends the given positions to the end of the array, with (0,0) velocities
//
void CubicSpline2D::AppendPositions(const Vector2* positionsArray, int numPoints)
{
	for (int pointIndex = 0; pointIndex < numPoints; ++pointIndex)
	{
		AppendPoint(positionsArray[pointIndex]);
	}
}


//-----------------------------------------------------------------------------------------------
// Inserts the point at the given location in the arrays
//
void CubicSpline2D::InsertPoint(int insertBeforeIndex, const Vector2& position, const Vector2& velocity/*=Vector2::ZERO */)
{
	m_positions.insert(m_positions.begin() + insertBeforeIndex, position);
	m_velocities.insert(m_velocities.begin() + insertBeforeIndex, velocity);
}


//-----------------------------------------------------------------------------------------------
// Removes the point at the given index
//
void CubicSpline2D::RemovePoint(int pointIndex)
{
	m_positions.erase(m_positions.begin() + pointIndex);
	m_velocities.erase(m_velocities.begin() + pointIndex);
}


//-----------------------------------------------------------------------------------------------
// Clears the position and velocity arrays
//
void CubicSpline2D::RemoveAllPoints()
{
	m_positions.clear();
	m_velocities.clear();
}


//-----------------------------------------------------------------------------------------------
// Sets the position and velocity at the given index to the ones specified
//
void CubicSpline2D::SetPoint(int pointIndex, const Vector2& newPosition, const Vector2& newVelocity)
{
	m_positions[pointIndex] = newPosition;
	m_velocities[pointIndex] = newVelocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the position at the given index to the ones specified
//
void CubicSpline2D::SetPosition(int pointIndex, const Vector2& newPosition)
{
	m_positions[pointIndex] = newPosition;
}


//-----------------------------------------------------------------------------------------------
// Sets the velocity at the given index to the ones specified
//
void CubicSpline2D::SetVelocity(int pointIndex, const Vector2& newVelocity)
{
	m_velocities[pointIndex] = newVelocity;
}


//-----------------------------------------------------------------------------------------------
// Sets the velocities of the curve to be representative of a Cardinal spline
//
void CubicSpline2D::SetCardinalVelocities(float tension/*=0.f*/, const Vector2& startVelocity/*=Vector2::ZERO*/, const Vector2& endVelocity/*=Vector2::ZERO */)
{
	int numPoints = GetNumPoints();

	// Set the start and end velocities to 0
	SetVelocity(0, startVelocity);
	SetVelocity(numPoints - 1, endVelocity);

	// Iterate across all interior points and assign their velocities
	for (int pointIndex = 1; pointIndex < numPoints - 1; ++pointIndex)
	{
		// Get neighbor points
		Vector2 startPosition	= m_positions[pointIndex - 1];
		Vector2 endPosition		= m_positions[pointIndex + 1];

		// Get the tangent
		Vector2 tangent = (endPosition - startPosition);

		// Set the velocity, factoring in tension
		Vector2 velocity = (1.f - tension) * tangent * 0.5f;
		SetVelocity(pointIndex, velocity);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the point at the given index
//
const Vector2 CubicSpline2D::GetPosition(int pointIndex) const
{
	return m_positions.at(pointIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns the velocity of the point at the given index
//
const Vector2 CubicSpline2D::GetVelocity(int pointIndex) const
{
	return m_velocities.at(pointIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns the positions of this spline in the reference passed, and the number of positions in
// the list as the return value
//
int CubicSpline2D::GetPositions(std::vector<Vector2>& out_positions) const
{
	// Clear the list to only return the positions in this Spline
	out_positions.clear();

	for (int positionIndex = 0; positionIndex < static_cast<int>(m_positions.size()); ++positionIndex)
	{
		out_positions.push_back(m_positions.at(positionIndex));
	}

	return static_cast<int>(out_positions.size());
}


//-----------------------------------------------------------------------------------------------
// Returns the velocities of this spline in the reference passed, and the number of velocities in
// the list as the return value
//
int CubicSpline2D::GetVelocities(std::vector<Vector2>& out_velocities) const
{
	// Clear the list to only return the velocities in this Spline
	out_velocities.clear();

	for (int velocityIndex = 0; velocityIndex < static_cast<int>(m_velocities.size()); ++velocityIndex)
	{
		out_velocities.push_back(m_velocities.at(velocityIndex));
	}

	return static_cast<int>(out_velocities.size());
}


//-----------------------------------------------------------------------------------------------
// Evaluates the spline at the given parameter t, where t is between 0 and NumPoints - 1 on the curve (can go past curve endpoints)
// t == 0.f is the startpoint of the first curve, t == NumPoints - 1 is the endpoint of the last curve
//
Vector2 CubicSpline2D::EvaluateAtCumulativeParametric(float t) const
{
	int startPosIndex		= static_cast<int>(t);								// Curve number is the integral value of t
	startPosIndex			= ClampInt(startPosIndex, 0, GetNumPoints() - 2);	// Clamp to avoid indexing off the end (first and last curves continue indefinitely)

	float curveParameter	= (t - static_cast<float>(startPosIndex));			// Curve value is the decimal	

	// Positions
	Vector2 startPosition	= m_positions.at(startPosIndex);
	Vector2 endPosition		= m_positions.at(startPosIndex + 1);

	// Velocities
	Vector2 startVelocity	= m_velocities.at(startPosIndex);
	Vector2 endVelocity		= m_velocities.at(startPosIndex + 1);

	return EvaluateCubicHermite(startPosition, startVelocity, endPosition, endVelocity, curveParameter);
}


//-----------------------------------------------------------------------------------------------
// Evaluates the spline at the given parameter t, where t is between 0 and 1
// t == 0.f is the startpoint of the first curve, t == 1 is the endpoint of the last curve
//
Vector2 CubicSpline2D::EvaluateAtNormalizedParametric(float t) const
{
	// Convert to cumulative parametric - 0 to 1 for each curve
	float numCurves = static_cast<float>(GetNumPoints() - 1);
	float cumulativeParametric = t * numCurves;

	 return EvaluateAtCumulativeParametric(cumulativeParametric);
}

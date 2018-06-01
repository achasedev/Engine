/************************************************************************/
/* File: Trajectory.cpp
/* Author: Andrew Chase
/* Date: March 2nd, 2018
/* Description: Implementation of Trajectory namespace functions
/************************************************************************/
#include <math.h>
#include "Engine/Math/Trajectory.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------------------
// Returns the (x,y) position of a projectile in the trajectory at the specified time
//
Vector2 Trajectory::EvaluateTrajectoryAtTime(float gravityPositive, const Vector2& launchVelocity, float timeIntoTrajectory)
{
	float launchSpeed = launchVelocity.GetLength();
	float launchAngle = launchVelocity.GetOrientationDegrees();

	return EvaluateTrajectoryAtTime(gravityPositive, launchSpeed, launchAngle, timeIntoTrajectory);
}


//-----------------------------------------------------------------------------------------------
// Returns the (x,y) position of a projectile in the trajectory at the specified time
//
Vector2 Trajectory::EvaluateTrajectoryAtTime(float gravityPositive, float launchSpeed, float launchAngle, float timeIntoTrajectory)
{
	Vector2 result;

	// vcos(theta)t
	result.x = launchSpeed * CosDegrees(launchAngle) * timeIntoTrajectory;

	// -1/2gt^2 + vsin(theta)t
	result.y = (-0.5f * gravityPositive * (timeIntoTrajectory * timeIntoTrajectory)) + (launchSpeed * SinDegrees(launchAngle) * timeIntoTrajectory);

	return result;
}


//-----------------------------------------------------------------------------------------------
// Calculates the minimum launch speed necessary to reach the desired distance
// ASSUMPTIONS - uses launch angle of 45 degrees for max distance, and assumes y-displacement is 0
//
float Trajectory::CalculateMinimumLaunchSpeed(float gravityPositive, float horizontalDisplacement)
{
	return sqrtf(horizontalDisplacement * gravityPositive);
}


//-----------------------------------------------------------------------------------------------
// Returns the launch angles that, when given a fixed launch speed, hit a designated distance at a designated height
//
bool Trajectory::CalculateLaunchAngles(Vector2& out_result, float gravityPositive, float launchSpeed, float horizontalDisplacement, float verticalDisplacement /*= 0.0f*/)
{
	// After doing some math, -1/2 * g * (dx^2/v^2) * tan^2(theta) + dx * tan(theta) - 1/2 * g * (dx^2/v^2) - dy

	// Set up the coefficients to the quadratic in tan(theta)
	float a = -0.5f * gravityPositive * ((horizontalDisplacement * horizontalDisplacement) / (launchSpeed * launchSpeed));
	float b = horizontalDisplacement;
	float c = -0.5f * gravityPositive * ((horizontalDisplacement * horizontalDisplacement ) / (launchSpeed * launchSpeed)) - verticalDisplacement;

	// Solve for the roots
	Vector2 roots;
 	bool rootExists = Quadratic(roots, a, b, c);

	// If no roots exist, return false
	if (!rootExists)
	{
		return false;
	}

	// We have two solutions for tan(theta) - convert them to degree angles
	float firstResult	= Atan2Degrees(roots.x);
	float secondResult	= Atan2Degrees(roots.y);

	// Order them by magnitude
	if (firstResult < secondResult)
	{
		out_result.x = firstResult;
		out_result.y = secondResult;
	}
	else
	{
		out_result.x = secondResult;
		out_result.y = firstResult;
	}

	// Found 2 solutions (note both might be the same), so return true
	return true;
}


//-----------------------------------------------------------------------------------------------
// Returns the max possible height we could reach at the target by just adjusting the angle
// NOTE: Returns a negative value if the specified displacement cannot be reached at all
//
float Trajectory::GetMaxHeight(float gravityPositive, float launchSpeed, float horizontalDisplacement)
{
	// Solving the derivative of the quadratic to find local max for dy (linear equation)
	float tanTheta = (launchSpeed * launchSpeed) / (gravityPositive * horizontalDisplacement);
	float maxAngle = Atan2Degrees(tanTheta);
	
	// Now that we have the max angle with launch speed, find t so we can find dy
	float time = (horizontalDisplacement) / (launchSpeed * CosDegrees(maxAngle));

	float verticalDisplacement = -0.5f * gravityPositive * (time * time) + launchSpeed * SinDegrees(maxAngle) * time;

	return verticalDisplacement;
}


//-----------------------------------------------------------------------------------------------
// Given a target apex height and a target horizontal/vertical displacement, determine the launch velocity
// Assumes apex height is larger than target height, and apex height is > 0 (positive initial y velocity)
//
Vector2 Trajectory::CalculateLaunchVelocity(float gravityPositive, float apexHeight, float horizontalDisplacement, float verticalDisplacement)
{
	// First find the initial y velocity to ensure we hit the apex exactly
	float initialYVelocity = sqrtf(2.f * gravityPositive * apexHeight);	// Don't do +- here on sqrt, since we want positive upward velocity

	// Then, using the initial y velocity, determine the flight time (Quadratic solution)
	float a = -0.5f * gravityPositive;
	float b = initialYVelocity;
	float c = -verticalDisplacement;

	Vector2 result;
	bool solutionExists = Quadratic(result, a, b, c);

	if (!solutionExists)
	{
		return Vector2(-1.0f, -1.0f);	// Return no solution (apex was less than final height)
	}

	// Solution is the second value, since the first value is the first time that we are at that height (either a negative
	// time, or the time before the apex on the trajectory)
	float time = result.y;

	// Finally, use the flight time to determine the initial x velocity
	float initialXVelocity = horizontalDisplacement / time;

	// Return the result
	return Vector2(initialXVelocity, initialYVelocity);
}


//-----------------------------------------------------------------------------------------------
// Returns the total flight time given gravity, initial vertical velocity and vertical displacement
//
float Trajectory::CalculateFlightTime(float gravityPositive, float initialYVelocity, float verticalDisplacement)
{
	float a = -0.5f * gravityPositive;
	
	Vector2 solutions;
	
	bool solutionsExist = Quadratic(solutions, a, initialYVelocity, -verticalDisplacement);
	if (solutionsExist)
	{
		// Return the greater solution of the two
		return solutions.y;
	}
	else
	{
		return -1.0f;
	}
}

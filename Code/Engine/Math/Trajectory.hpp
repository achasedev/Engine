/************************************************************************/
/* File: Trajectory.hpp
/* Author: Andrew Chase
/* Date: January 23rd, 2017
/* Bugs: None
/* Description: Namespace for Math Trajectory functions
/************************************************************************/
#pragma once
#include "Engine/Math/Vector2.hpp"

namespace Trajectory
{
	// Evaluates the (x,y) position of a projectile along the trajectory at the specified time
	Vector2 EvaluateTrajectoryAtTime(float gravityPositive, const Vector2& launchVelocity, float timeIntoTrajectory);
	Vector2 EvaluateTrajectoryAtTime(float gravityPositive, float launchSpeed, float launchAngle, float timeIntoTrajectory);

	// Calculates the minimum launch speed necessary to reach the desired distance
	float CalculateMinimumLaunchSpeed(float gravityPositive, float horizontalDisplacement);

	// Returns the launch angles that, when given a fixed launch speed, hit a designated distance at a designated height
	bool CalculateLaunchAngles(Vector2& out_result, float gravityPositive, float launchSpeed, float horizontalDisplacement, float verticalDisplacement = 0.0f);

	// Returns the max possible height we could reach at the target by just adjusting the angle
	float GetMaxHeight(float gravityPositive, float launchSpeed, float horizontalDisplacement);

	// Given a target apex height and a target horizontal/vertical displacement, determine the launch velocity
	// Assumes apex height is larger than target height, and apex height is > 0
	Vector2 CalculateLaunchVelocity(float gravityPositive, float apexHeight, float horizontalDisplacement, float verticalDisplacement);

	// Returns the time it will take from launch to land
	float CalculateFlightTime(float gravityPositive, float initialYVelocity, float verticalDisplacement);
}

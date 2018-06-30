/************************************************************************/
/* File: ProfileReport.hpp
/* Author: Andrew Chase
/* Date: June 30th, 2018
/* Description: Class to represent a profile result for a single frame
/************************************************************************/
#pragma once
#include <vector>

class ProfileMeasurement;
class ProfileReportEntry;

class ProfileReport
{
public:
	//-----Public Methods-----

	ProfileReport();
	~ProfileReport();

	void InitializeAsTreeReport(ProfileMeasurement* stack);
	void InitializeAsFlatReport(ProfileMeasurement* stack);

	void Finalize();


public:
	//-----Public Data-----

	ProfileReportEntry* m_rootEntry;

};

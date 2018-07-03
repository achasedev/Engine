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

enum eReportType
{
	REPORT_TYPE_TREE,
	REPORT_TYPE_FLAT,
	NUM_REPORT_TYPES
};

class ProfileReport
{
public:
	//-----Public Methods-----

	ProfileReport(int frameNumber);
	~ProfileReport();

	void InitializeAsTreeReport(ProfileMeasurement* stack);
	void InitializeAsFlatReport(ProfileMeasurement* stack);

	void Finalize();


public:
	//-----Public Data-----
	
	unsigned int			m_frameNumber;
	eReportType				m_type;
	ProfileReportEntry*		m_rootEntry;

};

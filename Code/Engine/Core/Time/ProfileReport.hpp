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

enum eSortOrder
{
	REPORT_SORT_TOTAL_TIME,
	REPORT_SORT_SELF_TIME,
	NUM_REPORT_SORT_ORDERS
};

class ProfileReport
{
public:
	//-----Public Methods-----

	ProfileReport(int frameNumber);
	~ProfileReport();

	void InitializeAsFlatReport(ProfileMeasurement* stack, eSortOrder sortOrder);
	void InitializeAsTreeReport(ProfileMeasurement* stack, eSortOrder sortOrder);

	void Finalize();


public:
	//-----Public Data-----
	
	unsigned int			m_frameNumber;
	eReportType				m_type;
	ProfileReportEntry*		m_rootEntry;
	eSortOrder				m_sortOrder;
};

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

#pragma once
#include "CFlow.h"

using namespace std;

class CFlowImage : public CFlow
{
protected:
	string LogImageLocation;
    bool isLogImage;
    unsigned short logfileRetentionInDays;
	const char* logTag;

	string CreateLogFolder(string time);
	void LogImage(string logPath, string name, float *resultFloat, int *resultInt, string time, CImageBasis *_img);


public:
	CFlowImage(const char* logTag);
	CFlowImage(std::vector<CFlow*> * lfc, const char* logTag);
	CFlowImage(std::vector<CFlow*> * lfc, CFlow *_prev, const char* logTag);
	
	void RemoveOldLogs();
};

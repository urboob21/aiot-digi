#pragma once

#include <string>
#include "CFlow.h"
#include "CFlowPostProcessing.h"
#include "CFlowAlignment.h"
#include "CFlowDigit.h"
#include "CFlowMakeImage.h"


class CFlowControl :
    public CFlow
{
protected:
	std::vector<CFlow*> FlowControll;
	CFlowPostProcessing* flowpostprocessing;
	CFlowAlignment* flowalignment;	
	CFlowDigit* flowdigit;
	CFlowMakeImage* flowmakeimage;
	CFlow* CreateCFlow(std::string _type);

	bool AutoStart;
	float AutoIntervall;
	bool SetupModeActive;
	void SetInitialParameter(void);	
	std::string aktstatus;
	int aktRunNr;

	void UpdateAktStatus(std::string _flow);

public:
	void InitFlow(std::string config);
	bool doFlow(string time);
	void doFlowMakeImageOnly(string time);
	bool getStatusSetupModus(){return SetupModeActive;};
	using CFlow::getReadout;
	string getReadout(bool _rawvalue, bool _noerror);
	string UpdatePrevalue(std::string _newvalue);
	string GetPrevalue();	
	bool ReadParameter(FILE* pfile, string& aktparamgraph);	

	esp_err_t GetJPGStream(std::string _fn, httpd_req_t *req);
	esp_err_t SendRawJPG(httpd_req_t *req);

	std::string doSingleStep(std::string _stepname, std::string _host);

	bool isAutoStart(long &_intervall);

	std::string getActStatus();

	std::vector<HTMLInfo*> GetAllDigital();

	int CleanTempFolder();

	string name(){return "CFlowControll";};
};



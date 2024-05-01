#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "Helper.h"
#include "CImageBasis.h"

using namespace std;

#define LOGFILE_TIME_FORMAT "%Y%m%d-%H%M%S"
#define LOGFILE_TIME_FORMAT_DATE_EXTR substr(0, 8)
#define LOGFILE_TIME_FORMAT_HOUR_EXTR substr(9, 2)

struct HTMLInfo
{
	float val;
	CImageBasis *image = NULL;
	CImageBasis *image_org = NULL;
	std::string filename;
	std::string filename_org;	
};

class CFlow
{
protected:
	std::vector<string> ZerlegeZeile(string input, string delimiter = " =, \t");
	bool isNewParagraph(string input);
	bool GetNextParagraph(FILE* pfile, string& aktparamgraph);
	bool getNextLine(FILE* pfile, string* rt);

	std::vector<CFlow*>* ListFlowControll;
	CFlow *previousElement;

	virtual void SetInitialParameter(void);

	bool disabled;

public:
	CFlow(void);
	CFlow(std::vector<CFlow*> * lfc);
	CFlow(std::vector<CFlow*> * lfc, CFlow *_prev);	
	
	virtual bool ReadParameter(FILE* pfile, string &aktparamgraph);
	virtual bool doFlow(string time);
	virtual string getHTMLSingleStep(string host);
	virtual string getReadout();
	virtual string name(){return "CFlow";};

};
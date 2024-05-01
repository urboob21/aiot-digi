#include "CFlow.h"
#include <fstream>
#include <string>
#include <iostream>
#include <string.h>



void CFlow::SetInitialParameter(void)
{
	ListFlowControll = NULL;
	previousElement = NULL;	
	disabled = false;
}




std::vector<string> CFlow::ZerlegeZeile(std::string input, std::string delimiter)
{
	std::vector<string> Output;

	input = trim(input, delimiter);
	size_t pos = findDelimiterPos(input, delimiter);
	std::string token;
	while (pos != std::string::npos) {
		token = input.substr(0, pos);
		token = trim(token, delimiter);
		Output.push_back(token);
		input.erase(0, pos + 1);
		input = trim(input, delimiter);
		pos = findDelimiterPos(input, delimiter);
	}
	Output.push_back(input);

	return Output;

}

bool CFlow::isNewParagraph(string input)
{
	if ((input[0] == '[') || ((input[0] == ';') && (input[1] == '[')))
	{
		return true;
	}
	return false;
}

bool CFlow::GetNextParagraph(FILE* pfile, string& aktparamgraph)
{
	while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph));

	if (isNewParagraph(aktparamgraph))
		return true;
	return false;
}


CFlow::CFlow(void)
{
	SetInitialParameter();
}

CFlow::CFlow(std::vector<CFlow*> * lfc)
{
	SetInitialParameter();	
	ListFlowControll = lfc;
}

CFlow::CFlow(std::vector<CFlow*> * lfc, CFlow *_prev)
{
	SetInitialParameter();	
	ListFlowControll = lfc;
	previousElement = _prev;
}	

bool CFlow::ReadParameter(FILE* pfile, string &aktparamgraph)
{
	return false;
}

bool CFlow::doFlow(string time)
{
	return false;
}

string CFlow::getHTMLSingleStep(string host){
	return "";
}

string CFlow::getReadout()
{
	return string();
}

bool CFlow::getNextLine(FILE* pfile, string *rt)
{
	char zw[1024];
	if (pfile == NULL)
	{
		*rt = "";
		return false;
	}
	fgets(zw, 1024, pfile);
	printf("%s", zw);
	if ((strlen(zw) == 0) && feof(pfile))
	{
		*rt = "";
		return false;
	}
	*rt = zw;
	*rt = trim(*rt);
	while ((zw[0] == ';' || zw[0] == '#' || (rt->size() == 0)) && !(zw[1] == '['))			// Kommentarzeilen (; oder #) und Leerzeilen überspringen, es sei denn es ist ein neuer auskommentierter Paragraph
	{
		fgets(zw, 1024, pfile);
		printf("%s", zw);		
		if (feof(pfile))
		{
			*rt = "";
			return false;
		}
		*rt = zw;
		*rt = trim(*rt);
	}
	return true;
}

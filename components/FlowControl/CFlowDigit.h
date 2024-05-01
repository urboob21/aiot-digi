#pragma once
#include "CFlowImage.h"
#include "CFlowAlignment.h"
#include "Helper.h"

#include <string>

struct roi {
    int posx, posy, deltax, deltay;
    int resultklasse;
    string name;
    CImageBasis *image, *image_org;
    roi* next;
};

class CFlowDigit :
    public CFlowImage
{
protected:
    std::vector<roi*> ROI;
    string cnnmodelfile;
    int modelxsize, modelysize;
    bool SaveAllFiles;

    CFlowAlignment* flowpostalignment;
 
    bool doNeuralNetwork(string time); 
    bool doAlignAndCut(string time); 

	void SetInitialParameter(void);    

public:
    CFlowDigit();
    CFlowDigit(std::vector<CFlow*>* lfc);
    CFlowDigit(std::vector<CFlow*>* lfc, CFlow *_prev);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host); 
    string getReadout();
   	std::vector<HTMLInfo*> GetHTMLInfo();

    void DrawROI(CImageBasis *_zw);        

    string name(){return "CFlowDigit";};
};


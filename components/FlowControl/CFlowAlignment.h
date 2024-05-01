#pragma once

#include "CFlow.h"
#include "Helper.h"
#include "CAlignAndCutImage.h"
#include "CFindTemplate.h"

#include <string>

using namespace std;

class CFlowAlignment :
    public CFlow
{
protected:
    float initalrotate;
    bool initialmirror;
    RefInfo References[2];
    int anz_ref;
    string namerawimage;
    bool SaveAllFiles;
    CAlignAndCutImage *AlignAndCutImage;
    std::string FileStoreRefAlignment;
    float SAD_criteria;

    void SetInitialParameter(void);
    bool LoadReferenceAlignmentValues(void);
    void SaveReferenceAlignmentValues();

public:
    CImageBasis *ImageBasis, *ImageTMP;
    
    CFlowAlignment(std::vector<CFlow*>* lfc);

    CAlignAndCutImage* GetAlignAndCutImage(){return AlignAndCutImage;};

    void DrawRef(CImageBasis *_zw);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    string name(){return "CFlowAlignment";};
};


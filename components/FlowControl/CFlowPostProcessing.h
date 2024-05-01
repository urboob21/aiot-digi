#pragma once
#include "CFlow.h"

#include <string>


class CFlowPostProcessing :
    public CFlow
{
protected:
    bool PreValueUse;
    int PreValueAgeStartup; 
    bool AllowNegativeRates;
    float MaxRateValue;
    bool useMaxRateValue;
    bool ErrorMessage;
    bool PreValueOkay;
    bool checkDigitIncreaseConsistency;
    int DecimalShift;

    string FilePreValue;
    float PreValue;             // letzter Wert, der gut ausgelesen wurde
    float Value;                // letzer ausgelesener Wert, inkl. Korrekturen
    string ReturnRawValue;      // Rohwert (mit N & führenden 0)    
    string ReturnValue;         // korrigierter Rückgabewert, ggf. mit Fehlermeldung
    string ReturnValueNoError;  // korrigierter Rückgabewert ohne Fehlermeldung
    string ErrorMessageText;        // Fehlermeldung bei Consistency Check

    bool LoadPreValue(void);
    string ShiftDecimal(string in, int _decShift);

    string ErsetzteN(string);
    float checkDigitConsistency(float input, int _decilamshift, bool _isanalog);
    string RundeOutput(float _in, int _anzNachkomma);

public:
    CFlowPostProcessing(std::vector<CFlow*>* lfc);
    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getReadout();
    string getReadoutParam(bool _rawValue, bool _noerror);
    string getReadoutError();
    void SavePreValue(float value, string time = "");
    string GetPreValue();

    string name(){return "CFlowPostProcessing";};
};


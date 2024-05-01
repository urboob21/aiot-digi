#pragma once
#include <string>
#include <fstream>


using namespace std;

std::string FormatFileName(std::string input);
void FindReplace(std::string& line, std::string& oldString, std::string& newString);

void CopyFile(string input, string output);

FILE* OpenFileAndWait(const char* nm, char* _mode, int _waitsec = 1);

size_t findDelimiterPos(string input, string delimiter);
//string trim(string istring);
string trim(string istring, string adddelimiter = "");
bool ctype_space(const char c, string adddelimiter);

string getFileType(string filename);

int mkdir_r(const char *dir, const mode_t mode);
int removeFolder(const char* folderPath, const char* logTag);

string toUpper(string in);

float temperatureRead();

time_t addDays(time_t startTime, int days);

void memCopyGen(uint8_t* _source, uint8_t* _target, int _size);

///////////////////////////
size_t getInternalESPHeapSize();
size_t getESPHeapSize();
string getESPHeapInfo();

/////////////////////////////

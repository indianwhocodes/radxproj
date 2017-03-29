
#ifndef Radx2GridPlus_h
#define Radx2GridPlus_h

#include <string>
#include "PolarDataStream.h"
#include "Polar2Cartesian.h"
#include "Cartesian2Grid.h"

class Radx2GridPlus
{
public:
    Radx2GridPlus(std::string pName);
    ~Radx2GridPlus();
    void processFiles(const time_t& startTime, const time_t& endTime, const Params& params);
    std::string* findFileNames();

    // setter, getter
    std::string getInputDir();
    std::string getOutputDir();
    
    void setTime(const time_t& startTime, const time_t& endTime);


private: 
    std::string _programName;
    std::string _inputDir;
    std::string _outputDir;
    time_t _startTime;
    time_t _endTime;
    
};

#endif



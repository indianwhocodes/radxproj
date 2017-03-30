
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
    void processFiles(const string& filePath, const Params& params);

    // setter, getter
    std::string getInputDir();
    std::string getOutputDir();


private: 
    std::string _programName;
    std::string _inputDir;
    std::string _outputDir;
    
};

#endif



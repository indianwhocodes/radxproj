
#include "Params.hh"
#include "Radx2GridPlus.hh"

Radx2GridPlus::Radx2GridPlus(std::string pName) 
{
    _programName = pName;
}

Radx2GridPlus::~Radx2GridPlus() 
{

}

void Radx2GridPlus::processFiles(const time_t& startTime, const time_t& endTime, const Params& params) 
{
    setTime(startTime, endTime);
    _inputDir = params.input_dir; _outputDir = params.output_dir;
    std::cout << "input dir : " << _inputDir << std::endl;
    std::string* fileNames = findFileNames();
    std::string tempFile = "test file name";
    

    // step 1 : Convert netcdf file to PolarDataStream
    // loop the number of files
    
//  PolarDataStream pds(fileNames[0]); 
    PolarDataStream pds(tempFile); 
    pds.LoadDataFromNetCDFFilesIntoRepository();
    pds.populateOutputValues();

    // step 2 : PolarDataStream to Cartesian coordinates
    Polar2Cartesian pc(pds.getRepository());
    pc.calculateCartesianCoords();

    // step 3 : Gridding
    Cartesian2Grid cg(pds.getRepository());
    cg.calculateGridSize(params);
    cg.calculateRefGrid();

    // step 4 :  Writing the grids to MDV File 


}
// search for the files from the directory
std::string* Radx2GridPlus::findFileNames()
{

}

void Radx2GridPlus::setTime(const time_t& startTime, const time_t& endTime)
{
    _startTime = startTime;
    _endTime = endTime;
}

std::string Radx2GridPlus::getInputDir()
{ 
    
}
std::string Radx2GridPlus::getOutputDir()
{

}

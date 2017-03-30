
#include "Params.hh"
#include "Radx2GridPlus.hh"

Radx2GridPlus::Radx2GridPlus(std::string pName) 
{
    _programName = pName;
}

Radx2GridPlus::~Radx2GridPlus() 
{

}

void Radx2GridPlus::processFiles(const string& filePath, const Params& params)
{
    _inputDir = params.input_dir; _outputDir = params.output_dir;
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

std::string Radx2GridPlus::getInputDir()
{ 
    
}
std::string Radx2GridPlus::getOutputDir()
{

}

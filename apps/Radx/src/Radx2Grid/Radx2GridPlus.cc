
#include "Radx2GridPlus.hh"
#include "Params.hh"
#include "tbb/task_group.h"
#include "tbb/tbb.h"
#include <ctime>
Radx2GridPlus::Radx2GridPlus(std::string pName) { _programName = pName; }

Radx2GridPlus::~Radx2GridPlus() {}

void Radx2GridPlus::processFiles(const vector<string> &filepaths,
                                 const Params &params) {
	_inputDir = params.input_dir;
	_outputDir = params.output_dir;
  
  
	std::vector<PolarDataStream> pds;
	std::vector<Cartesian2Grid> cg;
	std::vector<Polar2Cartesian> p2c;

	for(int i=0; i<filepaths.size(); i++){
		pds.push_back(PolarDataStream( filepaths[i]));
		p2c.push_back(Polar2Cartesian(pds[i].getRepository()));
		cg.push_back(Cartesian2Grid(pds[i].getRepository()));
	}

	clock_t start = clock();
	for(int i=0; i<filepaths.size(); i++){
		pds[i].LoadDataFromNetCDFFilesIntoRepository();
		pds[i].populateOutputValues();

		 // step 2 : PolarDataStream to Cartesian coordinates
		 p2c[i].calculateCartesianCoords();

		 // step 3 : Gridding
		 cg[i].calculateGridSize(params);
		 cg[i].calculateRefGrid();

	}
	std::cout<<"Serial time"<<clock() - start << std::endl;
  
	int n=filepaths.size();
	start = clock();
	tbb::parallel_for(0, n, [&](size_t i){
 		pds[i].LoadDataFromNetCDFFilesIntoRepository();
		pds[i].populateOutputValues();

		// step 2 : PolarDataStream to Cartesian coordinates
		p2c[i].calculateCartesianCoords();

		// step 3 : Gridding
		cg[i].calculateGridSize(params);
		cg[i].calculateRefGrid();

		//std::cout<<filepaths[i]<<" completed!"<<std::endl;
		});
  
	std::cout<<"Parallel time"<<clock() - start << std::endl;

}

std::string Radx2GridPlus::getInputDir() { throw "Not Implemented"; }

Radx2GridPlus::Radx2GridPlus(std::string pName) { _programName = pName; }

Radx2GridPlus::~Radx2GridPlus() {}

std::string Radx2GridPlus::getInputDir() { throw "Not Implemented"; }


std::string Radx2GridPlus::getOutputDir() { throw "Not Implemented"; }

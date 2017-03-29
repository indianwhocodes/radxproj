
#ifndef PolarDataStream_h
#define PolarDataStream_h

#include <string>
#include <iostream> 

struct Repository;
class PolarDataStream
{
public:
    // constructor & destructor
    PolarDataStream(const std::string& inputFile);
    ~PolarDataStream();

    // read variables from NetCDF files
    void LoadDataFromNetCDFFilesIntoRepository();

    // populate the output values
    void populateOutputValues();

    // setter, getter
    float* getOutElevation();
    float* getOutAzimuth();
    float* getOutGate();
    float* getOutRef();

    Repository* getRepository();


private:
 
    Repository* _store;

	// class PolarDataStreamImpl;
	// PolarDataStreamImpl* _dataStreamHandle;


};

#endif

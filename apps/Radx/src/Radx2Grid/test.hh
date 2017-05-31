
#ifndef Test_h
#define Test_h

#include "PolarDataStream.hh"
#include "WriteOutput.hh"
#include "Params.hh"
#include <Radx/RadxVol.hh>
#include <Radx/Radx.hh>
#include <string>


class Test 
{
public:
    Test(){}
    ~Test(){}


    int run(string progName, fl32 **outputFields, Params params, const string& filePath)
    {
        cout << "DEBUG: inside of Test::run()" << endl;
//        params.output_dir = "./test";
        cout << "DEBUG: set params_output_dir" << endl;
	    PolarDataStream pds(filePath, params);
        cout << "DEBUG: create PolarDataStream instance" << endl;
	    pds.LoadDataFromNetCDFFilesIntoRepository();
        cout << "DEBUG: call LoadDataFromNetCDFFilesIntoRepository()" << endl;
	    WriteOutput wo(progName, pds.getRepository(), pds.getRadxVol(), params, pds.getInterpFields());
        cout << "DEBUG: create WriteOutput instance" << endl;
	    wo.testWriteOutputFile(outputFields);
        cout << "DEBUG: call testWriteOutputFile()" << endl;

        return 0;
    }


};



#endif

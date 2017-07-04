#include "WriteOutput.hh"

#include <algorithm>
#include <fstream>
#include <memory>

WriteOutput::WriteOutput(const shared_ptr<Cart2Grid>& grid,
                         const shared_ptr<Repository>& store,
                         const Params& params)
  : _grid(grid)
  , _store(store)
  , _params(params)

{
}

WriteOutput::~WriteOutput()
{
}

/////////////////////////////////////////////////////
// write out data

int
WriteOutput::writeOutputFile()
{

  // Construct projection WKT;
  stringstream prj_writer;

  // Write projection file
  ofstream prj_file;
  prj_file.open("a.prj", std::ios::trunc);
  prj_file << prj_writer.str();
  prj_file.flush();
  prj_file.close();

  return 0;
}

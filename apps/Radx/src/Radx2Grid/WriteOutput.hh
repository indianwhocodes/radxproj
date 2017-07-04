#ifndef WriteOutput_h
#define WriteOutput_h

#include "Cart2Grid.hh"
#include "PolarDataStream.hh"
#include <memory>
#include <sstream>
#include <vector>

class WriteOutput
{
public:
  // constructor & destructor
  WriteOutput(const shared_ptr<Cart2Grid>& grid,
              const shared_ptr<Repository>& store,
              const Params& params);
  ~WriteOutput();

  int writeOutputFile();

private:
  std::shared_ptr<Cart2Grid> _grid;
  std::shared_ptr<Repository> _store;
  const Params& _params;

  stringstream _prj_writer;

  // output projection and grid
};

#endif

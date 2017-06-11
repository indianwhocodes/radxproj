#ifndef WriteOutput_h
#define WriteOutput_h

#include "Cart2Grid.hh"
#include "PolarDataStream.hh"
#include <memory>
#include <vector>

class WriteOutput
{
public:
  // constructor & destructor
  WriteOutput(const shared_ptr<Cart2Grid>& grid, const Params& params);
  ~WriteOutput();

  int writeOutputFile();

private:
  std::shared_ptr<Cart2Grid> _grid;
  const Params& _params;

  // output projection and grid
};

#endif

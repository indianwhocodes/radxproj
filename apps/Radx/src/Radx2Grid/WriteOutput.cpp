#include "WriteOutput.hh"
#include <Radx/RadxRay.hh>
#include <algorithm>
#include <memory>

WriteOutput::WriteOutput(const shared_ptr<Cart2Grid>& grid,
                         const Params& params)
  : _grid(grid)
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
  if (_params.output_format == Params::output_format_t::CF_NETCDF) {
    // Write out netcdf
  } else if (_params.output_format == Params::output_format_t::RASTER) {
    // Write out Raster
  } else {
    // Show a warning that outputs format are not supported
    return -1;
  }

  return 0;
}

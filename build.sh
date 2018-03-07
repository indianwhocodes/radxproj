export CC=icc
export CXX=icpc
export FC=ifort

#export CFLAGS='-O3 -xMIC-AVX512 -ip'
#export CXXFLAGS="-std=c++14 $CFLAGS"
#export FFLAGS=$CFLAGS
#export FCFLAGS=$CFLAGS

#autoreconf
#./configure --with-hdf5=$HOME/build --with-netcdf=$HOME/build --prefix=$HOME/build
#make

export CFLAGS='-g -xHost'
export CXXFLAGS="-std=c++14 -openmp -g"

export net_install='/home/apps/netcdf'
export gdal_install='/home/apps/gdal'
autoreconf

#./configure --with-hdf5=$net_install/ --with-netcdf=$net_install/ --with-radx=$net_install/radixcc_build --prefix=$net_install/radx2grid --with-tbb=/home/kunal/Downloads/tbb2017_20161128oss

./configure --with-hdf5=$net_install/ --with-netcdf=$net_install/ --with-radx=$net_install/radixcc_build --prefix=$net_install/radx2grid --with-tbb=/opt/intel/tbb --with-gdal=$gdal_install
#path is for intel tbb library
make



export CC=icc
export CXX=icpc
export FC=ifort

export CFLAGS='-O3 -xMIC-AVX512 -ip'
export CXXFLAGS="-std=c++14 $CFLAGS"
export FFLAGS=$CFLAGS
export FCFLAGS=$CFLAGS

autoreconf
./configure --with-hdf5=$HOME/build --with-netcdf=$HOME/build --prefix=$HOME/build
make

UCAR/EOL/RAL radx software for radial data for radar/lidar
==========================================================

README for BUILDING radx.

Web documentation
-----------------

For full details see:

  http://www.ral.ucar.edu/projects/titan/docs/radial_formats/

LINUX setup
-----------

Radx is primarily intended to run on LINUX and OSX, though it may
work on Windows using mingw.

Most good, up-to date LINUX distributions should work.

Recommended OSs are:

  Ubuntu
  Debian
  Fedora
  Scientific Linux
  Mac OSX

Required packages for compiling:

  gcc 4.3+ compiler
  g++ 4.3+ compiler
  gfortran compiler

  tcsh shell
  perl shell

  fftw3-devel development (for radar moments)
  bzip2-devel (for NEXRAD decompression)
  zlib-devel
  expat-devel (for udunits)

You will also need to download and build netcdf and
hdf5 libraries - see below.

Downloading
-----------

The radx tar file will be named:

  radx-yyyymmdd.src.tgz

where yyyymmdd is the date of the distribution.

Download this file from:

  ftp.rap.ucar.edu/pub/titan/radx

Previous releases can be found in:

  ftp.rap.ucar.edu/pub/titan/radx/previous_releases

Normally you will download the file into:

  /usr/local/src

and build from there.

However, any suitable directory can be used for this purpose.

Un-tarring the file
-------------------

Use the command

  tar xvfz radx-yyyymmdd.src.tgz

to un-tar the distribution into the current directory.

Here, yyyymmdd should be substituded with the actual date on the file.

Building
--------

You will probably need to be root for this step, unless you install
in your user area.

You run the build script as follows:

  ./build_radx

Be default, you will install into /usr/local.

The installed files will then be in:

  /usr/local/include
  /use/local/lib
  /usr/local/bin

You will need to be the root user perform this installation.

You can change the install location by specifying it as
a single argument to this script.

For example:

  build_radx /tmp/radx

will install in:

  /tmp/radx/include
  /tmp/radx/lib
  /tmp/radx/bin

Building netcdf separately
--------------------------

'build_radx' will invoke 'build_netcdf', since netcdf is required
for radx.

You can run the netcdf build separately. For example:

  build_netcdf /tmp/radx

will install netcdf in:

  /tmp/radx/include
  /tmp/radx/lib
  /tmp/radx/bin

Handling build errors
---------------------

If the build does not complete successfully, you will need to
track down the errors. It is the first errors in the build that
are the most important.

If you get errors, go into the directory giving problems, and
run the make as follows:

  make |& less

and scroll for errors.

Alternatively, run

  make >& make.log

and then inspect the make.log file.






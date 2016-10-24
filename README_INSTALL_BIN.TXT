UCAR/EOL/RAL radx software for radial data for radar/lidar
==========================================================

README for INSTALLING a BINARY distribution.

Web documentation
-----------------

For full details see:

  http://www.ral.ucar.edu/projects/titan/docs/radial_formats/

LINUX setup
-----------

Radx is primarily intended to run on LINUX and OSX.

Most good, up-to date LINUX distributions should work.

Recommended OSs are:

  Ubuntu
  Debian
  Fedora
  Scientific Linux
  Mac OSX

Required packages for compiling:

  tcsh shell
  perl shell
  python shell

Downloading
-----------

For 64-bit systems, the radx binary release will be named:

  radx-yyyymmdd.x86_64.tgz

For 32-bit systems, the radx binary release will be named:

  radx-yyyymmdd.i686.tgz

where yyyymmdd is the date of the distribution.

Download this file from:

  ftp.rap.ucar.edu/pub/titan/radx

Previous releases can be found in:

  ftp.rap.ucar.edu/pub/titan/radx/previous_releases

Download the file into a tmp area. For example:

  /tmp/radx

and install from there.

However, any suitable directory can be used for this purpose.

Un-tarring the file
-------------------

Use the command

  tar xvfz radx-yyyymmdd.x86_64.tgz (64-bit systems)

or

  tar xvfz radx-yyyymmdd.i686.tgz (32-bit systems)

to un-tar the distribution into the current directory.

yyyymmdd should be substituded with the actual date on the file.

Then, cd into the directory:

  cd radx-yyyymmdd.x86_64

or

  cd radx-yyyymmdd.i686

Installing binary release
-------------------------

You will probably need to be root for this step, unless you install
in your user area.

By default, the binaries will be installed in /usr/local/bin. You can specify an
alternative.

  install_bin_release 

will install in /usr/local/bin.

  install_bin_release /opt/local

will install in /opt/local/bin.

The support dynamic libraries will be found in:

  ..../bin/radx_runtime_libs

i.e. in a subdirectory of the bin directory.

Installing development release
------------------------------

If you want to compile and build against the Radx library, you will need the
development release which has the library and include files in addition to the
binary files.

To do this, use

  install_devel_release

will install in /usr/local/bin, /usr/local/lib and /usr/local/include.

Or, for example, 
  install_devel_release /opt/local

will install in /opt/local/bin, /opt/local/lib and /opt/local/include.

Sample configuration files.
---------------------------

The project files will be in:

  /usr/local/projects/radx/projDir

or

  /opt/local/projects/radx/projDir

or relative to wherever you performed the install.

The symbolic link:

  ~/projDir

will point to the project files.

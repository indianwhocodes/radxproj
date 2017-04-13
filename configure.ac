###############################################
#
# configure template for autoconf
#
# dir: .
#
# baseName: configure.base.shared
#
# written by script createConfigure.am.py
#
# created 2016-03-27 13:55:30.312163
#
###############################################
######### COPIED FROM BASE TEMPLATE ###########

###############################################
# base template for configure
###############################################

AC_PREREQ([2.50])
AC_INIT([radx], [1.0], [http://www.ral.ucar.edu/projects/titan/docs])

# Create the VERSION file, which contains the package version from
# AC_INIT.
echo -n AC_PACKAGE_VERSION>VERSION
AC_SUBST(PACKAGE_VERSION)
AC_MSG_NOTICE([netCDF AC_PACKAGE_VERSION])

# Keep libtool macros in an m4 directory.
AC_CONFIG_MACRO_DIR([m4])

# Find out about the host we're building on.
AC_CANONICAL_HOST

# We will output a config.h for the C library, and a fortran inc file,
# nfconfig.inc.
AC_CONFIG_HEADERS([config.h])

# initialize automake
AM_INIT_AUTOMAKE([-Wall foreign])


AC_MSG_NOTICE([checking user options])

# Does the user want a 32-bit build?
AC_ARG_ENABLE([build-32bit], [AS_HELP_STRING([--build-32bit],
              [build in 32-bit mode])])

# Did the user specify a location for the HDF5 library?
AC_MSG_CHECKING([whether a location for the HDF5 library was specified])
AC_ARG_WITH([hdf5],
              [AS_HELP_STRING([--with-hdf5=<directory>],
                              [Specify location of HDF5 library. Configure will \
			      expect to find subdirs include and lib.])],
            [HDF5DIR=$with_hdf5])
AC_MSG_RESULT([$HDF5DIR])
AC_SUBST(HDF5DIR, [$HDF5DIR])

# Did the user specify a location for the NETCDF library?
AC_MSG_CHECKING([whether a location for the NETCDF library was specified])
AC_ARG_WITH([netcdf],
              [AS_HELP_STRING([--with-netcdf=<directory>],
                              [Specify location of NETCDF library. Configure will \
			      expect to find subdirs include and lib.])],
            [NETCDFDIR=$with_netcdf])
AC_MSG_RESULT([$NETCDFDIR])
AC_SUBST(NETCDFDIR, [$NETCDFDIR])

# Did the user specify a location for the Radx library?
AC_MSG_CHECKING([whether a location for the Radx library was specified])
AC_ARG_WITH([radx],
              [AS_HELP_STRING([--with-radx=<directory>],
                              [Specify location of Radx library. Configure will \
                              expect to find subdirs include and lib.])],
            [RADXDIR=$with_radx])
AC_MSG_RESULT([$RADXDIR])
AC_SUBST(RADXDIR, [$RADXDIR])

# Did the user specify a location for the TBB library?
AC_MSG_CHECKING([whether a location for the tbb library was specified])
AC_ARG_WITH([tbb],
              [AS_HELP_STRING([--with-tbb=<directory>],
                              [Specify location of tbb library. Configure will \
			      expect to find subdirs include and lib.])],
            [TBBDIR=$with_tbb])
AC_MSG_RESULT([$TBBDIR])
AC_SUBST(TBBDIR, [$TBBDIR])


# Add the RADX directory to includes and link flags
if test ! "x$RADXDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${RADXDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${RADXDIR}/include"
fi




# Add the HDF5 directory to includes and link flags
if test ! "x$HDF5DIR" = x; then
   LDFLAGS="${LDFLAGS} -L${HDF5DIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${HDF5DIR}/include"
fi    

# Add the NETCDF directory to includes and link flags
if test ! "x$NETCDFDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${NETCDFDIR}/lib"
   CPPFLAGS="${CPPFLAGS} -I${NETCDFDIR}/include"
fi    

# Add the TBB directory to includes and link flags

if test ! "x$TBBDIR" = x; then
   LDFLAGS="${LDFLAGS} -L${TBBDIR}/lib/intel64/gcc4.7"
   CPPFLAGS="${CPPFLAGS} -I${TBBDIR}/include"
fi    

# Check for the existance of this file before proceeding.
# AC_CONFIG_SRCDIR([${NETCDFDIR}/include/Radx/Radx.hh])

# add lib dir relative to ORIGIN
# Note: @S|@ is a quadrigraph for $

# LDFLAGS="${LDFLAGS} -Wl,-rpath,'\@S|@\@S|@ORIGIN'/radx_runtime_libs"

# Set up libtool.
AM_PROG_AR
AC_MSG_NOTICE([setting up libtool])
LT_PREREQ([2.2])
LT_INIT
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_HEADER_STDC
AC_FUNC_STAT
AC_FUNC_UTIME_NULL
AC_CHECK_FUNCS([floor gettimeofday memset modf pow strerror])


############## DONE WITH BASE #################
###############################################


# create makefiles

AC_CONFIG_FILES([
  makefile
  apps/makefile
  apps/Radx/makefile
  apps/Radx/src/makefile
  apps/Radx/src/Radx2Grid/makefile
])
AC_OUTPUT


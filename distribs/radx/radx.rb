
require 'formula'

class radx < Formula
  homepage 'http://www.ral.ucar.edu/projects/titan/docs/radial_formats/radx.html'

  url 'ftp://ftp.rap.ucar.edu/pub/titan/radx/radx-20130315.src.tgz'
  version '20130315'
  sha1 'b83e14c25c1f565b38e95222d15b2d7f7b288a8d'

  depends_on 'hdf5'
  depends_on 'udunits'
  depends_on 'netcdf' => 'enable-cxx-compat'
  depends_on 'fftw'

  fails_with :clang do
    build 421
    cause "DsMdvx/msg_add.cc:516:11: error: '_printVsectWayPtsBuf' is a protected member of 'Mdvx'"
  end

  def install
    system "./configure", "--disable-dependency-tracking", "--prefix=#{prefix}"
    system "make install"
  end

  def test
    system "#{bin}/RadxPrint", "-h"
  end
end


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               libltfat
  vendor:           Zdenek Prusa, Peter L. Soendergaard
  version:          0.0.2
  name:             libltfat
  description:      The Large Time Frequency Analysis Toolbox Library
  website:          http://ltfat.github.io/libltfat
  license:          GPL

  searchpaths:      libltfat/modules/libltfat/include
    linuxLibs:      fftw3f

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

//==============================================================================
/** Config: NOBLASLAPACK

*/
#ifndef USEBLASLAPACK
#define USEBLASLAPACK 0
#endif

//==============================================================================
/** Config: USEFFTW

*/
#ifndef USEFFTW
#define USEFFTW 0
#endif

#if USEFFTW
#define FFTW 1
#else
#define KISS 1
#endif

#define LTFAT_SINGLE 1
#define LTFAT_API

#include "libltfat/modules/libltfat/include/ltfat.h"
#include "libpv/pv.h"

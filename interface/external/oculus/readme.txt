
Instructions for adding the Oculus library (LibOVR) to Interface
Stephen Birarda, March 6, 2014

You can download the Oculus SDK from https://developer.oculusvr.com/ (account creation required). Interface has been tested with SDK version 0.2.5.

1. Copy the Oculus SDK folders from the LibOVR directory (Lib, Include, Src) into the interface/externals/oculus folder.
   This readme.txt should be there as well.
   
   You may optionally choose to copy the SDK folders to a location outside the repository (so you can re-use with different checkouts and different projects). 
   If so our CMake find module expects you to set the ENV variable 'HIFI_LIB_DIR' to a directory containing a subfolder 'oculus' that contains the three folders mentioned above.
   
   NOTE: On OS X there is a linker error with version 0.2.5c of the Oculus SDK.
   It must be re-built (from the included LibOVR_With_Samples.xcodeproj) with RRTI support.
   In XCode Build Settings for the ovr target, set "Enable C++ Runtime Types" to yes.
   Then, Archive and use the organizer to save a copy of the built products.
   In the exported directory you will have a new libovr.a to copy into the oculus directory from above.

2. Clear your build directory, run cmake and build, and you should be all set.
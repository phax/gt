# gt
GetTyp/GetType - legacy file format detector

## Prerequisites for compiling
This application is designed to be compiled and run on Windows only.
  * Download Cygwin from https://cygwin.com/install.html
    * Despite the default setup also add the following:
      * make
      * perl
    * After installation ensure the following executables are not part of Cygwin installation (in e.g. C:\cygwin64\bin\)
      * dir.exe
      * link.exe
  * At least Visual C++ 7.1/2003/13.00 is required
    * The Platform SDK must be installed
  * The following makefile parameters are available:
    * `GT_DEBUG` with the value `0` or `1` - create a binary version with debug version or not. Default is `1`. If set to `1` the 'd' character is added to created files and folders.
    * `GT_UNICODE` with the value `0` or `1` - create a binary version with unicode or not. Default is `1`. If set to `1` the 'u' character is added to created files and folders.
      
### Using Visual Studio .NET 2003
  * Ensure `src\settings.mak` has set `GT_VC = 7`
  * Compile on the commandline by using batch file `src\_make_vs7.cmd`
  * The output is in the folder `target\7`
      
### Using Visual Studio 2005
  * Ensure `src\settings.mak` has set `GT_VC = 8`
  * Compile on the commandline by using batch file `src\_make_vs8.cmd`
  * The output is in the folder `target\8`
  
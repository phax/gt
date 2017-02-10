# gt
GetTyp/GetType/GT2 - legacy file format detector

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
  * The Platform SDK/Windows SDK must be installed
  * The following makefile parameters are available:
    * `GT_VC` - the version of the Visual C++ compiler to use. Automatically set in the `_make_*.cmd` files
    * `GT_DEBUG` with the value `0` or `1` - create a binary version with debug version or not. Default is `1`. If set to `1` the 'd' character is added to created files and folders.
    * `GT_UNICODE` with the value `0` or `1` - create a binary version with unicode or not. Default is `1`. If set to `1` the 'u' character is added to created files and folders.
      
### Using Visual Studio .NET 2003
  * Compile on the commandline by using batch file `src\_make_vs7.cmd`
  * The output is in the folder `target\7`
      
### Using Visual Studio 2005
  * Compile on the commandline by using batch file `src\_make_vs8.cmd`
  * The output is in the folder `target\8`
      
### Using Visual Studio 2008
  * Since that version the Platform SDK from `C:\Program Files\Microsoft SDKs\Windows` is used! 
  * Compile on the commandline by using batch file `src\_make_vs9.cmd`
  * The output is in the folder `target\9`
      
### Using Visual Studio 2010
  * Since that version the Platform SDK from `C:\Program Files\Microsoft SDKs\Windows` is used! 
  * Compile on the commandline by using batch file `src\_make_vs10.cmd`
  * The output is in the folder `target\10`
      
### Using Visual Studio 2012
  * Since that version the Platform SDK from `C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x64\` is used! 
  * Compile on the commandline by using batch file `src\_make_vs11.cmd`
  * The output is in the folder `target\11`

### Using Visual Studio 2015
  * The CRT was restructured 
    * See https://blogs.msdn.microsoft.com/vcblog/2015/03/03/introducing-the-universal-crt/
  * Please make sure to install the latest version of the Windows 10 SDK with the Universal CRT 
    * Download from https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk
  * The output is in the folder `target\14`

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

## Create an installer

All resources necessary to create an installer are in the directory `install`.
The installer requires binary versions compiled with VS 2005 by default.

The installation scripts are to be used by NullSoft Scriptable Install System (NSIS)
  http://nsis.sourceforge.net/

1. Download the latest version and install it
  * Tested with 2.39 and 2.45
2. Right click the "installer-gt2*.nsi" file and select "Compile NSIS Script"
3. Test the resulting "gt2setup-X.YY.exe" installer

----------

Check Microsoft KB 326922 for adding the runtime to the installer
  http://support.microsoft.com/kb/326922

=> VS2002/2003 include MSVCRT DLLs
=> VS2005 and later -> use external redistributables (see sub-directory)

----------

The following prerequisites are required (depending on the used compiler):
  Microsoft Visual C++ 2005 Redistributable Package (x86)
  http://www.microsoft.com/downloads/details.aspx?FamilyID=32BC1BEE-A3F9-4C13-9C99-220B62A191EE&displaylang=en

  Microsoft Visual C++ 2005 SP1 Redistributable Package (x86)
  http://www.microsoft.com/downloads/details.aspx?familyid=200B2FD9-AE1A-4A14-984D-389C36F85647&displaylang=en

  Microsoft Visual C++ 2008 Redistributable Package (x86)
  http://www.microsoft.com/downloads/details.aspx?FamilyID=9b2da534-3e03-4391-8a4d-074b9f2bc1bf&displaylang=en

  Microsoft Visual C++ 2008 SP1 Redistributable Package (x86)
  http://www.microsoft.com/downloads/details.aspx?familyid=A5C84275-3B97-4AB7-A40D-3802B2AF5FC2&displaylang=en

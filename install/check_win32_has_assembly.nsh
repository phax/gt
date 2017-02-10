;-------------------------------
; Test if Visual Studio Redistributables 2005+ SP1 installed
; Returns -1 if there is no VC redistributables intstalled
Function CheckVCRedist
   Push $R0
   ClearErrors
   ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{7299052b-02a4-4627-81f2-1818da5d550d}" "Version"

   ; if VS 2005+ redist SP1 not installed, install it
   IfErrors 0 VSRedistInstalled
   StrCpy $R0 "-1"

VSRedistInstalled:
   Exch $R0
FunctionEnd

;-------------------------------

Function WinSxS_HasAssembly ;legacyDllName,(Strong)AssemblyName
  /*
  push 'msvcr80.dll'
  push 'Microsoft.VC80.CRT,version="8.0.50727.42",type="win32",processorArchitecture="x86",publicKeyToken="1fc8b3b9a1e18e3b"'
  call WinSxS_HasAssembly
  pop $0 ;0 on fail, 1 if it is in WinSxS or 2 if LoadLibrary worked on pre xp (call GetDLLVersion to make sure if you require a minimum version)
  */
  Exch $8
  Exch
  Exch $7
  push $9
  StrCpy $9 0
  push $0
  push $R0
  push $R1
  ${If} ${AtLeastWinXP}
    System::Call "sxs::CreateAssemblyCache(*i.R0,i0)i.r0"
    ${If} $0 == 0
      System::Call '*(i 24,i0,l,i0,i0)i.R1' ;TODO,BUGBUG: check alloc success
      System::Call `$R0->4(i 0,w '$8',i $R1)i.r0` ;IAssemblyCache::QueryAssemblyInfo
      ${If} $0 == 0
        System::Call '*$R1(i,i.r0)'
        IntOp $0 $0 & 1 ;ASSEMBLYINFO_FLAG_INSTALLED=1
        ${IfThen} $0 <> 0 ${|} StrCpy $9 1 ${|}
        ${EndIf}
      System::Free $R1
      System::Call $R0->2() ;IAssemblyCache::Release
    ${EndIf}
  ${Else}
    System::Call kernel32::LoadLibrary(t"$7")i.r0
    ${If} $0 != 0
      StrCpy $9 2
      System::Call 'kernel32::FreeLibrary(i r0)'
    ${EndIf}
  ${EndIf}
  pop $R1
  pop $R0
  pop $0
  Exch 2
  pop $8
  pop $7
  Exch $9
FunctionEnd

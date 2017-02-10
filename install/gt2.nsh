; Usage:
;   Call IsWindowsNT
;   Pop $R0
;   ; at this point $R0 is "1" or "0"
Function IsWindowsNT
 
   Push $R0
 
   ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion

   IfErrors 0 lbl_yep
   
   StrCpy $R0 "0"
   Goto lbl_done
 
lbl_yep:
   Strcpy $R0 "1"

lbl_done:
   Exch $R0
 
 FunctionEnd

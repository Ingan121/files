#SingleInstance Force

Menu, Tray, NoStandard
Menu, Tray, Add, &Ignore ApplicationFrameHost, Toggle1
Menu, Tray, ToggleCheck, &Ignore ApplicationFrameHost
ignoreAFH := true
Menu, Tray, Add, Re&verting Mode, Toggle2
Menu, Tray, Add
Menu, Tray, Standard

While (true) {
   WinWait, A
   WinGet, ProcessName, ProcessName
   if (!ignoreAFH || ProcessName != "ApplicationFrameHost.exe")
   if (revert) {
      DllCall("dwmapi\DwmSetWindowAttribute","uint",WinExist(),"uint",2,"int*",0,"uint",4)
   } else {
      DllCall("dwmapi\DwmSetWindowAttribute","uint",WinExist(),"uint",2,"int*",1,"uint",4)
   }
}

Toggle1:
	Menu, %A_ThisMenu%, ToggleCheck, %A_ThisMenuItem%
    if (ignoreAFH) {
        ignoreAFH := false
    } else {
        IgnoreAFH := true
    }
	return
	
Toggle2:
	Menu, %A_ThisMenu%, ToggleCheck, %A_ThisMenuItem%
    if (revert) {
        revert := false
    } else {
        revert := true
    }
	return
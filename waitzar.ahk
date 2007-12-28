; 
; Copyright 2007 by Seth N. Hetu
; 
; Please refer to the end of the file for licensing information
; 

; NOTE: This script is obsolete; refer to the Win32_source folder for the most
;  recent, C++ source.


;;Configuration
k_FontSize = 10
k_FontName = Zawgyi-One
k_FontStyle = Bold
k_Myanmar = &Myanmar
k_English = &English
k_Exit = E&xit


;; Add the item to the tray's menu
Menu, Tray, Add, %k_English%, cmd_SwitchLanguage
Menu, Tray, Add, %k_Exit%, cmd_Exit
Menu, Tray, Default, %k_English%
Menu, Tray, NoStandard


;; Create a GUI window
Gui, Font, s%k_FontSize% %k_FontStyle%, %k_FontName%
Gui, -Caption +E0x200 +ToolWindow
TransColor = F1ECED
Gui, Color, %TransColor%  ; This color will be made transparent later below.

;;Add components
Txt := "?????? ?????? "

Ansi2Unicode(Txt, Txt, 65001)

Loop, parse, Txt
{
	Tmp := Asc(A_LoopField)
    MsgBox, %Tmp%
}



;CharStr = 1000
;Txt .= Chr(CharStr)
Gui, Add, Edit, r2 vTest, %Txt%
Gui, Show

;ToolTip, Hi, 50, 50

return ; End of auto-execute section.



cmd_SwitchLanguage:
return


;ButtonGoOut:
GuiClose:
cmd_Exit:
ExitApp





Ansi2UTF8(sString)
{
   Ansi2Unicode(sString, wString, 0)
   Unicode2Ansi(wString, zString, 65001)
   Return zString
}
UTF82Ansi(zString)
{
   Ansi2Unicode(zString, wString, 65001)
   Unicode2Ansi(wString, sString, 0)
   Return sString
}
Ansi2Unicode(ByRef sString, ByRef wString, CP = 0)
{
     nSize := DllCall("MultiByteToWideChar"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &sString
      , "int",  -1
      , "Uint", 0
      , "int",  0)

   VarSetCapacity(wString, nSize * 2)

   DllCall("MultiByteToWideChar"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &sString
      , "int",  -1
      , "Uint", &wString
      , "int",  nSize)
}
Unicode2Ansi(ByRef wString, ByRef sString, CP = 0)
{
     nSize := DllCall("WideCharToMultiByte"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &wString
      , "int",  -1
      , "Uint", 0
      , "int",  0
      , "Uint", 0
      , "Uint", 0)

   VarSetCapacity(sString, nSize)

   DllCall("WideCharToMultiByte"
      , "Uint", CP
      , "Uint", 0
      , "Uint", &wString
      , "int",  -1
      , "str",  sString
      , "int",  nSize
      , "Uint", 0
      , "Uint", 0)
}





















; 
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
; 
; http://www.apache.org/licenses/LICENSE-2.0
; 
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
; 
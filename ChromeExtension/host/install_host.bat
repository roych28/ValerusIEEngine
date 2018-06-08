:: Copyright 2014 The Chromium Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

:: Change HKCU to HKLM if you want to install globally.
:: %~dp0 is the directory containing this bat script and ends with a backslash.
REG ADD "HKCU\Software\Google\Chrome\NativeMessagingHosts\com.vicon.valerus.app" /ve /t REG_SZ /d "%~dp0com.vicon.valerus.app.json" /f
REG ADD "HKCU\Software\Microsoft\Internet Explorer\Main\FeatureControl\FEATURE_BROWSER_EMULATION" /v ValerusChromeEngine.exe /t REG_DWORD /d 11000 /f
REG ADD "HKCU\SOFTWARE\Policies\Google\Chrome" /v  HardwareAccelerationModeEnabled /t REG_DWORD /d 0 /f
REG ADD "HKCU\Software\Microsoft\Internet Explorer\Main" /v TabProcGrowth /t REG_SZ /d "1" /f

pause
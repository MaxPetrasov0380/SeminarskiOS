@echo off
REM ============================================
REM  pokreni_programe.bat
REM  Pokrece vise programa odjednom
REM ============================================

echo Pokrecem programe...

REM Primer 1: Notepad
start "" "notepad.exe"

REM Primer 2: Calculator
start "" "calc.exe"

REM Primer 3: Chrome (podesi putanju ako je drugacija)
start "" "C:\Program Files\BraveSoftware\Brave-Browser\Application\brave.exe"

REM Primer 4: Neki tvoj program (izmeni putanju)
start "" "C:\Users\Korisnik\AppData\Local\GitHubDesktop\GitHubDesktop.exe"

echo.
echo Svi programi su pokrenuti.
timeout /t 2 >nul
exit /b 0

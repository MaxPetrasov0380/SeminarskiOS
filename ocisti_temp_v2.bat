@echo off
setlocal enabledelayedexpansion
REM ============================================================
REM  ocisti_temp_v2.bat
REM  Cisti temp fajlove i kes, i BROJI koliko je fajlova/foldera
REM  preskoceno jer su bili u upotrebi (zakljucani).
REM
REM  NAPOMENA (fix): del /q /f i rd /s /q ne postavljaju pouzdan
REM  ERRORLEVEL kada fajl/folder ne moze da se obrise zbog toga
REM  sto je u upotrebi. Zato se posle pokusaja brisanja proverava
REM  da li fajl/folder JOS UVEK postoji (if exist) - to je
REM  pouzdan nacin da se utvrdi da li je brisanje stvarno uspelo.
REM ============================================================

echo Ciscenje temp fajlova i kesa u toku...
echo.

set ukupno_obrisano=0
set ukupno_preskoceno=0

REM ------------------------------------------------------------
REM  Funkcija (label) za ciscenje jednog foldera
REM  %1 = putanja do foldera koji cistimo
REM ------------------------------------------------------------
call :ocistiFolder "%TEMP%" "Korisnicki TEMP"
call :ocistiFolder "C:\Windows\Temp" "Windows TEMP (treba admin)"
call :ocistiFolder "C:\Windows\Prefetch" "Prefetch (treba admin)"

echo.
echo ========================================
echo   REZULTAT CISCENJA
echo   Uspesno obrisano: %ukupno_obrisano%
echo   Preskoceno (u upotrebi/zabranjeno): %ukupno_preskoceno%
echo ========================================
echo.

timeout /t 3 >nul
exit /b 0

REM ============================================================
REM  :ocistiFolder <putanja> <naziv_za_prikaz>
REM  Prolazi kroz svaki fajl pojedinacno, pokusava da ga obrise,
REM  i proverava da li fajl/folder i dalje postoji da bi znala
REM  da li je brisanje stvarno uspelo.
REM ============================================================
:ocistiFolder
set "putanja=%~1"
set "naziv=%~2"
echo --- Ciscenje: %naziv% ---

REM --- Brisanje fajlova, jedan po jedan ---
for %%F in ("%putanja%\*.*") do (
    del /q /f "%%F" >nul 2>&1
    if exist "%%F" (
        set /a ukupno_preskoceno+=1
    ) else (
        set /a ukupno_obrisano+=1
    )
)

REM --- Brisanje podfoldera, jedan po jedan ---
for /d %%D in ("%putanja%\*") do (
    rd /s /q "%%D" >nul 2>&1
    if exist "%%D" (
        set /a ukupno_preskoceno+=1
    ) else (
        set /a ukupno_obrisano+=1
    )
)

echo Zavrseno: %naziv%
echo.
exit /b 0
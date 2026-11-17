@echo off
setlocal
pushd "%~dp0.." || exit /b 2
set "WORK=%CD%\oracle\.build\fault-%RANDOM%-%RANDOM%"
if exist "%WORK%" goto fail
mkdir "%WORK%" || goto fail
set "MUTANT_CPP=%WORK%\mutant.cpp"
py -3 -c "import os,pathlib; p=pathlib.Path('src/structB/scapegoat_tree.cpp'); s=p.read_text(); old='    n--;'; assert s.splitlines().count(old)==1, 'upstream deletion changed: inspect the fixture'; pathlib.Path(os.environ['MUTANT_CPP']).write_text(s.replace(old,'    if (key != 17) n--;'))" || goto fail
set "CXX=g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc/structB"
%CXX% oracle\differential.cpp src\structA\hibbard_bst.cpp "%MUTANT_CPP%" -o "%WORK%\demo.exe" || goto fail
%CXX% oracle\shrink.cpp src\structA\hibbard_bst.cpp "%MUTANT_CPP%" -o "%WORK%\reduce.exe" || goto fail
set "FIXTURE=%CD%\oracle\shrink_fixture.txt"
cd /d "%WORK%" || goto fail
demo.exe replay "%FIXTURE%"
if not "%ERRORLEVEL%"=="1" goto fail
reduce.exe oracle-failure-1\prefix.txt minimal.txt || goto fail
type minimal.txt
demo.exe replay minimal.txt
if not "%ERRORLEVEL%"=="1" goto fail
echo PASS fault demonstration. Files in "%WORK%"
popd
exit /b 0
:fail
echo Fault demonstration did not complete.
popd
exit /b 1

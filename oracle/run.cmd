@echo off
setlocal
pushd "%~dp0.." || exit /b 2
set "OUT=oracle\.build"
if not exist "%OUT%\" mkdir "%OUT%" || goto fail
set "CXX=g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic"
set "TREES=src\structA\hibbard_bst.cpp src\structB\scapegoat_tree.cpp"
%CXX% oracle\test_sorted_array.cpp -o "%OUT%\sorted.exe" || goto fail
"%OUT%\sorted.exe" || goto fail
%CXX% oracle\test_invariants.cpp %TREES% -o "%OUT%\invariants.exe" || goto fail
"%OUT%\invariants.exe" || goto fail
%CXX% oracle\differential.cpp %TREES% -o "%OUT%\oracle.exe" || goto fail
%CXX% oracle\shrink.cpp %TREES% -o "%OUT%\shrink.exe" || goto fail
%CXX% oracle\test_d4.cpp src\structA\hibbard_bst.cpp -o "%OUT%\d4.exe" || goto fail
"%OUT%\d4.exe" || goto fail
"%OUT%\oracle.exe" mixed 20261117 1000000 || goto fail
"%OUT%\oracle.exe" replay oracle\shrink_fixture.txt || goto fail
if "%~1"=="" goto done
if /i not "%~1"=="full" goto usage
"%OUT%\oracle.exe" churn 1000 1000000 42 random || goto fail
"%OUT%\oracle.exe" churn 2000 4000000 42 random || goto fail
"%OUT%\oracle.exe" churn 4000 16000000 42 random || goto fail
:done
popd
exit /b 0
:usage
echo Usage: oracle\run.cmd [full]
:fail
popd
exit /b 1

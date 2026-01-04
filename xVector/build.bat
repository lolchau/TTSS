@echo off
chcp 65001 >nul
echo ============================================================
echo    BIEN DICH CHUONG TRINH NHAN MA TRAN VOI VECTOR
echo ============================================================
echo.

REM Kiểm tra MinGW hoặc Visual Studio
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [1] Phat hien g++ (MinGW/TDM-GCC)
    echo Dang bien dich voi g++...
    echo.
    g++ -fopenmp -O3 -Wall -std=c++11 matrix_vector_multiply.cpp -o matrix_multiply.exe
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ============================================================
        echo    BIEN DICH THANH CONG!
        echo ============================================================
        echo.
        echo Chay chuong trinh: matrix_multiply.exe
        echo Hoac chay: run.bat
        echo.
    ) else (
        echo.
        echo [LOI] Bien dich that bai!
        echo Vui long kiem tra OpenMP da duoc cai dat.
        echo.
    )
    goto end
)

where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [2] Phat hien cl (Visual Studio)
    echo Dang bien dich voi Visual Studio...
    echo.
    cl /EHsc /openmp /O2 matrix_vector_multiply.cpp /Fe:matrix_multiply.exe
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ============================================================
        echo    BIEN DICH THANH CONG!
        echo ============================================================
        echo.
        echo Chay chuong trinh: matrix_multiply.exe
        echo Hoac chay: run.bat
        echo.
    ) else (
        echo.
        echo [LOI] Bien dich that bai!
        echo.
    )
    goto end
)

echo [LOI] Khong tim thay g++ hoac cl!
echo.
echo Vui long cai dat mot trong cac compiler sau:
echo   1. MinGW-w64: https://winlibs.com/
echo   2. TDM-GCC: https://jmeubank.github.io/tdm-gcc/
echo   3. Visual Studio: https://visualstudio.microsoft.com/
echo.

:end
pause
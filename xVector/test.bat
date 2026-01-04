@echo off
chcp 65001 >nul

:menu
cls
echo ============================================================
echo    TEST CHUONG TRINH NHAN MA TRAN VOI VECTOR
echo ============================================================
echo.
echo Chon kich ban test:
echo.
echo [1] Test nhanh (500x500)
echo [2] Test trung binh (2000x2000)
echo [3] Test lon (5000x5000)
echo [4] So sanh so luong
echo [5] Quay lai
echo.
set /p choice="Nhap lua chon (1-5): "

if "%choice%"=="1" goto test_small
if "%choice%"=="2" goto test_medium
if "%choice%"=="3" goto test_large
if "%choice%"=="4" goto compare_threads
if "%choice%"=="5" goto end
echo Lua chon khong hop le!
pause
goto menu

:test_small
cls
echo ============================================================
echo    TEST NHANH - MA TRAN 500x500
echo ============================================================
echo.
echo MATRIX_ROWS=500 > matrix_config.txt
echo MATRIX_COLS=500 >> matrix_config.txt
echo NUM_THREADS=4 >> matrix_config.txt
echo RUN_SEQUENTIAL=true >> matrix_config.txt
echo RUN_1D_PARTITION=true >> matrix_config.txt
echo RUN_2D_PARTITION=true >> matrix_config.txt
echo RUN_RING_TOPOLOGY=true >> matrix_config.txt
echo Da tao file config cho ma tran 500x500
echo.
call run.bat
goto menu

:test_medium
cls
echo ============================================================
echo    TEST TRUNG BINH - MA TRAN 2000x2000
echo ============================================================
echo.
echo MATRIX_ROWS=2000 > matrix_config.txt
echo MATRIX_COLS=2000 >> matrix_config.txt
echo NUM_THREADS=4 >> matrix_config.txt
echo RUN_SEQUENTIAL=true >> matrix_config.txt
echo RUN_1D_PARTITION=true >> matrix_config.txt
echo RUN_2D_PARTITION=true >> matrix_config.txt
echo RUN_RING_TOPOLOGY=true >> matrix_config.txt
echo Da tao file config cho ma tran 2000x2000
echo.
call run.bat
goto menu

:test_large
cls
echo ============================================================
echo    TEST LON - MA TRAN 5000x5000
echo ============================================================
echo.
echo MATRIX_ROWS=5000 > matrix_config.txt
echo MATRIX_COLS=5000 >> matrix_config.txt
echo NUM_THREADS=8 >> matrix_config.txt
echo RUN_SEQUENTIAL=true >> matrix_config.txt
echo RUN_1D_PARTITION=true >> matrix_config.txt
echo RUN_2D_PARTITION=true >> matrix_config.txt
echo RUN_RING_TOPOLOGY=true >> matrix_config.txt
echo Da tao file config cho ma tran 5000x5000
echo.
call run.bat
goto menu

:compare_threads
cls
echo ============================================================
echo    SO SANH SO LUONG: 1, 2, 4, 8
echo ============================================================
echo.

for %%t in (1 2 4 8) do (
    echo.
    echo -------------------------------------------------------
    echo Test voi %%t luong...
    echo -------------------------------------------------------
    echo MATRIX_ROWS=2000 > matrix_config.txt
    echo MATRIX_COLS=2000 >> matrix_config.txt
    echo NUM_THREADS=%%t >> matrix_config.txt
    echo RUN_SEQUENTIAL=true >> matrix_config.txt
    echo RUN_1D_PARTITION=true >> matrix_config.txt
    echo RUN_2D_PARTITION=false >> matrix_config.txt
    echo RUN_RING_TOPOLOGY=false >> matrix_config.txt
    matrix_multiply.exe
    timeout /t 2 >nul
)

echo.
echo ============================================================
echo    HOAN THANH SO SANH!
echo ============================================================
pause
goto menu

:end
echo.
echo Tam biet!
timeout /t 2 >nul
exit
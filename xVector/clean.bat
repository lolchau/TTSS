@echo off
chcp 65001 >nul
echo ============================================================
echo    DON DEP FILE TAM
echo ============================================================
echo.

echo Dang xoa cac file tam...
echo.

REM Xóa file thực thi
if exist matrix_multiply.exe (
    del /Q matrix_multiply.exe
    echo [OK] Da xoa matrix_multiply.exe
)

REM Xóa file object
if exist *.obj (
    del /Q *.obj
    echo [OK] Da xoa cac file .obj
)

if exist *.o (
    del /Q *.o
    echo [OK] Da xoa cac file .o
)

REM Xóa file Python đồ thị
if exist plot_matrix_chart.py (
    del /Q plot_matrix_chart.py
    echo [OK] Da xoa plot_matrix_chart.py
)

REM Xóa file ảnh đồ thị
if exist matrix_vector_performance.png (
    del /Q matrix_vector_performance.png
    echo [OK] Da xoa matrix_vector_performance.png
)

REM Xóa file tạm của Visual Studio
if exist *.pdb (
    del /Q *.pdb
    echo [OK] Da xoa cac file .pdb
)

if exist *.ilk (
    del /Q *.ilk
    echo [OK] Da xoa cac file .ilk
)

echo.
echo ============================================================
echo    DON DEP HOAN TAT!
echo ============================================================
echo.

set /p delete_config="Ban co muon xoa file config khong? (Y/N): "
if /i "%delete_config%"=="Y" (
    if exist matrix_config.txt (
        del /Q matrix_config.txt
        echo [OK] Da xoa matrix_config.txt
        echo.
    )
)

echo Tam biet!
timeout /t 2 >nul
@echo off
chcp 65001 >nul
echo ============================================================
echo    CHAY CHUONG TRINH NHAN MA TRAN VOI VECTOR
echo ============================================================
echo.

REM Kiểm tra file thực thi
if not exist matrix_multiply.exe (
    echo [LOI] Khong tim thay file matrix_multiply.exe
    echo Vui long bien dich truoc: build.bat
    echo.
    pause
    exit /b 1
)

REM Kiểm tra file config
if not exist matrix_config.txt (
    echo [CANH BAO] Khong tim thay matrix_config.txt
    echo Su dung gia tri mac dinh.
    echo.
)

REM Chạy chương trình
echo Dang chay chuong trinh...
echo.
matrix_multiply.exe

echo.
echo ============================================================
echo    HOAN THANH!
echo ============================================================
echo.

REM Hỏi có muốn xem đồ thị không
set /p show_chart="Ban co muon xem do thi khong? (Y/N): "
if /i "%show_chart%"=="Y" (
    if exist plot_matrix_chart.py (
        echo.
        echo Dang hien thi do thi...
        python plot_matrix_chart.py
    ) else (
        echo.
        echo [LOI] Khong tim thay file plot_matrix_chart.py
        echo Vui long chay chuong trinh truoc.
    )
)

echo.
pause
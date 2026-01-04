# CHƯƠNG TRÌNH NHÂN MA TRẬN VỚI VECTOR - WINDOWS

## 📖 Mục lục

1. [Mô tả chương trình](#mô-tả-chương-trình)
2. [Các thuật toán](#các-thuật-toán)
3. [Cài đặt cho Windows](#cài-đặt-cho-windows)
4. [Cách sử dụng](#cách-sử-dụng)
5. [File cấu hình](#file-cấu-hình)
6. [Kết quả và đánh giá](#kết-quả-và-đánh-giá)

---

## 📋 Mô tả chương trình

Chương trình tính toán tích của ma trận với vector bằng OpenMP trên **Windows**, triển khai 4 phương pháp:

- **1.2.1 Nhân tuần tự**: Thuật toán cơ bản (baseline)
- **1.2.2 Nhân song song phân hoạch 1 chiều**: Chia hàng cho các luồng
- **1.2.3 Nhân song song phân hoạch 2 chiều**: Chia cả hàng và cột
- **1.2.4 Nhân song song kết nối vòng**: Ring topology với load balancing
- **1.2.5 So sánh speedup**: Đánh giá hiệu suất bằng đồ thị

### Tính năng chính:

✅ **Tương thích Windows 10/11**  
✅ **Hỗ trợ nhiều compiler**: MinGW, TDM-GCC, Visual Studio  
✅ **Script .bat** tự động biên dịch và chạy  
✅ **1.1.1 Tạo ma trận và vector ngẫu nhiên**  
✅ So sánh hiệu suất giữa tuần tự và song song  
✅ Xuất đồ thị trực quan  
✅ File config dễ dàng thay đổi tham số  

---

## 🔬 Các thuật toán

### 1. Nhân tuần tự (Sequential)

```
for i = 0 to rows:
    result[i] = 0
    for j = 0 to cols:
        result[i] += matrix[i][j] * vector[j]
```

**Đặc điểm:**
- Chạy trên 1 luồng duy nhất
- Làm baseline để so sánh
- Độ phức tạp: O(m×n)

### 2. Phân hoạch 1 chiều (1D Partition)

```
#pragma omp parallel for
for i = 0 to rows:
    result[i] = 0
    for j = 0 to cols:
        result[i] += matrix[i][j] * vector[j]
```

**Đặc điểm:**
- Chia các hàng cho các luồng
- Hiệu suất cao, dễ triển khai
- Không cần đồng bộ

### 3. Phân hoạch 2 chiều (2D Partition)

**Đặc điểm:**
- Chia cả hàng và cột thành các khối
- Tối ưu cache locality
- Phù hợp với ma trận lớn

### 4. Kết nối vòng (Ring Topology)

**Đặc điểm:**
- Các luồng xử lý dữ liệu theo vòng tròn
- Load balancing tốt
- Phù hợp cho ma trận không đồng nhất

---

## 🛠️ Cài đặt cho Windows

### Bước 1: Cài đặt Compiler (chọn 1 trong 3)

#### Tùy chọn 1: MinGW-w64 (Khuyến nghị) ⭐

1. Tải về từ: https://winlibs.com/
2. Chọn bản **UCRT runtime** (Win64 - GCC với POSIX threads)
3. Giải nén vào `C:\mingw64`
4. Thêm vào PATH:
   - Mở **Settings** → Tìm "Environment Variables"
   - Chỉnh sửa **Path**, thêm: `C:\mingw64\bin`
5. Kiểm tra:
   ```cmd
   g++ --version
   ```

#### Tùy chọn 2: TDM-GCC

1. Tải về từ: https://jmeubank.github.io/tdm-gcc/
2. Chạy installer và chọn **MinGW-w64/TDM64**
3. Chọn tất cả components (bao gồm OpenMP)
4. Kiểm tra:
   ```cmd
   g++ --version
   ```

#### Tùy chọn 3: Visual Studio

1. Tải **Visual Studio Community**: https://visualstudio.microsoft.com/
2. Trong installer, chọn **Desktop development with C++**
3. Đảm bảo chọn **C++ OpenMP support**
4. Mở **Developer Command Prompt for VS** để sử dụng

### Bước 2: Cài đặt Python (để vẽ đồ thị)

1. Tải Python từ: https://www.python.org/downloads/
2. **QUAN TRỌNG**: Tick vào "Add Python to PATH" khi cài đặt
3. Cài đặt libraries:
   ```cmd
   pip install matplotlib numpy
   ```

### Bước 3: Kiểm tra cài đặt

Mở Command Prompt và chạy:

```cmd
g++ --version
python --version
```

Nếu cả 2 lệnh đều hiển thị phiên bản → Cài đặt thành công! ✅

---

## 🚀 Cách sử dụng

### Cách 1: Sử dụng Batch Scripts (Dễ nhất) 🎯

```cmd
REM Bước 1: Biên dịch chương trình
build.bat

REM Bước 2: (Tùy chọn) Chỉnh sửa file cấu hình
notepad matrix_config.txt

REM Bước 3: Chạy chương trình
run.bat

REM Hoặc sử dụng menu test
test.bat
```

### Cách 2: Biên dịch thủ công

#### Với MinGW/TDM-GCC:

```cmd
g++ -fopenmp -O3 -Wall -std=c++11 matrix_vector_multiply.cpp -o matrix_multiply.exe
```

#### Với Visual Studio:

Mở **Developer Command Prompt for VS**:

```cmd
cl /EHsc /openmp /O2 matrix_vector_multiply.cpp /Fe:matrix_multiply.exe
```

### Cách 3: Sử dụng Visual Studio IDE

1. Tạo **Console App** mới
2. Copy code vào file `.cpp`
3. Bật OpenMP:
   - **Project** → **Properties**
   - **C/C++** → **Language** → **Open MP Support** → **Yes (/openmp)**
4. Build và Run (F5)

---

## ⚙️ File cấu hình

File `matrix_config.txt`:

```txt
# Kích thước ma trận
MATRIX_ROWS=1000
MATRIX_COLS=1000

# Số luồng (số core CPU hoặc 2x số core)
NUM_THREADS=4

# Chọn phương pháp chạy (true/false)
RUN_SEQUENTIAL=true
RUN_1D_PARTITION=true
RUN_2D_PARTITION=true
RUN_RING_TOPOLOGY=true
```

### Kiểm tra số core CPU:

```cmd
echo %NUMBER_OF_PROCESSORS%
```

hoặc mở Task Manager → Performance → CPU → Cores

---

## 📊 Kết quả và đánh giá

### Các file batch script:

| Script | Chức năng |
|--------|-----------|
| `build.bat` | Biên dịch chương trình |
| `run.bat` | Chạy chương trình và hiển thị đồ thị |
| `test.bat` | Menu test với nhiều kịch bản |
| `clean.bat` | Xóa file tạm |

### Menu test.bat:

```
[1] Test nhanh (500x500)        - Kiểm tra nhanh
[2] Test trung bình (2000x2000) - Cân bằng
[3] Test lớn (5000x5000)        - Hiệu suất thực
[4] So sánh số luồng            - Test 1,2,4,8 luồng
```

### Đầu ra chương trình:

```
+----------------------------------------------------------+
|  NHAN SONG SONG - Phan hoach 1 chieu                     |
+----------------------------------------------------------+
| Kich thuoc ma tran:    1000x1000                         |
| So luong:              4                                 |
| Thoi gian:             0.003421 giay                     |
| Toc do (Speedup):      3.654                             |
| Hieu suat:             91.35 %                           |
| Ket qua dung:          Dung                              |
+----------------------------------------------------------+
```

### Công thức đánh giá:

**Speedup:**
```
S = Thời gian tuần tự / Thời gian song song
```

**Efficiency:**
```
E = Speedup / Số luồng × 100%
```

---

## 🎨 Xem đồ thị

Sau khi chạy chương trình:

```cmd
python plot_matrix_chart.py
```

Đồ thị sẽ hiển thị:
1. So sánh thời gian thực thi
2. Tốc độ tăng tốc (Speedup)

---

## 💡 Các lưu ý cho Windows

### Về Performance:

- **Windows Defender**: Có thể làm chậm chương trình. Tạm tắt khi test hiệu suất.
- **Background Apps**: Đóng các ứng dụng không cần thiết
- **Power Plan**: Chọn "High Performance" trong Power Options

### Về bộ nhớ:

Ma trận m×n cần RAM:
```
RAM ≈ m × n × 8 bytes
```

Ví dụ:
- 1000×1000: ~8 MB ✅
- 5000×5000: ~200 MB ✅
- 10000×10000: ~800 MB ⚠️ (cần 16GB+ RAM)

### Khuyến nghị số luồng:

```
Số luồng lý tưởng = Số physical cores
Tối đa = Số logical processors (với HyperThreading)
```

Ví dụ: CPU 4 cores 8 threads → Test với 1, 2, 4, 8 luồng

---

## 🐛 Xử lý lỗi trên Windows

### Lỗi: "g++ is not recognized"

**Nguyên nhân**: Chưa thêm MinGW vào PATH

**Giải pháp**:
1. Tìm thư mục chứa `g++.exe` (thường `C:\mingw64\bin`)
2. Thêm vào PATH:
   - Windows 10/11: Settings → System → About → Advanced system settings
   - Environment Variables → Path → Edit → New
   - Thêm đường dẫn đến folder `bin`
3. Khởi động lại Command Prompt

### Lỗi: "cannot find -lgomp"

**Nguyên nhân**: OpenMP không được cài đặt với compiler

**Giải pháp**:
- Cài đặt lại MinGW và chọn **all packages**
- Hoặc cài TDM-GCC (đã bao gồm OpenMP)
- Hoặc dùng Visual Studio với `/openmp` flag

### Lỗi: Python "No module named 'matplotlib'"

**Giải pháp**:
```cmd
python -m pip install --upgrade pip
pip install matplotlib numpy
```

### Lỗi: "System cannot execute the specified program"

**Giải pháp**:
- Cài đặt **Visual C++ Redistributable**:
  - Tải từ: https://aka.ms/vs/17/release/vc_redist.x64.exe
  - Chạy và cài đặt

### Chương trình chạy quá chậm

**Giải pháp**:
1. Giảm kích thước ma trận trong `matrix_config.txt`
2. Tắt Windows Defender tạm thời
3. Đóng các ứng dụng nền
4. Chuyển Power Plan sang "High Performance"

---

## 📂 Cấu trúc thư mục

```
matrix_project/
│
├── matrix_vector_multiply.cpp    # Code chính
├── matrix_config.txt              # File cấu hình
│
├── build.bat                      # Script biên dịch
├── run.bat                        # Script chạy
├── test.bat                       # Script test menu
├── clean.bat                      # Script dọn dẹp
│
├── matrix_multiply.exe            # File thực thi (sau khi build)
├── plot_matrix_chart.py           # Script đồ thị (tự động tạo)
└── matrix_vector_performance.png  # Đồ thị kết quả
```

---

## 📚 Tài nguyên tham khảo

### Compiler Downloads:

- **MinGW-w64**: https://winlibs.com/
- **TDM-GCC**: https://jmeubank.github.io/tdm-gcc/
- **Visual Studio**: https://visualstudio.microsoft.com/

### Documentation:

- **OpenMP**: https://www.openmp.org/
- **Matrix Multiplication**: https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm

---

## 👨‍🏫 Hướng dẫn cho giáo viên

### Test nhanh:

```cmd
REM 1. Biên dịch
build.bat

REM 2. Chạy menu test
test.bat

REM 3. Chọn kịch bản test từ menu
```

### Thay đổi tham số:

1. Mở `matrix_config.txt` bằng Notepad
2. Thay đổi giá trị
3. Lưu và chạy lại `run.bat`

### So sánh hiệu suất:

```cmd
REM Chạy test.bat và chọn [4] So sánh số luồng
test.bat
```

---

## 🔧 Script clean.bat

Tạo file `clean.bat`:

```batch
@echo off
echo Dang don dep...
del /Q matrix_multiply.exe 2>nul
del /Q *.obj 2>nul
del /Q *.o 2>nul
del /Q plot_matrix_chart.py 2>nul
del /Q matrix_vector_performance.png 2>nul
echo Da xoa cac file tam!
pause
```

---

## 📞 Hỗ trợ

### Kiểm tra hệ thống:

```cmd
REM Kiểm tra compiler
g++ --version
cl

REM Kiểm tra Python
python --version

REM Kiểm tra số core
echo %NUMBER_OF_PROCESSORS%

REM Kiểm tra RAM
wmic ComputerSystem get TotalPhysicalMemory
```

### Nếu gặp vấn đề:

1. Kiểm tra compiler đã cài đúng
2. Kiểm tra PATH environment variable
3. Chạy Command Prompt **as Administrator**
4. Cài đặt Visual C++ Redistributable

---

**Phát triển bởi**: OpenMP & Parallel Computing  
**Platform**: Windows 10/11  
**Ngày tạo**: 2026  
**Mục đích**: Học tập và nghiên cứu về tính toán song song
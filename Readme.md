# BÀI TẬP LỚN – TÍNH TOÁN SONG SONG SỐ PI BẰNG MONTE CARLO (OPENMP)

---

## 1. Giới thiệu

Chương trình này thực hiện **tính xấp xỉ số π (PI)** bằng **thuật toán Monte Carlo**, được **song song hóa bằng OpenMP**.  
Mục tiêu của chương trình là **so sánh hiệu năng giữa phiên bản tuần tự và song song**, phục vụ cho **bài tập lớn môn Tính toán song song**.

Chương trình hỗ trợ **2 kịch bản thử nghiệm**:
- **Kịch bản 1**: Cố định dữ liệu, thay đổi số luồng
- **Kịch bản 2**: Cố định số luồng, thay đổi kích thước dữ liệu

Toàn bộ tham số được điều khiển thông qua **file cấu hình `config.txt`**, không cần chỉnh sửa mã nguồn khi chạy thử các kịch bản khác nhau.

---

## 2. Thuật toán sử dụng – Monte Carlo

Thuật toán Monte Carlo ước lượng số π dựa trên xác suất hình học:

- Sinh ngẫu nhiên các điểm `(x, y)` trong hình vuông đơn vị `[0,1] × [0,1]`
- Đếm số điểm nằm bên trong đường tròn bán kính 1
- Công thức xấp xỉ:

\[
\pi \approx 4 \times \frac{\text{số điểm trong đường tròn}}{\text{tổng số điểm}}
\]

Các phép thử là **độc lập**, do đó thuật toán **rất phù hợp để song song hóa**.

---

## 3. Yêu cầu hệ thống (Windows)

- **Hệ điều hành**: Windows 10 / Windows 11 (64-bit)
- **Trình biên dịch**: MinGW-w64 `g++` (có hỗ trợ OpenMP)
- **Python 3** (tùy chọn – để vẽ đồ thị)
  - Thư viện: `matplotlib`, `numpy`

---

## 4. Cấu trúc thư mục

```text
TTSS/
│── main.cpp        # Mã nguồn chính
│── main.exe        # File thực thi (sau khi biên dịch)
│── config.txt      # File cấu hình kịch bản
│── plot_chart.py   # Script vẽ đồ thị
│── README.md       # File hướng dẫn này

## 5. Cách chạy
compile : g++ -std=c++20 -fopenmp main.cpp -o main

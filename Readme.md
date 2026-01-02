# CHƯƠNG TRÌNH TÍNH TOÁN SONG SONG SỐ PI – MONTE CARLO (OPENMP)

## 📌 Giới thiệu

Đây là chương trình tính xấp xỉ số **π (PI)** bằng **thuật toán Monte Carlo**, được song song hóa bằng **OpenMP**.  
Chương trình được xây dựng phục vụ cho **bài tập lớn môn Tính toán song song**.

Chương trình hỗ trợ **2 kịch bản đánh giá hiệu suất**:
- **Kịch bản 1**: Cố định dữ liệu, thay đổi số luồng
- **Kịch bản 2**: Cố định số luồng, thay đổi kích thước dữ liệu

Toàn bộ tham số chạy được cấu hình thông qua file `config.txt`, **không cần chỉnh sửa mã nguồn** khi thử nghiệm các kịch bản khác nhau.

---

## 🧮 Thuật toán sử dụng – Monte Carlo

Thuật toán Monte Carlo ước lượng số π dựa trên xác suất hình học:

- Sinh ngẫu nhiên các điểm `(x, y)` trong hình vuông đơn vị `[0,1] × [0,1]`
- Đếm số điểm rơi vào đường tròn bán kính 1
- Công thức xấp xỉ:

\[
\pi \approx 4 \times \frac{\text{số điểm trong đường tròn}}{\text{tổng số điểm}}
\]

Do các phép thử **độc lập nhau**, thuật toán này **rất phù hợp để song song hóa**.

---

## ⚙️ Yêu cầu hệ thống (Windows)

- **Hệ điều hành**: Windows 10 / Windows 11 (64-bit)
- **Trình biên dịch**: MinGW-w64 `g++` (có hỗ trợ OpenMP)
- **Python 3** (tùy chọn – để vẽ đồ thị)
  - Thư viện: `matplotlib`, `numpy`

---

## 📦 Cấu trúc thư mục

```text
TTSS/
│── main.cpp        # Mã nguồn chính
│── main.exe        # File thực thi (sau khi biên dịch)
│── config.txt      # File cấu hình kịch bản chạy
│── plot_chart.py   # Vẽ đồ thị hiệu suất (tùy chọn)
│── README.md       # File hướng dẫn

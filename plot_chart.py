import matplotlib
matplotlib.use("TkAgg")

import matplotlib.pyplot as plt
import csv
import os
from collections import defaultdict

FILENAME = "results.csv"

if not os.path.exists(FILENAME):
    print("❌ Khong tim thay results.csv")
    exit(1)

# =====================
# Đọc dữ liệu
# =====================
samples = []
threads = []
time_seq = []
time_par = []
speedup = []

with open(FILENAME, "r", encoding="utf-8") as f:
    reader = csv.reader(f)
    header = next(reader)

    for row in reader:
        samples.append(int(row[1]))
        threads.append(int(row[2]))
        time_seq.append(float(row[4]))
        time_par.append(float(row[5]))
        speedup.append(float(row[6]))

# =====================
# Nhận diện kịch bản
# =====================
unique_samples = set(samples)
unique_threads = set(threads)

# =====================
# KỊCH BẢN 1
# =====================
if len(unique_samples) == 1 and len(unique_threads) > 1:
    print(">>> Phat hien KICH BAN 1: Co dinh du lieu, doi so luong")

    x = threads

    # --- Đồ thị thời gian ---
    plt.figure(figsize=(8, 5))
    plt.plot(x, time_par, marker="o", label="Song song")
    plt.axhline(y=time_seq[0], linestyle="--", label="Tuan tu")
    plt.xlabel("So luong (threads)")
    plt.ylabel("Thoi gian (s)")
    plt.title("Kich ban 1: Thoi gian theo so luong")
    plt.legend()
    plt.grid(True)
    plt.savefig("scenario1_time.png", dpi=300)
    plt.show()

    # --- Đồ thị speedup ---
    plt.figure(figsize=(8, 5))
    plt.plot(x, speedup, marker="o")
    plt.xlabel("So luong (threads)")
    plt.ylabel("Speedup")
    plt.title("Kich ban 1: Speedup theo so luong")
    plt.grid(True)
    plt.savefig("scenario1_speedup.png", dpi=300)
    plt.show()

# =====================
# KỊCH BẢN 2
# =====================
elif len(unique_threads) == 1 and len(unique_samples) > 1:
    print(">>> Phat hien KICH BAN 2: Co dinh so luong, doi du lieu")

    x = [s / 1e6 for s in samples]

    # --- Đồ thị thời gian ---
    plt.figure(figsize=(8, 5))
    plt.plot(x, time_seq, marker="o", label="Tuan tu")
    plt.plot(x, time_par, marker="s", label="Song song")
    plt.xlabel("So mau (trieu)")
    plt.ylabel("Thoi gian (s)")
    plt.title("Kich ban 2: Thoi gian theo du lieu")
    plt.legend()
    plt.grid(True)
    plt.savefig("scenario2_time.png", dpi=300)
    plt.show()

    # --- Đồ thị speedup ---
    plt.figure(figsize=(8, 5))
    plt.plot(x, speedup, marker="o")
    plt.xlabel("So mau (trieu)")
    plt.ylabel("Speedup")
    plt.title("Kich ban 2: Speedup theo du lieu")
    plt.grid(True)
    plt.savefig("scenario2_speedup.png", dpi=300)
    plt.show()

else:
    print("⚠️ Du lieu khong phu hop ro rang cho kich ban 1 hoac 2")

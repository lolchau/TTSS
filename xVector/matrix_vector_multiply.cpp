#include <iostream>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <windows.h>

using namespace std;

// Struct để lưu cấu hình
struct Config {
    int rows;                  // Số hàng ma trận
    int cols;                  // Số cột ma trận
    int num_threads;           // Số luồng
    bool run_sequential;       // Chạy tuần tự
    bool run_1d_partition;     // Chạy phân hoạch 1 chiều
    bool run_2d_partition;     // Chạy phân hoạch 2 chiều
    bool run_ring_topology;    // Chạy kết nối vòng
};

// Hỗ trợ UTF-8 cho Windows Console
void enableUTF8Console() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

// Đọc file config
Config readConfig(const string& filename) {
    Config config;
    // Giá trị mặc định
    config.rows = 2000;
    config.cols = 2000;
    config.num_threads = 4;
    config.run_sequential = true;
    config.run_1d_partition = true;
    config.run_2d_partition = true;
    config.run_ring_topology = true;
    
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Khong tim thay file config. Su dung gia tri mac dinh." << endl;
        return config;
    }
    
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line.find("MATRIX_ROWS=") == 0) {
            config.rows = stoi(line.substr(12));
        }
        else if (line.find("MATRIX_COLS=") == 0) {
            config.cols = stoi(line.substr(12));
        }
        else if (line.find("NUM_THREADS=") == 0) {
            config.num_threads = stoi(line.substr(12));
        }
        else if (line.find("RUN_SEQUENTIAL=") == 0) {
            string val = line.substr(15);
            // Trim whitespace
            size_t first = val.find_first_not_of(' ');
            if (string::npos != first) {
                size_t last = val.find_last_not_of(' ');
                val = val.substr(first, (last - first + 1));
            }
            config.run_sequential = (val == "true" || val == "1");
        }
        else if (line.find("RUN_1D_PARTITION=") == 0) {
            string val = line.substr(17);
            size_t first = val.find_first_not_of(' ');
            if (string::npos != first) {
                size_t last = val.find_last_not_of(' ');
                val = val.substr(first, (last - first + 1));
            }
            config.run_1d_partition = (val == "true" || val == "1");
        }
        else if (line.find("RUN_2D_PARTITION=") == 0) {
            string val = line.substr(17);
            size_t first = val.find_first_not_of(' ');
            if (string::npos != first) {
                size_t last = val.find_last_not_of(' ');
                val = val.substr(first, (last - first + 1));
            }
            config.run_2d_partition = (val == "true" || val == "1");
        }
        else if (line.find("RUN_RING_TOPOLOGY=") == 0) {
            string val = line.substr(18);
            size_t first = val.find_first_not_of(' ');
            if (string::npos != first) {
                size_t last = val.find_last_not_of(' ');
                val = val.substr(first, (last - first + 1));
            }
            config.run_ring_topology = (val == "true" || val == "1");
        }
    }
    
    file.close();
    return config;
}

// Tạo ma trận ngẫu nhiên
void createRandomMatrix(double* &mat, int rows, int cols) {
    mat = new double[(long long)rows * cols];
    for (long long i = 0; i < (long long)rows * cols; i++) {
        mat[i] = (double)rand() / RAND_MAX * 2.0;
    }
}

// Tạo vector ngẫu nhiên
void createRandomVector(double* &vec, int size) {
    vec = new double[size];
    for (int i = 0; i < size; i++) {
        vec[i] = (double)rand() / RAND_MAX * 2.0;
    }
}

// In mẫu ma trận
void printMatrixSample(double* mat, int rows, int cols, int sample = 5) {
    int r = min(rows, sample);
    int c = min(cols, sample);
    cout << "Ma tran [" << rows << "x" << cols << "] (mau " << r << "x" << c << "):" << endl;
    for (int i = 0; i < r; i++) {
        cout << "  ";
        for (int j = 0; j < c; j++) {
            cout << fixed << setprecision(2) << setw(6) << mat[i * cols + j] << " ";
        }
        if (cols > c) cout << "...";
        cout << endl;
    }
    if (rows > r) cout << "  ..." << endl;
    cout << endl;
}

// In một phần của vector
void printVectorSample(double* vec, int size, int sample_size = 10) {
    int print_size = min(sample_size, size);
    cout << "Vector [" << size << "] (mau " << print_size << " phan tu):" << endl;
    cout << "  [";
    for (int i = 0; i < print_size; i++) {
        cout << fixed << setprecision(2) << vec[i];
        if (i < print_size - 1) cout << ", ";
    }
    if (print_size < size) cout << ", ...";
    cout << "]" << endl << endl;
}

// So sánh 2 kết quả vector
bool compareResults(double* res1, double* res2, int size, double epsilon = 1e-3) {
    for (int i = 0; i < size; i++) {
        if (abs(res1[i] - res2[i]) > epsilon) return false;
    }
    return true;
}

// 1.2.1 Nhân ma trận - vector tuần tự
void multiplySequential(double* mat, double* vec, double* res, int rows, int cols, double& time_taken) {
    double start_time = omp_get_wtime();
    for (int i = 0; i < rows; i++) {
        res[i] = 0.0;
        for (int j = 0; j < cols; j++) {
            res[i] += mat[i * cols + j] * vec[j];
        }
    }
    time_taken = omp_get_wtime() - start_time;
}

// 1.2.2 Nhân ma trận - vector song song phân hoạch 1 chiều (Row-wise)
void multiply1DPartition(double* mat, double* vec, double* res, int rows, int cols, 
                          int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    #pragma omp parallel for num_threads(num_threads)
    for (int i = 0; i < rows; i++) {
        double sum = 0.0;
        for (int j = 0; j < cols; j++) {
            sum += mat[i * cols + j] * vec[j];
        }
        res[i] = sum;
    }
    time_taken = omp_get_wtime() - start_time;
}

// 1.2.3 Nhân ma trận - vector song song phân hoạch 2 chiều (Block-wise)
// Chia ma trận thành lưới các khối
void multiply2DPartition(double* mat, double* vec, double* res, int rows, int cols, 
                          int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    // Khởi tạo kết quả bằng 0
    for(int i = 0; i < rows; i++) res[i] = 0.0;

    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        
        // Giả sử chia theo lưới (cực kỳ đơn giản cho mục đích minh họa)
        // Mỗi luồng xử lý một dải cột cho tất cả các hàng, sau đó cộng dồn
        int block_cols = (cols + nt - 1) / nt;
        int start_c = tid * block_cols;
        int end_c = min(start_c + block_cols, cols);
        
        if (start_c < cols) {
            vector<double> local_res(rows, 0.0);
            for (int i = 0; i < rows; i++) {
                for (int j = start_c; j < end_c; j++) {
                    local_res[i] += mat[i * cols + j] * vec[j];
                }
            }
            
            // Critical để gộp kết quả vào res
            #pragma omp critical
            {
                for (int i = 0; i < rows; i++) {
                    res[i] += local_res[i];
                }
            }
        }
    }
    time_taken = omp_get_wtime() - start_time;
}

// 1.2.4 Nhân ma trận - vector kết nối vòng (Ring topology simulation)
void multiplyRingTopology(double* mat, double* vec, double* res, int rows, int cols, 
                           int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    for(int i = 0; i < rows; i++) res[i] = 0.0;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        int nt = omp_get_num_threads();
        
        int rows_per_thread = (rows + nt - 1) / nt;
        int start_r = tid * rows_per_thread;
        int end_r = min(start_r + rows_per_thread, rows);
        
        int cols_per_step = (cols + nt - 1) / nt;
        
        if (start_r < rows) {
            // Ring simulation: Mỗi luồng tính toán phần hàng của mình
            // Bằng cách xoay vòng qua các khối cột của vector
            for (int step = 0; step < nt; step++) {
                int current_col_block = (tid + step) % nt;
                int start_c = current_col_block * cols_per_step;
                int end_c = min(start_c + cols_per_step, cols);
                
                if (start_c < cols) {
                    for (int i = start_r; i < end_r; i++) {
                        for (int j = start_c; j < end_c; j++) {
                            res[i] += mat[i * cols + j] * vec[j];
                        }
                    }
                }
                // Đồng bộ hóa giữa các bước (mô phỏng chuyển dữ liệu trong ring)
                #pragma omp barrier
            }
        }
    }
    
    time_taken = omp_get_wtime() - start_time;
}

// In kết quả
void printResult(const string& method, int rows, int cols, int threads, 
                double* result, double time_taken, double speedup, 
                double efficiency, bool is_correct, bool is_sequential = false) {
    
    // Tính tổng kết quả để hiển thị đại diện
    double sum = 0;
    for(int i = 0; i < rows; i++) sum += result[i];

    cout << "+----------------------------------------------------------+" << endl;
    cout << "|  " << left << setw(56) << method << "|" << endl;
    cout << "+----------------------------------------------------------+" << endl;
    cout << "| Ma tran:               " << setw(10) << rows << " x " << left << setw(19) << cols << "|" << endl;
    if (!is_sequential) {
        cout << "| So luong luong:        " << setw(32) << threads << "|" << endl;
    }
    cout << "| Tong vector ket qua:   " << setw(32) << fixed << setprecision(4) << sum << "|" << endl;
    cout << "| Thoi gian:             " << setw(26) << fixed << setprecision(6) 
         << time_taken << " giay |" << endl;
    if (!is_sequential) {
        cout << "| Toc do (Speedup):      " << setw(32) << fixed << setprecision(3) 
             << speedup << "|" << endl;
        cout << "| Hieu suat:             " << setw(29) << fixed << setprecision(2) 
             << (efficiency * 100) << " % |" << endl;
    }
    cout << "| Trang thai:            " << setw(32) << (is_correct ? "CHINH XAC" : "CO LOI") << "|" << endl;
    cout << "+----------------------------------------------------------+" << endl << endl;
}

// Xuất dữ liệu cho đồ thị Python
void exportChartData(const string& filename,
                    const vector<string>& methods,
                    const vector<double>& times,
                    const vector<double>& speedups) {
    ofstream file(filename);
    
    file << "# -*- coding: utf-8 -*-" << endl;
    file << "import matplotlib.pyplot as plt" << endl;
    file << "import numpy as np" << endl;
    file << "plt.rcParams['font.family'] = 'Arial'" << endl << endl;
    
    file << "methods = [";
    for (size_t i = 0; i < methods.size(); i++) {
        file << "'" << methods[i] << "'";
        if (i < methods.size() - 1) file << ", ";
    }
    file << "]" << endl;
    
    file << "times = [";
    for (size_t i = 0; i < times.size(); i++) {
        file << times[i];
        if (i < times.size() - 1) file << ", ";
    }
    file << "]" << endl;
    
    file << "speedups = [";
    for (size_t i = 0; i < speedups.size(); i++) {
        file << speedups[i];
        if (i < speedups.size() - 1) file << ", ";
    }
    file << "]" << endl << endl;
    
    file << "fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5))" << endl << endl;
    
    file << "colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#FFA07A', '#98D8C8']" << endl;
    file << "x_pos = np.arange(len(methods))" << endl << endl;
    
    file << "bars1 = ax1.bar(x_pos, times, color=colors[:len(methods)])" << endl;
    file << "ax1.set_xlabel('Phuong phap', fontsize=12)" << endl;
    file << "ax1.set_ylabel('Thoi gian (giay)', fontsize=12)" << endl;
    file << "ax1.set_title('So sanh thoi gian thuc thi - Tich vo huong vector', fontsize=14, fontweight='bold')" << endl;
    file << "ax1.set_xticks(x_pos)" << endl;
    file << "ax1.set_xticklabels(methods, rotation=15, ha='right', fontsize=9)" << endl;
    file << "ax1.grid(True, alpha=0.3, axis='y')" << endl;
    file << "for bar in bars1:" << endl;
    file << "    height = bar.get_height()" << endl;
    file << "    ax1.text(bar.get_x() + bar.get_width()/2., height," << endl;
    file << "            f'{height:.6f}', ha='center', va='bottom', fontsize=8)" << endl << endl;
    
    file << "speedup_methods = methods[1:]" << endl;
    file << "speedup_values = speedups[1:]" << endl;
    file << "x_pos2 = np.arange(len(speedup_methods))" << endl;
    file << "bars2 = ax2.bar(x_pos2, speedup_values, color=colors[1:len(methods)])" << endl;
    file << "ax2.axhline(y=1, color='r', linestyle='--', label='Baseline', alpha=0.5)" << endl;
    file << "ax2.set_xlabel('Phuong phap song song', fontsize=12)" << endl;
    file << "ax2.set_ylabel('Toc do (Speedup)', fontsize=12)" << endl;
    file << "ax2.set_title('Toc do tang toc so voi tuan tu', fontsize=14, fontweight='bold')" << endl;
    file << "ax2.set_xticks(x_pos2)" << endl;
    file << "ax2.set_xticklabels(speedup_methods, rotation=15, ha='right', fontsize=9)" << endl;
    file << "ax2.legend(fontsize=10)" << endl;
    file << "ax2.grid(True, alpha=0.3, axis='y')" << endl;
    file << "for bar in bars2:" << endl;
    file << "    height = bar.get_height()" << endl;
    file << "    ax2.text(bar.get_x() + bar.get_width()/2., height," << endl;
    file << "            f'{height:.2f}x', ha='center', va='bottom', fontsize=8)" << endl << endl;
    
    file << "plt.tight_layout()" << endl;
    file << "plt.savefig('matrix_vector_performance.png', dpi=300, bbox_inches='tight')" << endl;
    file << "plt.show()" << endl;
    file << "print('\\nDo thi da duoc luu vao: matrix_vector_performance.png')" << endl;
    
    file.close();
}

int main() {
    enableUTF8Console();
    srand((unsigned int)time(NULL));
    
    cout << "\n+============================================================+" << endl;
    cout << "|     CHUONG TRINH NHAN MA TRAN - VECTOR (OpenMP)           |" << endl;
    cout << "+============================================================+\n" << endl;
    
    // Đọc cấu hình
    Config config = readConfig("matrix_config.txt");
    
    cout << "================ CAU HINH =================" << endl;
    cout << "Kich thuoc ma tran: " << config.rows << " x " << config.cols << endl;
    cout << "So luong luong:     " << config.num_threads << endl;
    cout << "Phuong phap chay:" << endl;
    if (config.run_sequential) cout << "  - Tuan tu" << endl;
    if (config.run_1d_partition) cout << "  - Phan hoach 1 chieu (Row-wise)" << endl;
    if (config.run_2d_partition) cout << "  - Phan hoach 2 chieu (Block-wise)" << endl;
    if (config.run_ring_topology) cout << "  - Ket noi vong (Ring topology)" << endl;
    cout << "==========================================\n" << endl;
    
    // Cấp phát và tạo dữ liệu
    cout << "-> Dang khoi tao ma tran va vector..." << endl;
    double* A;
    double* x;
    double* res_seq = new double[config.rows];
    double* res_par = new double[config.rows];
    
    createRandomMatrix(A, config.rows, config.cols);
    createRandomVector(x, config.cols);
    
    cout << "Hoan tat!\n" << endl;
    
    printMatrixSample(A, config.rows, config.cols);
    printVectorSample(x, config.cols);
    
    cout << "\n=============== BAT DAU TINH TOAN ===============\n" << endl;
    
    vector<string> methods;
    vector<double> times;
    vector<double> speedups;
    double time_seq = 1e-9; // Tránh chia cho 0
    
    // 1.2.1 Tuần tự
    if (config.run_sequential) {
        cout << "-> Dang thuc hien: TUAN TU..." << endl;
        multiplySequential(A, x, res_seq, config.rows, config.cols, time_seq);
        printResult("NHAN MA TRAN TUAN TU", config.rows, config.cols, 
                   0, res_seq, time_seq, 1.0, 1.0, true, true);
        
        methods.push_back("Tuan tu");
        times.push_back(time_seq);
        speedups.push_back(1.0);
    }
    
    // 1.2.2 1D Partition
    if (config.run_1d_partition) {
        cout << "-> Dang thuc hien: PHAN HOACH 1 CHIEU..." << endl;
        double t;
        multiply1DPartition(A, x, res_par, config.rows, config.cols, config.num_threads, t);
        
        double speedup = time_seq / t;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareResults(res_seq, res_par, config.rows);
        
        printResult("PHAN HOACH 1 CHIEU (Row-wise)", config.rows, config.cols, 
                   config.num_threads, res_par, t, speedup, efficiency, is_correct);
        
        methods.push_back("1D Partition");
        times.push_back(t);
        speedups.push_back(speedup);
    }
    
    // 1.2.3 2D Partition
    if (config.run_2d_partition) {
        cout << "-> Dang thuc hien: PHAN HOACH 2 CHIEU..." << endl;
        double t;
        multiply2DPartition(A, x, res_par, config.rows, config.cols, config.num_threads, t);
        
        double speedup = time_seq / t;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareResults(res_seq, res_par, config.rows);
        
        printResult("PHAN HOACH 2 CHIEU (Block-wise)", config.rows, config.cols, 
                   config.num_threads, res_par, t, speedup, efficiency, is_correct);
        
        methods.push_back("2D Partition");
        times.push_back(t);
        speedups.push_back(speedup);
    }
    
    // 1.2.4 Ring Topology
    if (config.run_ring_topology) {
        cout << "-> Dang thuc hien: KET NOI VONG..." << endl;
        double t;
        multiplyRingTopology(A, x, res_par, config.rows, config.cols, config.num_threads, t);
        
        double speedup = time_seq / t;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareResults(res_seq, res_par, config.rows);
        
        printResult("KET NOI VONG (Ring Topology Simulation)", config.rows, config.cols, 
                   config.num_threads, res_par, t, speedup, efficiency, is_correct);
        
        methods.push_back("Ring Topology");
        times.push_back(t);
        speedups.push_back(speedup);
    }
    
    // Bảng so sánh
    cout << "\n=============== BANG SO SANH HIEU NANG ===============" << endl;
    cout << "+--------------------------------+-------------+--------------+" << endl;
    cout << "| Phuong phap                    | Thoi gian   | Speedup      |" << endl;
    cout << "+--------------------------------+-------------+--------------+" << endl;
    for (size_t i = 0; i < methods.size(); i++) {
        cout << "| " << left << setw(30) << methods[i] << " | " 
             << fixed << setprecision(6) << setw(11) << times[i] << " | "
             << fixed << setprecision(3) << setw(12) << speedups[i] << " |" << endl;
    }
    cout << "+--------------------------------+-------------+--------------+" << endl;
    
    // Xuất đồ thị
    exportChartData("plot_matrix_chart.py", methods, times, speedups);
    
    // Giải phóng
    delete[] A;
    delete[] x;
    delete[] res_seq;
    delete[] res_par;
    
    cout << "\n Hoan thanh! Nhan Enter de ket thuc." << endl;
    cin.get();
    return 0;
}
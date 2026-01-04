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
    int matrix_rows;           // Số hàng ma trận
    int matrix_cols;           // Số cột ma trận (cũng là kích thước vector)
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
    config.matrix_rows = 1000;
    config.matrix_cols = 1000;
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
            config.matrix_rows = stoi(line.substr(12));
        }
        else if (line.find("MATRIX_COLS=") == 0) {
            config.matrix_cols = stoi(line.substr(12));
        }
        else if (line.find("NUM_THREADS=") == 0) {
            config.num_threads = stoi(line.substr(12));
        }
        else if (line.find("RUN_SEQUENTIAL=") == 0) {
            string val = line.substr(15);
            config.run_sequential = (val == "true" || val == "1");
        }
        else if (line.find("RUN_1D_PARTITION=") == 0) {
            string val = line.substr(17);
            config.run_1d_partition = (val == "true" || val == "1");
        }
        else if (line.find("RUN_2D_PARTITION=") == 0) {
            string val = line.substr(17);
            config.run_2d_partition = (val == "true" || val == "1");
        }
        else if (line.find("RUN_RING_TOPOLOGY=") == 0) {
            string val = line.substr(18);
            config.run_ring_topology = (val == "true" || val == "1");
        }
    }
    
    file.close();
    return config;
}

// Tạo ma trận ngẫu nhiên
void createRandomMatrix(double** &matrix, int rows, int cols) {
    matrix = new double*[rows];
    for (int i = 0; i < rows; i++) {
        matrix[i] = new double[cols];
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = (double)rand() / RAND_MAX * 10.0;
        }
    }
}

// Tạo vector ngẫu nhiên
void createRandomVector(double* &vec, int size) {
    vec = new double[size];
    for (int i = 0; i < size; i++) {
        vec[i] = (double)rand() / RAND_MAX * 10.0;
    }
}

// Giải phóng ma trận
void freeMatrix(double** matrix, int rows) {
    for (int i = 0; i < rows; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

// In một phần của ma trận (để kiểm tra)
void printMatrixSample(double** matrix, int rows, int cols, int sample_size = 5) {
    int print_rows = min(sample_size, rows);
    int print_cols = min(sample_size, cols);
    
    cout << "Ma tran [" << rows << "x" << cols << "] (hien thi " 
         << print_rows << "x" << print_cols << " phan tu dau):" << endl;
    for (int i = 0; i < print_rows; i++) {
        cout << "  ";
        for (int j = 0; j < print_cols; j++) {
            cout << fixed << setprecision(2) << setw(7) << matrix[i][j] << " ";
        }
        if (print_cols < cols) cout << "...";
        cout << endl;
    }
    if (print_rows < rows) cout << "  ..." << endl;
    cout << endl;
}

// In một phần của vector
void printVectorSample(double* vec, int size, int sample_size = 10) {
    int print_size = min(sample_size, size);
    
    cout << "Vector [" << size << "] (hien thi " << print_size << " phan tu dau):" << endl;
    cout << "  [";
    for (int i = 0; i < print_size; i++) {
        cout << fixed << setprecision(2) << vec[i];
        if (i < print_size - 1) cout << ", ";
    }
    if (print_size < size) cout << ", ...";
    cout << "]" << endl << endl;
}

// So sánh 2 vector (để kiểm tra tính đúng đắn)
bool compareVectors(double* v1, double* v2, int size, double epsilon = 1e-6) {
    for (int i = 0; i < size; i++) {
        if (abs(v1[i] - v2[i]) > epsilon) {
            return false;
        }
    }
    return true;
}

// 1.2.1 Nhân tuần tự
void multiplySequential(double** matrix, double* vec, double* result, 
                       int rows, int cols, double& time_taken) {
    double start_time = omp_get_wtime();
    
    for (int i = 0; i < rows; i++) {
        result[i] = 0.0;
        for (int j = 0; j < cols; j++) {
            result[i] += matrix[i][j] * vec[j];
        }
    }
    
    time_taken = omp_get_wtime() - start_time;
}

// 1.2.2 Nhân song song phân hoạch 1 chiều (phân phối hàng)
void multiply1DPartition(double** matrix, double* vec, double* result, 
                        int rows, int cols, int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    #pragma omp parallel num_threads(num_threads)
    {
        #pragma omp for schedule(static)
        for (int i = 0; i < rows; i++) {
            result[i] = 0.0;
            for (int j = 0; j < cols; j++) {
                result[i] += matrix[i][j] * vec[j];
            }
        }
    }
    
    time_taken = omp_get_wtime() - start_time;
}

// 1.2.3 Nhân song song phân hoạch 2 chiều (phân phối cả hàng và cột)
void multiply2DPartition(double** matrix, double* vec, double* result, 
                        int rows, int cols, int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    // Khởi tạo result
    for (int i = 0; i < rows; i++) {
        result[i] = 0.0;
    }
    
    // Phân hoạch 2 chiều: chia ma trận thành các khối
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        int num_t = omp_get_num_threads();
        
        // Tính kích thước khối cho mỗi luồng
        int block_rows = (rows + num_t - 1) / num_t;
        int start_row = tid * block_rows;
        int end_row = min(start_row + block_rows, rows);
        
        // Mỗi luồng xử lý một khối hàng
        for (int i = start_row; i < end_row; i++) {
            double sum = 0.0;
            for (int j = 0; j < cols; j++) {
                sum += matrix[i][j] * vec[j];
            }
            result[i] = sum;
        }
    }
    
    time_taken = omp_get_wtime() - start_time;
}

// 1.2.4 Nhân song song phân hoạch 1 chiều - Kết nối vòng (Ring topology)
void multiplyRingTopology(double** matrix, double* vec, double* result, 
                         int rows, int cols, int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    // Khởi tạo result
    for (int i = 0; i < rows; i++) {
        result[i] = 0.0;
    }
    
    int block_size = (cols + num_threads - 1) / num_threads;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        int num_t = omp_get_num_threads();
        
        // Mỗi luồng xử lý một phần của vector
        double* local_result = new double[rows];
        for (int i = 0; i < rows; i++) {
            local_result[i] = 0.0;
        }
        
        // Ring topology: mỗi luồng xử lý các cột theo vòng tròn
        for (int round = 0; round < num_t; round++) {
            int current_block = (tid + round) % num_t;
            int start_col = current_block * block_size;
            int end_col = min(start_col + block_size, cols);
            
            // Tính tích cho khối cột hiện tại
            for (int i = 0; i < rows; i++) {
                for (int j = start_col; j < end_col; j++) {
                    local_result[i] += matrix[i][j] * vec[j];
                }
            }
            
            #pragma omp barrier
        }
        
        // Gộp kết quả
        #pragma omp critical
        {
            for (int i = 0; i < rows; i++) {
                result[i] += local_result[i];
            }
        }
        
        delete[] local_result;
    }
    
    time_taken = omp_get_wtime() - start_time;
}

// In kết quả
void printResult(const string& method, int rows, int cols, int threads, 
                double time_taken, double speedup, double efficiency, 
                bool is_correct, bool is_sequential = false) {
    cout << "+----------------------------------------------------------+" << endl;
    cout << "|  " << left << setw(56) << method << "|" << endl;
    cout << "+----------------------------------------------------------+" << endl;
    cout << "| Kich thuoc ma tran:    " << setw(32) << (to_string(rows) + "x" + to_string(cols)) << "|" << endl;
    if (!is_sequential) {
        cout << "| So luong:              " << setw(32) << threads << "|" << endl;
    }
    cout << "| Thoi gian:             " << setw(26) << fixed << setprecision(6) 
         << time_taken << " giay |" << endl;
    if (!is_sequential) {
        cout << "| Toc do (Speedup):      " << setw(32) << fixed << setprecision(3) 
             << speedup << "|" << endl;
        cout << "| Hieu suat:             " << setw(29) << fixed << setprecision(2) 
             << (efficiency * 100) << " % |" << endl;
    }
    cout << "| Ket qua dung:          " << setw(32) << (is_correct ? "Dung" : "SAI") << "|" << endl;
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
    file << "ax1.set_title('So sanh thoi gian thuc thi', fontsize=14, fontweight='bold')" << endl;
    file << "ax1.set_xticks(x_pos)" << endl;
    file << "ax1.set_xticklabels(methods, rotation=15, ha='right', fontsize=9)" << endl;
    file << "ax1.grid(True, alpha=0.3, axis='y')" << endl;
    file << "for bar in bars1:" << endl;
    file << "    height = bar.get_height()" << endl;
    file << "    ax1.text(bar.get_x() + bar.get_width()/2., height," << endl;
    file << "            f'{height:.4f}', ha='center', va='bottom', fontsize=8)" << endl << endl;
    
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
    cout << "|     CHUONG TRINH NHAN MA TRAN VOI VECTOR - OpenMP         |" << endl;
    cout << "+============================================================+\n" << endl;
    
    // Đọc cấu hình
    Config config = readConfig("matrix_config.txt");
    
    cout << "================ CAU HINH =================" << endl;
    cout << "Kich thuoc ma tran: " << config.matrix_rows << "x" << config.matrix_cols << endl;
    cout << "So luong: " << config.num_threads << endl;
    cout << "Phuong phap chay:" << endl;
    if (config.run_sequential) cout << "  - Tuan tu" << endl;
    if (config.run_1d_partition) cout << "  - Phan hoach 1 chieu" << endl;
    if (config.run_2d_partition) cout << "  - Phan hoach 2 chieu" << endl;
    if (config.run_ring_topology) cout << "  - Ket noi vong" << endl;
    cout << "==========================================\n" << endl;
    
    // 1.1.1 Tạo ma trận và vector ngẫu nhiên
    cout << "-> Dang tao ma tran va vector ngau nhien..." << endl;
    double** matrix;
    double* vec;
    double* result_seq;
    double* result_1d;
    double* result_2d;
    double* result_ring;
    
    createRandomMatrix(matrix, config.matrix_rows, config.matrix_cols);
    createRandomVector(vec, config.matrix_cols);
    
    result_seq = new double[config.matrix_rows];
    result_1d = new double[config.matrix_rows];
    result_2d = new double[config.matrix_rows];
    result_ring = new double[config.matrix_rows];
    
    cout << "Hoan tat!\n" << endl;
    
    // Hiển thị mẫu dữ liệu
    printMatrixSample(matrix, config.matrix_rows, config.matrix_cols, 5);
    printVectorSample(vec, config.matrix_cols, 10);
    
    cout << "\n=============== BAT DAU TINH TOAN ===============\n" << endl;
    
    vector<string> methods;
    vector<double> times;
    vector<double> speedups;
    double time_seq = 0;
    
    // 1.2.1 Nhân tuần tự
    if (config.run_sequential) {
        cout << "-> Dang thuc hien nhan TUAN TU..." << endl;
        multiplySequential(matrix, vec, result_seq, config.matrix_rows, 
                          config.matrix_cols, time_seq);
        printResult("NHAN TUAN TU", config.matrix_rows, config.matrix_cols, 
                   0, time_seq, 0, 0, true, true);
        
        methods.push_back("Tuan tu");
        times.push_back(time_seq);
        speedups.push_back(1.0);
    }
    
    // 1.2.2 Nhân song song phân hoạch 1 chiều
    if (config.run_1d_partition) {
        cout << "-> Dang thuc hien nhan SONG SONG - Phan hoach 1 chieu..." << endl;
        double time_1d;
        multiply1DPartition(matrix, vec, result_1d, config.matrix_rows, 
                           config.matrix_cols, config.num_threads, time_1d);
        
        double speedup = time_seq / time_1d;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareVectors(result_seq, result_1d, config.matrix_rows);
        
        printResult("NHAN SONG SONG - Phan hoach 1 chieu", 
                   config.matrix_rows, config.matrix_cols, config.num_threads,
                   time_1d, speedup, efficiency, is_correct);
        
        methods.push_back("Song song 1D");
        times.push_back(time_1d);
        speedups.push_back(speedup);
    }
    
    // 1.2.3 Nhân song song phân hoạch 2 chiều
    if (config.run_2d_partition) {
        cout << "-> Dang thuc hien nhan SONG SONG - Phan hoach 2 chieu..." << endl;
        double time_2d;
        multiply2DPartition(matrix, vec, result_2d, config.matrix_rows, 
                           config.matrix_cols, config.num_threads, time_2d);
        
        double speedup = time_seq / time_2d;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareVectors(result_seq, result_2d, config.matrix_rows);
        
        printResult("NHAN SONG SONG - Phan hoach 2 chieu", 
                   config.matrix_rows, config.matrix_cols, config.num_threads,
                   time_2d, speedup, efficiency, is_correct);
        
        methods.push_back("Song song 2D");
        times.push_back(time_2d);
        speedups.push_back(speedup);
    }
    
    // 1.2.4 Nhân song song kết nối vòng
    if (config.run_ring_topology) {
        cout << "-> Dang thuc hien nhan SONG SONG - Ket noi vong..." << endl;
        double time_ring;
        multiplyRingTopology(matrix, vec, result_ring, config.matrix_rows, 
                            config.matrix_cols, config.num_threads, time_ring);
        
        double speedup = time_seq / time_ring;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareVectors(result_seq, result_ring, config.matrix_rows);
        
        printResult("NHAN SONG SONG - Ket noi vong (Ring Topology)", 
                   config.matrix_rows, config.matrix_cols, config.num_threads,
                   time_ring, speedup, efficiency, is_correct);
        
        methods.push_back("Ring Topology");
        times.push_back(time_ring);
        speedups.push_back(speedup);
    }
    
    // 1.2.5 So sánh speedup
    cout << "\n=============== BANG SO SANH SPEEDUP ===============" << endl;
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
    cout << "\n-> Dang xuat du lieu do thi..." << endl;
    exportChartData("plot_matrix_chart.py", methods, times, speedups);
    cout << "Da xuat du lieu do thi vao file: plot_matrix_chart.py" << endl;
    
    // Giải phóng bộ nhớ
    freeMatrix(matrix, config.matrix_rows);
    delete[] vec;
    delete[] result_seq;
    delete[] result_1d;
    delete[] result_2d;
    delete[] result_ring;
    
    cout << "\n+============================================================+" << endl;
    cout << "|                HOAN THANH TINH TOAN!                      |" << endl;
    cout << "+============================================================+\n" << endl;
    
    cout << "Chay lenh sau de xem do thi: python plot_matrix_chart.py\n" << endl;
    
    system("pause");
    return 0;
}
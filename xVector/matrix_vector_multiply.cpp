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
    long long vector_size;     // Kích thước vector
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
    config.vector_size = 10000000;
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
        
        if (line.find("VECTOR_SIZE=") == 0) {
            config.vector_size = stoll(line.substr(12));
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

// Tạo vector ngẫu nhiên
void createRandomVector(double* &vec, long long size) {
    vec = new double[size];
    for (long long i = 0; i < size; i++) {
        vec[i] = (double)rand() / RAND_MAX * 10.0;
    }
}

// In một phần của vector
void printVectorSample(double* vec, long long size, int sample_size = 10) {
    int print_size = min((long long)sample_size, size);
    
    cout << "Vector [" << size << "] (hien thi " << print_size << " phan tu dau):" << endl;
    cout << "  [";
    for (int i = 0; i < print_size; i++) {
        cout << fixed << setprecision(2) << vec[i];
        if (i < print_size - 1) cout << ", ";
    }
    if (print_size < size) cout << ", ...";
    cout << "]" << endl << endl;
}

// So sánh 2 kết quả (để kiểm tra tính đúng đắn)
bool compareResults(double result1, double result2, double epsilon = 1e-3) {
    return abs(result1 - result2) < epsilon;
}

// 1.2.1 Tích vô hướng tuần tự
double dotProductSequential(double* vec1, double* vec2, long long size, double& time_taken) {
    double start_time = omp_get_wtime();
    
    double result = 0.0;
    for (long long i = 0; i < size; i++) {
        result += vec1[i] * vec2[i];
    }
    
    time_taken = omp_get_wtime() - start_time;
    return result;
}

// 1.2.2 Tích vô hướng song song phân hoạch 1 chiều
double dotProduct1DPartition(double* vec1, double* vec2, long long size, 
                             int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    double result = 0.0;
    
    #pragma omp parallel num_threads(num_threads) reduction(+:result)
    {
        #pragma omp for schedule(static)
        for (long long i = 0; i < size; i++) {
            result += vec1[i] * vec2[i];
        }
    }
    
    time_taken = omp_get_wtime() - start_time;
    return result;
}

// 1.2.3 Tích vô hướng song song phân hoạch 2 chiều (chia thành các khối)
double dotProduct2DPartition(double* vec1, double* vec2, long long size, 
                             int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    double result = 0.0;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        int num_t = omp_get_num_threads();
        
        // Tính kích thước khối cho mỗi luồng
        long long block_size = (size + num_t - 1) / num_t;
        long long start_idx = tid * block_size;
        long long end_idx = min(start_idx + block_size, size);
        
        // Mỗi luồng xử lý một khối
        double local_sum = 0.0;
        for (long long i = start_idx; i < end_idx; i++) {
            local_sum += vec1[i] * vec2[i];
        }
        
        // Gộp kết quả
        #pragma omp critical
        {
            result += local_sum;
        }
    }
    
    time_taken = omp_get_wtime() - start_time;
    return result;
}

// 1.2.4 Tích vô hướng kết nối vòng (Ring topology)
double dotProductRingTopology(double* vec1, double* vec2, long long size, 
                              int num_threads, double& time_taken) {
    double start_time = omp_get_wtime();
    
    double result = 0.0;
    long long block_size = (size + num_threads - 1) / num_threads;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int tid = omp_get_thread_num();
        int num_t = omp_get_num_threads();
        
        double local_sum = 0.0;
        
        // Ring topology: mỗi luồng xử lý các khối theo vòng tròn
        for (int round = 0; round < num_t; round++) {
            int current_block = (tid + round) % num_t;
            long long start_idx = current_block * block_size;
            long long end_idx = min(start_idx + block_size, size);
            
            // Tính tích cho khối hiện tại
            for (long long i = start_idx; i < end_idx; i++) {
                local_sum += vec1[i] * vec2[i];
            }
            
            #pragma omp barrier
        }
        
        // Gộp kết quả
        #pragma omp critical
        {
            result += local_sum;
        }
    }
    
    time_taken = omp_get_wtime() - start_time;
    return result;
}

// In kết quả
void printResult(const string& method, long long size, int threads, 
                double result, double time_taken, double speedup, 
                double efficiency, bool is_correct, bool is_sequential = false) {
    cout << "+----------------------------------------------------------+" << endl;
    cout << "|  " << left << setw(56) << method << "|" << endl;
    cout << "+----------------------------------------------------------+" << endl;
    cout << "| Kich thuoc vector:     " << setw(32) << size << "|" << endl;
    if (!is_sequential) {
        cout << "| So luong:              " << setw(32) << threads << "|" << endl;
    }
    cout << "| Ket qua (dot product): " << setw(32) << fixed << setprecision(6) << result << "|" << endl;
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
    cout << "|     CHUONG TRINH TICH VO HUONG VECTOR - OpenMP            |" << endl;
    cout << "|              (Vector Dot Product)                          |" << endl;
    cout << "+============================================================+\n" << endl;
    
    // Đọc cấu hình
    Config config = readConfig("matrix_config.txt");
    
    cout << "================ CAU HINH =================" << endl;
    cout << "Kich thuoc vector: " << config.vector_size << endl;
    cout << "So luong: " << config.num_threads << endl;
    cout << "Phuong phap chay:" << endl;
    if (config.run_sequential) cout << "  - Tuan tu" << endl;
    if (config.run_1d_partition) cout << "  - Phan hoach 1 chieu" << endl;
    if (config.run_2d_partition) cout << "  - Phan hoach 2 chieu" << endl;
    if (config.run_ring_topology) cout << "  - Ket noi vong" << endl;
    cout << "==========================================\n" << endl;
    
    // 1.1.1 Tạo 2 vector ngẫu nhiên
    cout << "-> Dang tao 2 vector ngau nhien..." << endl;
    double* vec1;
    double* vec2;
    
    createRandomVector(vec1, config.vector_size);
    createRandomVector(vec2, config.vector_size);
    
    cout << "Hoan tat!\n" << endl;
    
    // Hiển thị mẫu dữ liệu
    cout << "Vector 1:" << endl;
    printVectorSample(vec1, config.vector_size, 10);
    
    cout << "Vector 2:" << endl;
    printVectorSample(vec2, config.vector_size, 10);
    
    cout << "\n=============== BAT DAU TINH TOAN ===============\n" << endl;
    
    vector<string> methods;
    vector<double> times;
    vector<double> speedups;
    double time_seq = 0;
    double result_seq = 0;
    
    // 1.2.1 Tích vô hướng tuần tự
    if (config.run_sequential) {
        cout << "-> Dang thuc hien tinh toan TUAN TU..." << endl;
        result_seq = dotProductSequential(vec1, vec2, config.vector_size, time_seq);
        printResult("TICH VO HUONG TUAN TU", config.vector_size, 
                   0, result_seq, time_seq, 0, 0, true, true);
        
        methods.push_back("Tuan tu");
        times.push_back(time_seq);
        speedups.push_back(1.0);
    }
    
    // 1.2.2 Tích vô hướng song song phân hoạch 1 chiều
    if (config.run_1d_partition) {
        cout << "-> Dang thuc hien tinh toan SONG SONG - Phan hoach 1 chieu..." << endl;
        double time_1d;
        double result_1d = dotProduct1DPartition(vec1, vec2, config.vector_size, 
                                                 config.num_threads, time_1d);
        
        double speedup = time_seq / time_1d;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareResults(result_seq, result_1d);
        
        printResult("TICH VO HUONG SONG SONG - Phan hoach 1 chieu", 
                   config.vector_size, config.num_threads,
                   result_1d, time_1d, speedup, efficiency, is_correct);
        
        methods.push_back("Song song 1D");
        times.push_back(time_1d);
        speedups.push_back(speedup);
    }
    
    // 1.2.3 Tích vô hướng song song phân hoạch 2 chiều
    if (config.run_2d_partition) {
        cout << "-> Dang thuc hien tinh toan SONG SONG - Phan hoach 2 chieu..." << endl;
        double time_2d;
        double result_2d = dotProduct2DPartition(vec1, vec2, config.vector_size, 
                                                 config.num_threads, time_2d);
        
        double speedup = time_seq / time_2d;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareResults(result_seq, result_2d);
        
        printResult("TICH VO HUONG SONG SONG - Phan hoach 2 chieu", 
                   config.vector_size, config.num_threads,
                   result_2d, time_2d, speedup, efficiency, is_correct);
        
        methods.push_back("Song song 2D");
        times.push_back(time_2d);
        speedups.push_back(speedup);
    }
    
    // 1.2.4 Tích vô hướng kết nối vòng
    if (config.run_ring_topology) {
        cout << "-> Dang thuc hien tinh toan SONG SONG - Ket noi vong..." << endl;
        double time_ring;
        double result_ring = dotProductRingTopology(vec1, vec2, config.vector_size, 
                                                    config.num_threads, time_ring);
        
        double speedup = time_seq / time_ring;
        double efficiency = speedup / config.num_threads;
        bool is_correct = compareResults(result_seq, result_ring);
        
        printResult("TICH VO HUONG SONG SONG - Ket noi vong (Ring Topology)", 
                   config.vector_size, config.num_threads,
                   result_ring, time_ring, speedup, efficiency, is_correct);
        
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
    delete[] vec1;
    delete[] vec2;
    
    cout << "\n+============================================================+" << endl;
    cout << "|                HOAN THANH TINH TOAN!                      |" << endl;
    cout << "+============================================================+\n" << endl;
    
    cout << "Chay lenh sau de xem do thi: python plot_matrix_chart.py\n" << endl;
    
    system("pause");
    return 0;
}
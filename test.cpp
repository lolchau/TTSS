#include <iostream>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>

using namespace std;

// Cấu trúc lưu cấu hình
struct Config {
    int scenario;           // 1: Dữ liệu cố định, thay đổi luồng; 2: Dữ liệu thay đổi, luồng cố định
    long long fixedSamples; // Số mẫu cố định (kịch bản 1)
    int fixedThreads;       // Số luồng cố định (kịch bản 2)
    vector<int> threadList; // Danh sách số luồng (kịch bản 1)
    vector<long long> sampleList; // Danh sách số mẫu (kịch bản 2)
    int numRuns;           // Số lần chạy để lấy trung bình
};

// Đọc file config
Config readConfig(const string& filename) {
    Config config;
    config.scenario = 1;
    config.fixedSamples = 10000000;
    config.fixedThreads = 4;
    config.numRuns = 1;
    
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Khong mo duoc file config.txt, dung gia tri mac dinh" << endl;
        config.threadList = {1, 2, 4};
        config.sampleList = {1000000, 5000000, 10000000};
        return config;
    }
    
    string line, key, value;
    
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == string::npos) continue;
        
        key = line.substr(0, pos);
        value = line.substr(pos + 1);
        
        // Loại bỏ khoảng trắng và ký tự xuống dòng
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        
        if (key == "SCENARIO") {
            config.scenario = atoi(value.c_str());
        } else if (key == "FIXED_SAMPLES") {
            config.fixedSamples = atoll(value.c_str());
        } else if (key == "FIXED_THREADS") {
            config.fixedThreads = atoi(value.c_str());
        } else if (key == "THREAD_LIST") {
            istringstream vss(value);
            string num;
            while (getline(vss, num, ',')) {
                // Loại bỏ khoảng trắng
                num.erase(0, num.find_first_not_of(" \t\r\n"));
                num.erase(num.find_last_not_of(" \t\r\n") + 1);
                if (!num.empty()) {
                    config.threadList.push_back(atoi(num.c_str()));
                }
            }
        } else if (key == "SAMPLE_LIST") {
            istringstream vss(value);
            string num;
            while (getline(vss, num, ',')) {
                // Loại bỏ khoảng trắng
                num.erase(0, num.find_first_not_of(" \t\r\n"));
                num.erase(num.find_last_not_of(" \t\r\n") + 1);
                if (!num.empty()) {
                    config.sampleList.push_back(atoll(num.c_str()));
                }
            }
        } else if (key == "NUM_RUNS") {
            config.numRuns = atoi(value.c_str());
        }
    }
    
    file.close();
    return config;
}

// Tính PI tuần tự - Windows compatible
double calculatePiSequential(long long numSamples) {
    long long insideCircle = 0;
    srand((unsigned int)time(NULL));
    
    for (long long i = 0; i < numSamples; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        if (x * x + y * y <= 1.0) {
            insideCircle++;
        }
    }
    
    return 4.0 * insideCircle / numSamples;
}

// Tính PI song song - Windows compatible với LCG
double calculatePiParallel(long long numSamples, int numThreads) {
    long long insideCircle = 0;
    
    #pragma omp parallel num_threads(numThreads)
    {
        // Mỗi thread có seed riêng
        unsigned int seed = (unsigned int)(time(NULL) * (omp_get_thread_num() + 1) + omp_get_thread_num() * 1000);
        long long localInside = 0;
        
        #pragma omp for schedule(static)
        for (long long i = 0; i < numSamples; i++) {
            // Linear Congruential Generator (LCG)
            seed = seed * 1103515245 + 12345;
            double x = (double)((seed / 65536) % 32768) / 32768.0;
            
            seed = seed * 1103515245 + 12345;
            double y = (double)((seed / 65536) % 32768) / 32768.0;
            
            if (x * x + y * y <= 1.0) {
                localInside++;
            }
        }
        
        #pragma omp atomic
        insideCircle += localInside;
    }
    
    return 4.0 * insideCircle / numSamples;
}

int main() {
    Config config = readConfig("config.txt");
    
    cout << "========================================" << endl;
    cout << "  TINH TOAN SONG SONG SO PI" << endl;
    cout << "  Thuat toan Monte Carlo - OpenMP" << endl;
    cout << "========================================" << endl << endl;
    
    ofstream resultFile("results.csv");
    resultFile << "Test,Samples,Threads,PI_Value,Sequential_Time(s),Parallel_Time(s),Speedup,Efficiency(%)" << endl;
    
    if (config.scenario == 1) {
        // Kịch bản 1: Dữ liệu cố định, thay đổi số luồng
        cout << "KICH BAN 1: Du lieu co dinh, thay doi so luong" << endl;
        cout << "So mau: " << config.fixedSamples << endl;
        cout << "Danh sach luong: ";
        for (int t : config.threadList) cout << t << " ";
        cout << endl << endl;
        
        // Tính tuần tự
        cout << "Dang tinh tuan tu...";
        double seqTime = 0;
        double seqPi = 0;
        for (int run = 0; run < config.numRuns; run++) {
            cout << "." << flush;
            double start = omp_get_wtime();
            double pi = calculatePiSequential(config.fixedSamples);
            double end = omp_get_wtime();
            seqTime += (end - start);
            seqPi += pi;
        }
        seqTime /= config.numRuns;
        seqPi /= config.numRuns;
        
        cout << " XONG!" << endl;
        cout << "Ket qua tuan tu: PI = " << fixed << setprecision(10) << seqPi 
             << ", Thoi gian = " << seqTime << "s" << endl << endl;
        
        // Tính song song với các số luồng khác nhau
        for (int threads : config.threadList) {
            cout << "Dang tinh voi " << threads << " luong...";
            double parTime = 0;
            double parPi = 0;
            
            for (int run = 0; run < config.numRuns; run++) {
                cout << "." << flush;
                double start = omp_get_wtime();
                double pi = calculatePiParallel(config.fixedSamples, threads);
                double end = omp_get_wtime();
                parTime += (end - start);
                parPi += pi;
            }
            parTime /= config.numRuns;
            parPi /= config.numRuns;
            
            double speedup = seqTime / parTime;
            double efficiency = (speedup / threads) * 100;
            
            cout << " XONG!" << endl;
            cout << "  PI = " << parPi << ", Thoi gian = " << parTime 
                 << "s, Speedup = " << speedup << "x, Hieu suat = " << efficiency << "%" << endl << endl;
            
            resultFile << "Threads_" << threads << "," << config.fixedSamples << "," 
                      << threads << "," << parPi << "," << seqTime << "," 
                      << parTime << "," << speedup << "," << efficiency << endl;
        }
        
    } else if (config.scenario == 2) {
        // Kịch bản 2: Dữ liệu thay đổi, luồng cố định
        cout << "KICH BAN 2: Du lieu thay doi, luong co dinh" << endl;
        cout << "So luong: " << config.fixedThreads << endl;
        cout << "Danh sach so mau: ";
        for (long long s : config.sampleList) cout << s << " ";
        cout << endl << endl;
        
        for (long long samples : config.sampleList) {
            cout << "=== So mau: " << samples << " ===" << endl;
            
            // Tính tuần tự
            cout << "  Tuan tu...";
            double seqTime = 0;
            double seqPi = 0;
            for (int run = 0; run < config.numRuns; run++) {
                cout << "." << flush;
                double start = omp_get_wtime();
                double pi = calculatePiSequential(samples);
                double end = omp_get_wtime();
                seqTime += (end - start);
                seqPi += pi;
            }
            seqTime /= config.numRuns;
            seqPi /= config.numRuns;
            cout << " XONG!" << endl;
            
            // Tính song song
            cout << "  Song song...";
            double parTime = 0;
            double parPi = 0;
            for (int run = 0; run < config.numRuns; run++) {
                cout << "." << flush;
                double start = omp_get_wtime();
                double pi = calculatePiParallel(samples, config.fixedThreads);
                double end = omp_get_wtime();
                parTime += (end - start);
                parPi += pi;
            }
            parTime /= config.numRuns;
            parPi /= config.numRuns;
            cout << " XONG!" << endl;
            
            double speedup = seqTime / parTime;
            double efficiency = (speedup / config.fixedThreads) * 100;
            
            cout << "  Tuan tu: PI = " << fixed << setprecision(10) << seqPi 
                 << ", Thoi gian = " << seqTime << "s" << endl;
            cout << "  Song song: PI = " << parPi << ", Thoi gian = " << parTime 
                 << "s, Speedup = " << speedup << "x, Hieu suat = " << efficiency << "%" << endl << endl;
            
            resultFile << "Samples_" << samples << "," << samples << "," 
                      << config.fixedThreads << "," << parPi << "," << seqTime << "," 
                      << parTime << "," << speedup << "," << efficiency << endl;
        }
    }
    
    resultFile.close();
    cout << "\n========================================" << endl;
    cout << "Ket qua da duoc luu vao file results.csv" << endl;
    cout << "Chay script Python de tao do thi: python plot_results.py" << endl;
    cout << "========================================" << endl;
    
    return 0;
}
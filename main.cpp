#include <iostream>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <string>
#include <filesystem>

using namespace std;

/* ================== HẰNG SỐ ================== */
const double PI_REF = 3.14159265358979323846;

/* ================== RNG NHANH (LCG) ================== */
inline double fast_rand(unsigned int& seed) {
    seed = seed * 1664525u + 1013904223u;
    return (double)(seed & 0xFFFFFF) / (double)0x1000000; // [0,1)
}

/* ================== STRUCT CONFIG ================== */
struct Config {
    int scenario;
    long long fixed_samples;
    int fixed_threads;
    vector<int> thread_list;
    vector<long long> sample_list;
    int num_runs; // <=== thêm
};

/* ================== READ CONFIG ================== */
Config readConfig(const string& filename) {
    // set default trước để tránh thiếu key bị rác
    Config config;
    config.scenario = 1;
    config.fixed_samples = 10000000;
    config.fixed_threads = 4;
    config.thread_list = {1, 2, 4};
    config.sample_list = {5000000, 10000000};
    config.num_runs = 1;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Khong mo duoc config.txt. Dung gia tri mac dinh.\n";
        return config;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.find("SCENARIO=") == 0)
            config.scenario = stoi(line.substr(9));
        else if (line.find("FIXED_SAMPLES=") == 0)
            config.fixed_samples = stoll(line.substr(14));
        else if (line.find("FIXED_THREADS=") == 0)
            config.fixed_threads = stoi(line.substr(14));
        else if (line.find("NUM_RUNS=") == 0)
            config.num_runs = max(1, stoi(line.substr(9)));
        else if (line.find("THREAD_LIST=") == 0) {
            config.thread_list.clear();
            string s = line.substr(12);
            size_t pos;
            while ((pos = s.find(',')) != string::npos) {
                config.thread_list.push_back(stoi(s.substr(0, pos)));
                s.erase(0, pos + 1);
            }
            if (!s.empty()) config.thread_list.push_back(stoi(s));
        }
        else if (line.find("SAMPLE_LIST=") == 0) {
            config.sample_list.clear();
            string s = line.substr(12);
            size_t pos;
            while ((pos = s.find(',')) != string::npos) {
                config.sample_list.push_back(stoll(s.substr(0, pos)));
                s.erase(0, pos + 1);
            }
            if (!s.empty()) config.sample_list.push_back(stoll(s));
        }
    }

    file.close();
    return config;
}

/* ================== TUẦN TỰ ================== */
double calculatePiSequential(long long samples, double& time_taken, unsigned int run_id = 0) {
    // trộn seed theo run_id để mỗi lần chạy khác nhau
    unsigned int seed = (unsigned int)time(NULL) ^ (run_id * 2654435761u);
    long long inside = 0;

    double start = omp_get_wtime();

    for (long long i = 0; i < samples; i++) {
        double x = fast_rand(seed);
        double y = fast_rand(seed);
        if (x * x + y * y <= 1.0)
            inside++;
    }

    time_taken = omp_get_wtime() - start;
    return 4.0 * inside / samples;
}

/* ================== SONG SONG ================== */
double calculatePiParallel(long long samples, int threads, double& time_taken, unsigned int run_id = 0) {
    vector<long long> local_count(threads, 0);

    double start = omp_get_wtime();

    #pragma omp parallel num_threads(threads)
    {
        int tid = omp_get_thread_num();
        unsigned int seed = (unsigned int)time(NULL)
                            ^ (tid * 123456789u)
                            ^ (run_id * 362437u);

        #pragma omp for schedule(static)
        for (long long i = 0; i < samples; i++) {
            double x = fast_rand(seed);
            double y = fast_rand(seed);
            if (x * x + y * y <= 1.0)
                local_count[tid]++;
        }
    }

    long long inside = 0;
    for (int i = 0; i < threads; i++)
        inside += local_count[i];

    time_taken = omp_get_wtime() - start;
    return 4.0 * inside / samples;
}

/* ================== IN KẾT QUẢ ================== */
void printResult(long long n, int threads, double pi, double time_taken,
                 double speedup, double efficiency, bool sequential) {

    cout << fixed << setprecision(8);
    cout << "------------------------------------------\n";
    if (sequential)
        cout << "TUAN TU (TB)\n";
    else
        cout << "SONG SONG (" << threads << " luong) (TB)\n";

    cout << "So mau      : " << n << "\n";
    cout << "Gia tri PI  : " << pi << "\n";
    cout << "Sai so      : " << fabs(pi - PI_REF) << "\n";
    cout << "Thoi gian   : " << time_taken << " s\n";

    if (!sequential) {
        cout << "Speedup     : " << speedup << "\n";
        cout << "Hieu suat   : " << efficiency * 100 << " %\n";
    }
    cout << "------------------------------------------\n\n";
}

/* ================== MAIN ================== */
int main() {
    omp_set_dynamic(0);

    Config config = readConfig("config.txt");
    filesystem::path outPath = filesystem::current_path() / "results.csv";
    cout << " >>> Ghi ket qua vao :" << outPath.string() << "\n";

    bool writeHeader = !filesystem::exists(outPath);
    ofstream resultFile(outPath, ios::app);
    ofstream csv(outPath, ios::app | ios::app);
    if (!csv.is_open()) {
        cerr << "Khong mo duoc file results.csv de ghi ket qua.\n";
        return 1;
    }
    if (writeHeader) {
        csv << "Test,Samples,Threads,PI_Value,Sequential_Time(s),Parallel_Time(s),Speedup,Efficiency(%)\n";
        csv.flush();
    }
    cout << "NUM_RUNS = " << config.num_runs << "\n\n";

    if (config.scenario == 1) {
        cout << "KICH BAN 1: CO DINH DU LIEU - DOI SO LUONG\n\n";

        // === TB tuần tự ===
        double sum_t_seq = 0.0, sum_pi_seq = 0.0;
        for (int r = 0; r < config.num_runs; r++) {
            double t, pi;
            pi = calculatePiSequential(config.fixed_samples, t, (unsigned)r + 1);
            sum_t_seq += t;
            sum_pi_seq += pi;
        }
        double t_seq_avg = sum_t_seq / config.num_runs;
        double pi_seq_avg = sum_pi_seq / config.num_runs;

        printResult(config.fixed_samples, 0, pi_seq_avg, t_seq_avg, 0, 0, true);

        // === TB song song theo từng threads ===
        for (int threads : config.thread_list) {
            cout << ">>> BAT DAU THREAD = " << threads << endl;
            if (threads <= 1) {
                cout << ">>> BO QUA (threads <= 1)\n\n";
                continue;
            }

            double sum_t_par = 0.0, sum_pi_par = 0.0;
            for (int r = 0; r < config.num_runs; r++) {
                double t, pi;
                pi = calculatePiParallel(config.fixed_samples, threads, t, (unsigned)r + 1);
                sum_t_par += t;
                sum_pi_par += pi;
            }
            double t_par_avg = sum_t_par / config.num_runs;
            double pi_par_avg = sum_pi_par / config.num_runs;

            double speedup = t_seq_avg / t_par_avg;
            double efficiency = speedup / threads;

            printResult(config.fixed_samples, threads, pi_par_avg, t_par_avg, speedup, efficiency, false);
            csv << "Threads_" << threads << "," << config.fixed_samples << "," << threads << ","
                << pi_par_avg << "," << t_seq_avg << "," << t_par_avg << ","
                << speedup << "," << efficiency * 100 << "\n";
            cout << ">>> KET THUC THREAD = " << threads << "\n\n";
        }
    }
    else {
        cout << "KICH BAN 2: CO DINH LUONG - DOI DU LIEU\n\n";

        for (long long samples : config.sample_list) {

            // === TB tuần tự ===
            double sum_t_seq = 0.0, sum_pi_seq = 0.0;
            for (int r = 0; r < config.num_runs; r++) {
                double t, pi;
                pi = calculatePiSequential(samples, t, (unsigned)r + 1);
                sum_t_seq += t;
                sum_pi_seq += pi;
            }
            double t_seq_avg = sum_t_seq / config.num_runs;
            double pi_seq_avg = sum_pi_seq / config.num_runs;

            printResult(samples, 0, pi_seq_avg, t_seq_avg, 0, 0, true);

            // === TB song song ===
            double sum_t_par = 0.0, sum_pi_par = 0.0;
            for (int r = 0; r < config.num_runs; r++) {
                double t, pi;
                pi = calculatePiParallel(samples, config.fixed_threads, t, (unsigned)r + 1);
                sum_t_par += t;
                sum_pi_par += pi;
            }
            double t_par_avg = sum_t_par / config.num_runs;
            double pi_par_avg = sum_pi_par / config.num_runs;

            double speedup = t_seq_avg / t_par_avg;
            double efficiency = speedup / config.fixed_threads;

            printResult(samples, config.fixed_threads, pi_par_avg, t_par_avg, speedup, efficiency, false);
            csv << "Threads_" << samples << "," << samples << "," << config.fixed_threads << ","
                << pi_par_avg << "," << t_seq_avg << "," << t_par_avg << ","
                << speedup << "," << efficiency * 100 << "\n";
        }
    }
    csv.close();
    cout << "\n Hoan Thanh";
    return 0;
}

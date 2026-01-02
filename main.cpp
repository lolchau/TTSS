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

/* ================== HÀM SINH SỐ NGẪU NHIÊN NHANH ================== */
inline double fast_rand(unsigned int& seed) {
    seed = seed * 1664525u + 1013904223u;
    return (double)(seed & 0xFFFFFF) / (double)0x1000000;
}

/* ================== STRUCT CONFIG ================== */
struct Config {
    int scenario;
    long long fixed_samples;
    int fixed_threads;
    vector<int> thread_list;
    vector<long long> sample_list;
};

/* ================== READ CONFIG ================== */
Config readConfig(const string& filename) {
    Config config;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Khong mo duoc config.txt. Dung gia tri mac dinh.\n";
        config.scenario = 1;
        config.fixed_samples = 10000000;
        config.fixed_threads = 4;
        config.thread_list = {1, 2, 4};
        config.sample_list = {5000000, 10000000};
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
        else if (line.find("THREAD_LIST=") == 0) {
            string s = line.substr(12);
            size_t pos;
            while ((pos = s.find(',')) != string::npos) {
                config.thread_list.push_back(stoi(s.substr(0, pos)));
                s.erase(0, pos + 1);
            }
            config.thread_list.push_back(stoi(s));
        }
        else if (line.find("SAMPLE_LIST=") == 0) {
            string s = line.substr(12);
            size_t pos;
            while ((pos = s.find(',')) != string::npos) {
                config.sample_list.push_back(stoll(s.substr(0, pos)));
                s.erase(0, pos + 1);
            }
            config.sample_list.push_back(stoll(s));
        }
    }

    file.close();
    return config;
}

/* ================== TUẦN TỰ ================== */
double calculatePiSequential(long long samples, double& time_taken) {
    unsigned int seed = (unsigned int)time(NULL);
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
double calculatePiParallel(long long samples, int threads, double& time_taken) {
    vector<long long> local_count(threads, 0);

    double start = omp_get_wtime();

    #pragma omp parallel num_threads(threads)
    {
        int tid = omp_get_thread_num();
        unsigned int seed =
            (unsigned int)time(NULL) ^ (tid * 123456789u);

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
        cout << "TUAN TU\n";
    else
        cout << "SONG SONG (" << threads << " luong)\n";

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
cout << ">>> Ghi ket qua vao: " << outPath.string() << "\n";

bool writeHeader = !filesystem::exists(outPath);

ofstream csv(outPath, ios::out | ios::app);
if (!csv.is_open()) {
    cerr << " Khong mo duoc results.csv (co the file dang bi khoa boi Excel/Editor).\n";
    cerr << " Hay dong results.csv (Excel/Preview) va chay lai.\n";
    return 1;
}

if (writeHeader) {
    csv << "Test,Samples,Threads,PI_Value,Sequential_Time(s),Parallel_Time(s),Speedup,Efficiency(%)\n";
    csv.flush();
}

    if (config.scenario == 1) {
        cout << "KICH BAN 1: CO DINH DU LIEU - DOI SO LUONG\n\n";

        double t_seq, pi_seq;
        pi_seq = calculatePiSequential(config.fixed_samples, t_seq);
        printResult(config.fixed_samples, 0, pi_seq, t_seq, 0, 0, true);

        for (int threads : config.thread_list) {
            cout << ">>> BAT DAU THREAD = " << threads << endl;
            for (int t : config.thread_list) cout << t << " ";
            cout << endl;

            if (threads <= 1) continue;

            double t_par, pi_par;
            pi_par = calculatePiParallel(config.fixed_samples, threads, t_par);

            double speedup = t_seq / t_par;
            double efficiency = speedup / threads;

            printResult(config.fixed_samples, threads,
                        pi_par, t_par, speedup, efficiency, false);
            cout << ">>> KET THUC THREAD = " << threads << endl;
            cout << "Threads " << threads
                 << " | Speedup = " << speedup
                 << " | Efficiency = " << efficiency << "\n";
/* ================== file results ================== */
            csv << "Threads_" << threads << ","
                << config.fixed_samples << ","
                << threads << ","
                << pi_par << ","
                << t_seq << ","
                << t_par << ","
                << speedup << ","
                << efficiency << "\n";
        }
    }
    else {
        cout << "KICH BAN 2: CO DINH LUONG - DOI DU LIEU\n\n";

        for (long long samples : config.sample_list) {
            double t_seq, pi_seq;
            pi_seq = calculatePiSequential(samples, t_seq);
            printResult(samples, 0, pi_seq, t_seq, 0, 0, true);

            double t_par, pi_par;
            pi_par = calculatePiParallel(samples,
                                         config.fixed_threads, t_par);

            double speedup = t_seq / t_par;
            double efficiency = speedup / config.fixed_threads;

            printResult(samples, config.fixed_threads,
                        pi_par, t_par, speedup, efficiency, false);

            csv << "Samples_" << samples << ","
                << samples << ","
                << config.fixed_threads << ","
                << pi_par << ","
                << t_seq << ","
                << t_par << ","
                << speedup << ","
                << efficiency << "\n";
        }
    }
    csv.close();
    cout << "\nHOAN THANH!\n";
    return 0;
}

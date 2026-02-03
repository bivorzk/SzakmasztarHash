#include <iostream>
#include <cstdio>
#include <string>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
#include <functional>
#include <fstream>

void check_range(int start, int end, const std::string& base_url, const std::string& extension, std::mutex& mtx, std::ofstream& outfile) {
    for (int i = start; i < end; ++i) {
        std::stringstream ss;
        ss << std::hex << std::setw(4) << std::setfill('0') << i;
        std::string hash = ss.str();

        std::string url = base_url + hash + extension;
        std::string command = "curl -s -o NUL -w \"%{http_code}\" -m 5 \"" + url + "\"";

        FILE* pipe = _popen(command.c_str(), "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                int status = atoi(buffer);
                std::lock_guard<std::mutex> lock(mtx);
                if (status == 200) {
                    std::cout << "Found valid hash: " << hash << std::endl;
                    outfile << "Found valid hash: " << hash << std::endl;
                } else {
                    std::cout << "Unavailable hash: " << hash << " - Status: " << status << std::endl;
                    outfile << "Unavailable hash: " << hash << " - Status: " << status << std::endl;
                }
            }
            _pclose(pipe);// 
        }
    }
}

int main() {
    std::string base_url = "https://s3-eu-west-1.amazonaws.com/szakmavilag/Informatika%20%C3%A9s%20T%C3%A1vk%C3%B6zl%C3%A9s%20%C3%A1gazat-";
    std::string extension = ".xlsx";

    const int total_hashes = 65536;
    const int num_threads = 32;
    int range_size = total_hashes / num_threads;

    std::vector<std::thread> threads;
    std::mutex mtx;
    std::ofstream outfile("results.txt");

    for (int t = 0; t < num_threads; ++t) {
        int start = t * range_size;
        int end = (t == num_threads - 1) ? total_hashes : start + range_size;
        threads.emplace_back(check_range, start, end, std::ref(base_url), std::ref(extension), std::ref(mtx), std::ref(outfile));
    }

    for (auto& th : threads) {
        th.join();
    }

    outfile.close();
    std::cout << "Brute force completed." << std::endl;
    return 0;
}
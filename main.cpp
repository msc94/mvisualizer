#include "capture.h"
#include "fft.h"
#include "window.h"

#include <chrono>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <thread>

std::vector<float> bin(const std::vector<float> &data, int num_bins) {
    const std::size_t n = data.size();
    const std::size_t bin_size = n / num_bins;
    std::vector<float> bins(num_bins);

    for (std::size_t i = 0; i < data.size(); i++) {
        std::size_t bin = i / bin_size;
        bins[bin] += data[i] / bin_size;
    }

    return bins;
}

int main() {
    spdlog::default_logger()->set_level(spdlog::level::debug);

    Capture capture("Family 17h/19h HD Audio Controller Analog Stereo");
    capture.start_capture();

    int sample_rate = capture.sample_rate();
    spdlog::debug("Sample rate {}", sample_rate);

    Window window;
    while (true) {
        auto start = std::chrono::steady_clock::now();
        std::vector<float> data = capture.data(0);
        std::vector<float> fft_output = fft_analyze(data);
        std::vector<float> bins = bin(fft_output, 100);

        bool go = window.render(bins);
        if (!go) {
            break;
        }

        auto sleep_until = start + std::chrono::milliseconds{16};
        std::this_thread::sleep_until(sleep_until);
    }
}

#include "capture.h"
#include "fft.h"
#include "window.h"

#include <spdlog/spdlog.h>

std::vector<float> bin(const std::vector<float> &data, int num_bins) {
    std::vector<float> bins(num_bins);

    for (std::size_t i = 0; i < data.size(); i++) {
        std::size_t bin = i / num_bins;
        bins[bin] += data[i];
    }

    for (std::size_t i = 0; i < bins.size(); i++) {
        bins[i] /= (data.size() / num_bins);
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
        std::vector<float> data = capture.data(0);
        std::vector<float> fft_output = fft_analyze(data);
        std::vector<float> bins = bin(fft_output, 100);

        bool go = window.render(bins);
        if (!go) {
            break;
        }
    }
}

#include "capture.h"
#include "fft.h"
#include "window.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <thread>

namespace {

std::vector<float> apply_hann(const std::vector<float> &data) {
    std::size_t n = data.size();
    std::vector<float> result(n);
    for (size_t i = 0; i < n; ++i) {
        float t = (float)i / (n - 1);
        float hann = 0.5 - 0.5 * cosf(2 * M_PI * t);
        result[i] = data[i] * hann;
    }
    return result;
}

std::vector<float> normalize(const std::vector<float> &data) {
    constexpr bool normalize = true;
    if (!normalize) {
        return data;
    }

    float max_element = *std::max_element(std::begin(data), std::end(data));
    if (max_element == 0.0f) {
        return data;
    }

    std::size_t n = data.size();
    std::vector<float> result(n);
    for (size_t i = 0; i < n; ++i) {
        result[i] = data[i] / max_element;
    }

    return result;
}

std::vector<float> calc_fft_freqs(int fft_size, int sample_rate) {
    std::vector<float> freqs(fft_size / 2);

    for (std::size_t i = 0; i < freqs.size(); i++) {
        freqs[i] = static_cast<float>(sample_rate) / fft_size * i;
    }

    return freqs;
}

std::vector<float> bin(const std::vector<float> &fft_output, const std::vector<float> freqs, int num_bins) {
    // https://stackoverflow.com/questions/4364823/how-do-i-obtain-the-frequencies-of-each-value-in-an-fft

    // Only use first half of FFT output (it's mirrored for real-valued inputs)
    const std::size_t n = fft_output.size() / 2;
    std::vector<float> bins(num_bins);

    const float min_freq = 20.0f;
    const float max_freq = 5000.0f;
    const float base = std::pow(max_freq / min_freq, 1.0 / num_bins);

    float current_freq_cutoff = min_freq;
    std::size_t current_bin = 0;
    float current_bin_max_magnitude = 0.0f;
    std::size_t i = 0;

    while (i < n) {
        float current_freq = freqs[i];

        if (current_freq > current_freq_cutoff && current_bin < bins.size() - 1) {
            bins[current_bin] = current_bin_max_magnitude;
            current_bin_max_magnitude = 0.0f;
            current_bin++;
            current_freq_cutoff *= base;
        }

        float current_magnitude = fft_output[i];
        current_bin_max_magnitude = std::max(current_bin_max_magnitude, current_magnitude);
        i++;
    }

    bins[current_bin] = current_bin_max_magnitude;

    return bins;
}

std::string get_device() {
    constexpr bool use_microphone = false;
    if (use_microphone) {
        return " KLIM  LINGO Mono";
    } else {
        return "Family 17h/19h HD Audio Controller Analog Stereo";
    }
}

} // namespace

int main() {
    spdlog::default_logger()->set_level(spdlog::level::debug);

    const std::string device = get_device();
    Capture capture(device);
    capture.list_devices();
    capture.start_capture();

    int sample_rate = capture.sample_rate();
    int fft_size = capture.buffer_size();
    std::vector<float> fft_freqs = calc_fft_freqs(fft_size, sample_rate);
    spdlog::debug("Sample rate {}, buffer size {}", sample_rate, fft_size);

    const int num_bins = 256;
    Window window(num_bins);

    while (true) {
        auto start = std::chrono::steady_clock::now();

        std::vector<float> data = capture.data(0);
        std::vector<float> hann = apply_hann(data);
        std::vector<float> fft_output = fft_analyze(hann);
        std::vector<float> bins = bin(fft_output, fft_freqs, num_bins);
        std::vector<float> normalized = normalize(bins);

        window.update(normalized);
        bool go = window.render();
        if (!go) {
            break;
        }

        auto sleep_until = start + std::chrono::milliseconds{16};
        std::this_thread::sleep_until(sleep_until);
    }
}

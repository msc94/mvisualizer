#include "fft.h"

#include <algorithm>
#include <fmt/format.h>
#include <kissfft/kiss_fft.h>

std::vector<float> fft_analyze(const std::vector<float> &data) {
    const std::size_t N = data.size();

    std::vector<kiss_fft_cpx> input(N);
    std::vector<kiss_fft_cpx> output(N);

    for (std::size_t i = 0; i < input.size(); i++) {
        input[i].r = data[i];
        input[i].i = 0.0f;
    }

    kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, NULL, NULL);
    kiss_fft(cfg, input.data(), output.data());
    kiss_fft_free(cfg);

    std::vector<float> magnitudes(N);
    for (std::size_t i = 0; i < magnitudes.size(); i++) {
        float power = output[i].r * output[i].r + output[i].i * output[i].i;
        float magnitude = sqrt(power);
        magnitudes[i] = 20.0f * log10f(magnitude);
    }

    return magnitudes;
}

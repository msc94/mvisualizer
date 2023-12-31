#include "capture.h"

#include <cstddef>
#include <fmt/format.h>
#include <mutex>
#include <soundio/soundio.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace {

void read_callback(SoundIoInStream *instream, int, int frame_count_max) {
    RecordingContext *context = static_cast<RecordingContext *>(instream->userdata);
    std::unique_lock<std::mutex> lock{context->mutex};

    // spdlog::debug("Read callback called, received [{}, {}] frames", frame_count_min, frame_count_max);

    int err = SoundIoErrorNone;
    SoundIoChannelArea *areas;

    int read_frames = frame_count_max;

    if ((err = soundio_instream_begin_read(instream, &areas, &read_frames)) != SoundIoErrorNone) {
        throw std::runtime_error(fmt::format("Failed to begin reading from instream: {}", soundio_strerror(err)));
    }

    if (areas != nullptr) {
        for (int channel = 0; channel < instream->layout.channel_count; channel++) {
            SoundIoChannelArea &channel_area = areas[channel];
            std::deque<float> &channel_buffer = context->channels[channel].samples;

            const int buffer_size = context->buffer_size;
            const int elements_to_delete = (channel_buffer.size() - buffer_size) + read_frames;

            if (elements_to_delete > 0) {
                channel_buffer.erase(channel_buffer.begin(), channel_buffer.begin() + elements_to_delete);
            }

            for (int frame = 0; frame < read_frames; frame++) {
                float sample = *reinterpret_cast<float *>(channel_area.ptr);
                channel_buffer.push_back(sample);
                channel_area.ptr += channel_area.step;
            }
        }
    } else {
        spdlog::warn("Detected skipped frames");
    }

    if ((err = soundio_instream_end_read(instream)) != SoundIoErrorNone) {
        throw std::runtime_error(fmt::format("Failed to begin reading from instream: {}", soundio_strerror(err)));
    }
}

} // namespace

Capture::Capture(const std::string &device_id) {
    int err = SoundIoErrorNone;

    soundio_ = soundio_create();
    if (!soundio_) {
        throw std::runtime_error("Failed to create soundio context");
    }

    if ((err = soundio_connect(soundio_)) != SoundIoErrorNone) {
        throw std::runtime_error(fmt::format("Failed to connect: {}", soundio_strerror(err)));
    }

    spdlog::info("Connected to backend {}", soundio_backend_name(soundio_->current_backend));

    for (int i = 0; i < soundio_input_device_count(soundio_); i++) {
        SoundIoDevice *device = soundio_get_input_device(soundio_, i);
        const std::string &id = device->id;

        if (id == device_id) {
            device_ = device;
            spdlog::debug("Choose device: {}", device_->id);
            break;
        }

        soundio_device_unref(device);
    }

    if (!device_ || device_->probe_error) {
        throw std::runtime_error(fmt::format("Failed to find device with ID {}", device_id));
    }

    instream_ = soundio_instream_create(device_);
    if (!instream_) {
        throw std::runtime_error("Failed to create instream");
    }

    instream_->format = SoundIoFormatFloat32NE;
    instream_->sample_rate = 48000;
    instream_->read_callback = read_callback;
    instream_->userdata = &recording_context_;

    if ((err = soundio_instream_open(instream_)) != SoundIoErrorNone) {
        throw std::runtime_error(fmt::format("Failed to open instream: {}", soundio_strerror(err)));
    }

    recording_context_.buffer_size = 1024;
    recording_context_.channels.resize(instream_->layout.channel_count);
}

Capture::~Capture() {
    soundio_instream_destroy(instream_);
    soundio_device_unref(device_);
    soundio_disconnect(soundio_);
    soundio_destroy(soundio_);
}

void Capture::list_devices() {
    spdlog::info("Available devices:");
    for (int i = 0; i < soundio_input_device_count(soundio_); i++) {
        SoundIoDevice *device = soundio_get_input_device(soundio_, i);
        spdlog::info("  - {}", device->id);
        soundio_device_unref(device);
    }
}

void Capture::start_capture() {
    int err = SoundIoErrorNone;
    if ((err = soundio_instream_start(instream_)) != SoundIoErrorNone) {
        throw std::runtime_error(fmt::format("Failed to start instream: {}", soundio_strerror(err)));
    }
}

std::vector<float> Capture::data(int channel_index) {
    std::unique_lock lock{recording_context_.mutex};
    const Channel &channel = recording_context_.channels[channel_index];
    const std::deque<float> &samples = channel.samples;
    if (samples.size() < static_cast<std::size_t>(recording_context_.buffer_size)) {
        return {};
    } else {
        return std::vector<float>{samples.begin(), samples.end()};
    }
}

int Capture::sample_rate() const {
    return instream_->sample_rate;
}

int Capture::buffer_size() const {
    return recording_context_.buffer_size;
}

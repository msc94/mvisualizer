#pragma once

#include <boost/noncopyable.hpp>
#include <soundio/soundio.h>

#include <deque>
#include <mutex>
#include <string>
#include <vector>

struct Channel {
    std::deque<float> samples{};
};

struct RecordingContext {
    std::vector<Channel> channels{};
    int buffer_size{0};
    std::mutex mutex{};
};

class Capture : private boost::noncopyable {
  public:
    Capture(const std::string &device_id);
    ~Capture();

    void list_devices();
    void start_capture();
    void flush();
    std::vector<float> data(int channel_index);
    int sample_rate() const;
    int buffer_size() const;

  private:
    SoundIo *soundio_{nullptr};
    SoundIoDevice *device_{nullptr};
    SoundIoInStream *instream_{nullptr};
    RecordingContext recording_context_{};
};

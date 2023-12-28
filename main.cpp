#include "capture.h"
#include <spdlog/spdlog.h>

int main() {
  spdlog::default_logger()->set_level(spdlog::level::level_enum::debug);

  Capture capture("Family 17h/19h HD Audio Controller Analog Stereo");
  capture.start_capture();

  while (true)
    ;
}

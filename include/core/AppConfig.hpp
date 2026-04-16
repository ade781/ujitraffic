#pragma once

#include <string>

namespace traffic::core {

struct AppConfig {
    unsigned int windowWidth {1280};
    unsigned int windowHeight {720};
    std::string windowTitle {"Traffic Microsimulation - Stage 1"};
    unsigned int frameLimit {60};
};

}  // namespace traffic::core


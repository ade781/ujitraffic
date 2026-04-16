#pragma once

#include <algorithm>

namespace traffic::core {

class FixedTimestep {
public:
    explicit FixedTimestep(float stepSeconds = 1.0F / 60.0F)
        : stepSeconds_(stepSeconds) {}

    float step_seconds() const noexcept {
        return stepSeconds_;
    }

    void set_step_seconds(float stepSeconds) noexcept {
        stepSeconds_ = stepSeconds > 0.0F ? stepSeconds : stepSeconds_;
    }

    void reset() noexcept {
        accumulatorSeconds_ = 0.0F;
    }

    void add_frame_time(float frameSeconds) noexcept {
        accumulatorSeconds_ = std::max(0.0F, accumulatorSeconds_ + frameSeconds);
    }

    bool has_step() const noexcept {
        return accumulatorSeconds_ >= stepSeconds_;
    }

    bool consume_step() noexcept {
        if (!has_step()) {
            return false;
        }

        accumulatorSeconds_ -= stepSeconds_;
        return true;
    }

    float alpha() const noexcept {
        if (stepSeconds_ <= 0.0F) {
            return 0.0F;
        }

        return std::clamp(accumulatorSeconds_ / stepSeconds_, 0.0F, 1.0F);
    }

    float accumulator_seconds() const noexcept {
        return accumulatorSeconds_;
    }

private:
    float stepSeconds_ {1.0F / 60.0F};
    float accumulatorSeconds_ {0.0F};
};

}  // namespace traffic::core


#include <unity.h>

#include "../../control/PressureController.h"
#include "../../control/SafetyManager.h"

void test_controller_enables_feed_when_pressure_is_low() {
    PressureControlInput input;
    input.currentPressureKpa = 0;
    input.targetPressureKpa = 3;
    input.toleranceKpa = 1;
    input.depressurizing = false;

    const PressureControlResult result = PressureController::compute(input);

    TEST_ASSERT_TRUE(result.actuators.feedOn);
    TEST_ASSERT_FALSE(result.actuators.suctionOn);
    TEST_ASSERT_TRUE(result.actuators.motorOn);
    TEST_ASSERT_FALSE(result.targetReached);
}

void test_controller_enables_suction_when_pressure_is_high() {
    PressureControlInput input;
    input.currentPressureKpa = 5;
    input.targetPressureKpa = -1;
    input.toleranceKpa = 1;
    input.depressurizing = true;

    const PressureControlResult result = PressureController::compute(input);

    TEST_ASSERT_FALSE(result.actuators.feedOn);
    TEST_ASSERT_TRUE(result.actuators.suctionOn);
    TEST_ASSERT_TRUE(result.actuators.motorOn);
    TEST_ASSERT_FALSE(result.targetReached);
}

void test_controller_marks_target_reached() {
    PressureControlInput input;
    input.currentPressureKpa = 4;
    input.targetPressureKpa = 5;
    input.toleranceKpa = 1;
    input.depressurizing = false;

    const PressureControlResult result = PressureController::compute(input);

    TEST_ASSERT_TRUE(result.targetReached);
    TEST_ASSERT_FALSE(result.actuators.motorOn);
}

void test_safety_detects_timeout() {
    ModeConfig mode = {};
    mode.minRangeKpa = 1;
    mode.maxRangeKpa = 100;
    mode.alarmMarginKpa = 5;
    mode.maxPhaseTimeMs = 1000;

    SafetyInput input;
    input.sample.pressureKpa = 0;
    input.sample.valid = true;
    input.mode = mode;
    input.state = SystemState::Pressurizing;
    input.nowMs = 1501;
    input.phaseStartedAtMs = 0;

    const FaultCode fault = SafetyManager::evaluate(input);

    TEST_ASSERT_EQUAL(static_cast<int>(FaultCode::RegulationTimeout),
                      static_cast<int>(fault));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_controller_enables_feed_when_pressure_is_low);
    RUN_TEST(test_controller_enables_suction_when_pressure_is_high);
    RUN_TEST(test_controller_marks_target_reached);
    RUN_TEST(test_safety_detects_timeout);
    return UNITY_END();
}

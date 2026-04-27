#include <unity.h>

#include "../../control/PressureController.h"
#include "../../control/SafetyManager.h"

void test_controller_enables_feed_when_pressure_is_low() {
    const PressureControlResult result = PressureController::compute({
        0,
        3,
        1,
        0,
        0,
        5000,
        false
    });

    TEST_ASSERT_TRUE(result.actuators.feedOn);
    TEST_ASSERT_FALSE(result.actuators.suctionOn);
    TEST_ASSERT_FALSE(result.pressureStable);
}

void test_controller_enables_suction_when_pressure_is_high() {
    const PressureControlResult result = PressureController::compute({
        5,
        1,
        1,
        0,
        0,
        5000,
        false
    });

    TEST_ASSERT_FALSE(result.actuators.feedOn);
    TEST_ASSERT_TRUE(result.actuators.suctionOn);
    TEST_ASSERT_FALSE(result.pressureStable);
}

void test_controller_finishes_hold_after_duration() {
    const PressureControlResult result = PressureController::compute({
        3,
        3,
        1,
        6000,
        1000,
        5000,
        true
    });

    TEST_ASSERT_TRUE(result.pressureStable);
    TEST_ASSERT_TRUE(result.holdComplete);
}

void test_safety_detects_timeout() {
    ModeConfig mode = {};
    mode.minTargetKpa = -5;
    mode.maxTargetKpa = 5;
    mode.alarmMarginKpa = 2;
    mode.maxRegulationTimeMs = 1000;

    const FaultCode fault = SafetyManager::evaluate({
        {0, true},
        mode,
        SystemState::Running,
        1501,
        0
    });

    TEST_ASSERT_EQUAL(static_cast<int>(FaultCode::RegulationTimeout),
                      static_cast<int>(fault));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_controller_enables_feed_when_pressure_is_low);
    RUN_TEST(test_controller_enables_suction_when_pressure_is_high);
    RUN_TEST(test_controller_finishes_hold_after_duration);
    RUN_TEST(test_safety_detects_timeout);
    return UNITY_END();
}

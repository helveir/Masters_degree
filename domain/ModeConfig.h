#ifndef DOMAIN_MODE_CONFIG_H
#define DOMAIN_MODE_CONFIG_H

struct ModeConfig {
    int minTargetKpa;
    int maxTargetKpa;
    int defaultTargetKpa;
    int toleranceKpa;
    int alarmMarginKpa;
    unsigned long holdDurationMs;
    unsigned long maxRegulationTimeMs;
};

#endif

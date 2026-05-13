#ifndef DOMAIN_MODE_CONFIG_H
#define DOMAIN_MODE_CONFIG_H

struct ModeConfig {
    int minRangeKpa;
    int maxRangeKpa;
    int defaultRangeKpa;
    int toleranceKpa;
    int alarmMarginKpa;
    unsigned long maxPhaseTimeMs;
};

#endif

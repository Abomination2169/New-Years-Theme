#pragma once
#include "config-utils/shared/config-utils.hpp"
DECLARE_CONFIG(ModConfig) {
    CONFIG_VALUE(EnableConfetti, bool, "Enable Falling Confetti", true);
    CONFIG_VALUE(EnableMidnightLighting, bool, "Enable Midnight Lighting", true);
    CONFIG_VALUE(EnableCelebrationAmbience, bool, "Enable Celebration Ambience", true);
};

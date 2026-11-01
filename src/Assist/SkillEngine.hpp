#pragma once
#include <Geode/Geode.hpp>

namespace skill {

struct Decision {
    int delay = 0;        // frames to hold the click back
    float risk = 0.f;     // death chance (0-1) of the click as the player made it
    float best = 1.f;     // reliability (0-1) of the frame that was picked
};

struct Stats {
    int clicks = 0;
    int delayed = 0;
    long long totalDelay = 0;
};

namespace engine {
    void buildCache(GJBaseGameLayer* layer);   // once per level start
    void clearCache();
    bool supported(PlayerObject* p);           // cube only for now
    Decision evaluate(PlayerObject* p);        // called for every jump click
    Stats& stats();
    void resetStats();
}

}

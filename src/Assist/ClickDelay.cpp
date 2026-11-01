#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <algorithm>
#include <deque>
#include "SkillEngine.hpp"
#include "../Preset/SkillPreset.hpp"

using namespace geode::prelude;

namespace {

struct Pending { int left; bool down; int button; bool p1; };

std::deque<Pending> g_queue;
bool g_flushing = false;
int g_lastDelay[2] = {0, 0};   // delay given to the last push; its release reuses it

void clearQueue() {
    g_queue.clear();
    g_lastDelay[0] = g_lastDelay[1] = 0;
}

}

class $modify(SkillBaseLayer, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        if (!isHalfTick && !g_queue.empty()) {
            for (auto& e : g_queue) e.left--;
            g_flushing = true;
            while (!g_queue.empty() && g_queue.front().left <= 0) {
                auto e = g_queue.front();
                g_queue.pop_front();
                GJBaseGameLayer::handleButton(e.down, e.button, e.p1);
            }
            g_flushing = false;
        }
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }

    void handleButton(bool down, int button, bool p1) {
        auto& cfg = skill::settings();
        auto pl = PlayLayer::get();
        bool inLevel = pl && static_cast<GJBaseGameLayer*>(pl) == static_cast<GJBaseGameLayer*>(this);

        if (g_flushing || !cfg.enabled || !inLevel || button != 1) {
            GJBaseGameLayer::handleButton(down, button, p1);
            return;
        }

        int idx = p1 ? 0 : 1;
        int delay = 0;

        if (down) {
            auto player = p1 ? m_player1 : m_player2;
            auto dec = skill::engine::evaluate(player);
            delay = dec.delay;

            auto& st = skill::engine::stats();
            st.clicks++;
            if (delay > 0) { st.delayed++; st.totalDelay += delay; }

            if (!g_queue.empty()) delay = std::max(delay, g_queue.back().left);
            g_lastDelay[idx] = delay;
        } else {
            // release keeps the same shift as its push so hold length is preserved
            delay = g_lastDelay[idx];
            g_lastDelay[idx] = 0;
            if (!g_queue.empty()) delay = std::max(delay, g_queue.back().left);
        }

        if (delay <= 0 && g_queue.empty()) {
            GJBaseGameLayer::handleButton(down, button, p1);
            return;
        }
        g_queue.push_back({std::max(delay, 1), down, button, p1});
    }
};

class $modify(SkillPlayLayer, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        skill::engine::buildCache(this);
        skill::engine::resetStats();
        clearQueue();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        clearQueue();
    }

    void onQuit() {
        skill::engine::clearCache();
        clearQueue();
        PlayLayer::onQuit();
    }
};

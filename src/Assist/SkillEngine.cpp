#include "SkillEngine.hpp"
#include "../Preset/SkillPreset.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

using namespace geode::prelude;

namespace skill::engine {
namespace {

struct Obj {
    float minX, maxX, minY, maxY;
    bool round;
    float cx, cy, r;
};

std::vector<Obj> g_hazards;
std::vector<Obj> g_solids;
Stats g_stats;

// ---- physics constants (60 Hz units, scaled per physics step) -------------
constexpr float kTps       = 240.f;
constexpr float kStep      = 60.f / kTps;
constexpr float kGravity   = 0.958199f;
constexpr float kJump      = 11.180032f;
constexpr float kTerminal  = -15.f;
constexpr float kFloorTop  = 90.f;
constexpr float kMaxObjW   = 140.f;
constexpr float kEps       = 0.01f;
constexpr int   kHorizon   = 200;   // steps simulated after each candidate click
constexpr int   kHold      = 12;    // steps a click is assumed to stay held

struct Start {
    float x, y, vy, vx, half, jump;
    bool grounded;
};

struct SimResult { bool alive; bool jumped; };

Obj makeObj(GameObject* o, bool round) {
    auto r = o->getObjectRect();
    Obj b;
    b.minX = r.getMinX(); b.maxX = r.getMaxX();
    b.minY = r.getMinY(); b.maxY = r.getMaxY();
    b.round = round;
    b.cx = r.getMidX(); b.cy = r.getMidY();
    b.r = r.size.width * 0.5f;
    return b;
}

bool touches(const Obj& o, float x0, float y0, float x1, float y1, float pad) {
    if (o.round) {
        float cx = std::clamp(o.cx, x0, x1);
        float cy = std::clamp(o.cy, y0, y1);
        float dx = o.cx - cx, dy = o.cy - cy, rr = o.r + pad;
        return dx * dx + dy * dy < rr * rr;
    }
    return x1 > o.minX - pad + kEps && x0 < o.maxX + pad - kEps &&
           y1 > o.minY - pad + kEps && y0 < o.maxY + pad - kEps;
}

void gather(const std::vector<Obj>& src, float xMin, float xMax, bool withRound,
            std::vector<const Obj*>& out) {
    auto it = std::lower_bound(src.begin(), src.end(), xMin - kMaxObjW,
        [](const Obj& o, float v) { return o.minX < v; });
    for (; it != src.end() && it->minX <= xMax; ++it) {
        if (it->maxX < xMin) continue;
        if (it->round && !withRound) continue;
        out.push_back(&*it);
    }
}

bool standingOnSolid(const std::vector<const Obj*>& sl, float x, float y, float half) {
    for (auto o : sl) {
        if (std::fabs(o->maxY - (y - half)) < 0.6f && x + half > o->minX && x - half < o->maxX)
            return true;
    }
    return false;
}

// Runs the cube forward with a click at `clickFrame` and reports if it lives.
SimResult simulate(const Start& s, int clickFrame,
                   const std::vector<const Obj*>& hz, const std::vector<const Obj*>& sl,
                   const Settings& c, float xEnd) {
    float x = s.x, y = s.y, vy = s.vy;
    bool grounded = s.grounded, jumped = false;
    const float half = s.half;
    const float inner = half * 0.3f;   // inner hitbox: touching a block with it = crash

    for (int f = 0; f < kHorizon; ++f) {
        if (grounded && y - half > kFloorTop + 0.01f && !standingOnSolid(sl, x, y, half))
            grounded = false;   // walked off the end of a block

        bool held = f >= clickFrame && f < clickFrame + kHold;
        if (held && grounded) { vy = s.jump; grounded = false; jumped = true; }

        float prevBottom = y - half;
        if (!grounded) {
            vy = std::max(vy - kGravity * kStep, kTerminal);
            y += vy * kStep;
        }
        x += s.vx * kStep;

        if (y - half <= kFloorTop) { y = kFloorTop + half; vy = 0.f; grounded = true; }

        if (c.checkSolids) {
            for (auto o : sl) {
                if (!touches(*o, x - half, y - half, x + half, y + half, 0.f)) continue;
                if (vy <= 0.f && prevBottom >= o->maxY - 6.f) {
                    y = o->maxY + half; vy = 0.f; grounded = true;       // landed on top
                } else if (touches(*o, x - inner, y - inner, x + inner, y + inner, 0.f)) {
                    return {false, jumped};                               // side / ceiling crash
                }
            }
        }

        for (auto o : hz) {
            if (touches(*o, x - half, y - half, x + half, y + half, c.hazardPadding))
                return {false, jumped};
        }

        if (f >= clickFrame + kHold && x - half > xEnd) break;   // past everything
    }
    return {true, jumped};
}

} // namespace

void buildCache(GJBaseGameLayer* layer) {
    clearCache();
    if (!layer || !layer->m_objects) return;
    for (auto obj : CCArrayExt<GameObject*>(layer->m_objects)) {
        if (!obj) continue;
        switch (obj->m_objectType) {
            case GameObjectType::Hazard:
                g_hazards.push_back(makeObj(obj, obj->m_objectRadius > 0.f));
                break;
            case GameObjectType::Solid:
                g_solids.push_back(makeObj(obj, false));
                break;
            default: break;
        }
    }
    auto byX = [](const Obj& a, const Obj& b) { return a.minX < b.minX; };
    std::sort(g_hazards.begin(), g_hazards.end(), byX);
    std::sort(g_solids.begin(), g_solids.end(), byX);
}

void clearCache() {
    g_hazards.clear();
    g_solids.clear();
}

bool supported(PlayerObject* p) {
    if (!p) return false;
    if (p->m_isShip || p->m_isBird || p->m_isBall || p->m_isDart ||
        p->m_isRobot || p->m_isSpider || p->m_isSwing) return false;
    if (p->m_isUpsideDown || p->m_isSideways) return false;
    return true;
}

Decision evaluate(PlayerObject* p) {
    Decision d;
    auto& c = settings();
    if (!c.enabled || !supported(p)) return d;

    Start s;
    s.x = p->getPositionX();
    s.y = p->getPositionY();
    s.vy = static_cast<float>(p->m_yVelocity);
    s.grounded = p->m_isOnGround;
    s.vx = p->m_playerSpeed * 5.77f;
    float size = p->m_vehicleSize > 0.f ? p->m_vehicleSize : 1.f;
    s.half = 15.f * size;
    s.jump = kJump * (size < 1.f ? 0.8f : 1.f);   // mini jump is lower; tune if it feels off

    const int N = std::max(1, c.lookahead);
    const int J = std::max(0, (c.samples - 1) / 2);

    float xMin = s.x - 40.f;
    float xMax = s.x + s.vx * kStep * kHorizon + 60.f;
    std::vector<const Obj*> hz, sl;
    if (c.checkHazards) gather(g_hazards, xMin, xMax, c.checkSaws, hz);
    if (c.checkSolids)  gather(g_solids, xMin, xMax, true, sl);
    if (hz.empty() && sl.empty()) return d;   // nothing ahead, click passes untouched

    float xEnd = -1e9f;
    for (auto o : hz) xEnd = std::max(xEnd, o->maxX);
    for (auto o : sl) xEnd = std::max(xEnd, o->maxX);

    // airborne: only plan if this click would actually become a jump (landing soon)
    if (!s.grounded) {
        auto r = simulate(s, 0, hz, sl, c, xEnd);
        if (!r.jumped) return d;   // orb / pad click, leave it alone
    }

    const int maxF = N + J;
    std::vector<char> ok(maxF + 1, 0);
    for (int f = 0; f <= maxF; ++f) {
        auto r = simulate(s, f, hz, sl, c, xEnd);
        ok[f] = (r.alive && r.jumped) ? 1 : 0;
    }

    // reliability of frame f = triangular-weighted survival of f-J ... f+J.
    // Frames at the edge of the safe window score low, the middle scores high.
    auto reliability = [&](int f) {
        float acc = 0.f, w = 0.f;
        for (int dl = -J; dl <= J; ++dl) {
            int ff = std::clamp(f + dl, 0, maxF);
            float wt = static_cast<float>(J + 1 - std::abs(dl));
            acc += wt * ok[ff];
            w += wt;
        }
        return w > 0.f ? acc / w : 0.f;
    };

    float rel0 = reliability(0);
    d.risk = 1.f - rel0;
    d.best = rel0;
    if (d.risk * 100.f <= c.deathRate) return d;   // risk is acceptable, don't touch it

    int best = 0;
    float bestRel = rel0;
    for (int f = 1; f <= N; ++f) {
        float rel = reliability(f);
        if (rel > bestRel + 1e-4f) { best = f; bestRel = rel; }
    }
    d.delay = best;
    d.best = bestRel;
    return d;
}

Stats& stats() { return g_stats; }
void resetStats() { g_stats = Stats{}; }

}

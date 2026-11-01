#include "SkillPreset.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace skill {

static Settings g;
static bool g_loaded = false;

Settings& settings() {
    if (!g_loaded) loadSettings();
    return g;
}

void loadSettings() {
    auto m = Mod::get();
    g.enabled       = m->getSavedValue<bool>("enabled", true);
    g.preset        = static_cast<PresetId>(m->getSavedValue<int>("preset", 0));
    g.lookahead     = m->getSavedValue<int>("lookahead", 8);
    g.deathRate     = m->getSavedValue<float>("death-rate", 40.f);
    g.samples       = m->getSavedValue<int>("samples", 5);
    g.checkHazards  = m->getSavedValue<bool>("check-hazards", true);
    g.checkSaws     = m->getSavedValue<bool>("check-saws", false);
    g.checkSolids   = m->getSavedValue<bool>("check-solids", false);
    g.hazardPadding = m->getSavedValue<float>("hazard-padding", 0.f);
    g_loaded = true;
}

void saveSettings() {
    auto m = Mod::get();
    m->setSavedValue("enabled", g.enabled);
    m->setSavedValue("preset", static_cast<int>(g.preset));
    m->setSavedValue("lookahead", g.lookahead);
    m->setSavedValue("death-rate", g.deathRate);
    m->setSavedValue("samples", g.samples);
    m->setSavedValue("check-hazards", g.checkHazards);
    m->setSavedValue("check-saws", g.checkSaws);
    m->setSavedValue("check-solids", g.checkSolids);
    m->setSavedValue("hazard-padding", g.hazardPadding);
}

void applyPreset(PresetId id) {
    auto& s = settings();
    s.preset = id;
    switch (id) {
        case PresetId::Top500:
            // spikes only, short look-ahead, lets a lot of sloppy clicks through
            s.lookahead = 8;  s.deathRate = 40.f; s.samples = 5;
            s.checkHazards = true; s.checkSaws = false; s.checkSolids = false; s.hazardPadding = 0.f;
            break;
        case PresetId::Top100:
            // + saws and solid blocks, wider look-ahead, stricter risk
            s.lookahead = 14; s.deathRate = 15.f; s.samples = 9;
            s.checkHazards = true; s.checkSaws = true; s.checkSolids = true; s.hazardPadding = 1.f;
            break;
        case PresetId::Top1:
            // everything on, long look-ahead, near-zero risk, padded hazards
            s.lookahead = 24; s.deathRate = 3.f;  s.samples = 15;
            s.checkHazards = true; s.checkSaws = true; s.checkSolids = true; s.hazardPadding = 2.5f;
            break;
        case PresetId::Custom:
            break;
    }
    saveSettings();
}

const char* presetName(PresetId id) {
    switch (id) {
        case PresetId::Top500: return "Top 500 skill";
        case PresetId::Top100: return "Top 100 skill";
        case PresetId::Top1:   return "Top 1 skill";
        default:               return "Custom";
    }
}

const char* presetBlurb(PresetId id) {
    switch (id) {
        case PresetId::Top500: return "Spikes only. Short look-ahead, forgiving.";
        case PresetId::Top100: return "Spikes + saws + blocks. Wider look-ahead.";
        case PresetId::Top1:   return "Full hazard/solid checks. Almost never dies.";
        default:               return "Your own values.";
    }
}

}

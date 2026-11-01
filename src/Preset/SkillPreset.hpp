#pragma once

namespace skill {

enum class PresetId : int { Top500 = 0, Top100 = 1, Top1 = 2, Custom = 3 };

struct Settings {
    bool enabled = true;
    PresetId preset = PresetId::Top500;
    int lookahead = 8;          // frames (physics steps) looked ahead = max click delay
    float deathRate = 40.f;     // % risk a click may carry before it gets moved
    int samples = 5;            // timing-jitter samples used to rate every frame
    bool checkHazards = true;   // spikes + other rect hazards
    bool checkSaws = false;     // round hazards
    bool checkSolids = false;   // blocks: landings, side hits, ceilings
    float hazardPadding = 0.f;  // extra units around hazards (safer = bigger)
};

Settings& settings();
void loadSettings();
void saveSettings();
void applyPreset(PresetId id);
const char* presetName(PresetId id);
const char* presetBlurb(PresetId id);

}

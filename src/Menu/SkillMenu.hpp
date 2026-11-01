#pragma once
#include <Geode/Geode.hpp>

// Geode v5: Popup is no longer templated; init replaces setup/initAnchored.
class SkillMenu : public geode::Popup {
protected:
    ButtonSprite* m_presetSprites[3] = {nullptr, nullptr, nullptr};
    Slider* m_lookSlider = nullptr;
    Slider* m_deathSlider = nullptr;
    CCMenuItemToggler* m_toggles[4] = {nullptr, nullptr, nullptr, nullptr}; // enabled, hazards, saws, solids
    cocos2d::CCLabelBMFont* m_presetLabel = nullptr;
    cocos2d::CCLabelBMFont* m_blurbLabel = nullptr;
    cocos2d::CCLabelBMFont* m_lookLabel = nullptr;
    cocos2d::CCLabelBMFont* m_deathLabel = nullptr;
    cocos2d::CCLabelBMFont* m_statsLabel = nullptr;

    bool initMenu();
    void refresh();
    void markCustom();

    void onPreset(cocos2d::CCObject* sender);
    void onToggle(cocos2d::CCObject* sender);
    void onLook(cocos2d::CCObject* sender);
    void onDeath(cocos2d::CCObject* sender);

public:
    ~SkillMenu() override;
    static SkillMenu* create();
    static void toggle();   // open if closed, close if open
};

#include "SkillMenu.hpp"
#include "../Preset/SkillPreset.hpp"
#include "../Assist/SkillEngine.hpp"

using namespace geode::prelude;

static SkillMenu* s_open = nullptr;

SkillMenu::~SkillMenu() {
    if (s_open == this) s_open = nullptr;
}

SkillMenu* SkillMenu::create() {
    auto ret = new SkillMenu();
    if (ret->initMenu()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

void SkillMenu::toggle() {
    if (s_open) {
        auto m = s_open;
        s_open = nullptr;
        m->onClose(nullptr);
        return;
    }
    if (auto m = SkillMenu::create()) {
        s_open = m;
        m->show();
    }
}

bool SkillMenu::initMenu() {
    constexpr float W = 340.f;
    constexpr float H = 270.f;

    if (!Popup::init(W, H)) return false;
    this->setTitle("Skill Assist");

    auto at = [&](float x, float y) { return ccp(x - W / 2.f, y - H / 2.f); };

    auto label = [&](const char* text, float scale, CCPoint pos, CCPoint anchor = ccp(.5f, .5f)) {
        auto l = CCLabelBMFont::create(text, "bigFont.fnt");
        l->setScale(scale);
        l->setAnchorPoint(anchor);
        m_mainLayer->addChildAtPosition(l, Anchor::Center, pos);
        return l;
    };

    auto toggler = [&](int tag, const char* text, float x, float y) {
        auto t = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(SkillMenu::onToggle), .6f);
        t->setTag(tag);
        m_buttonMenu->addChildAtPosition(t, Anchor::Center, at(x, y));
        label(text, .4f, at(x + 16.f, y), ccp(0.f, .5f));
        return t;
    };

    // master switch
    m_toggles[0] = toggler(0, "Assist enabled", 38.f, H - 50.f);

    // presets
    const char* names[3] = {"Top 500", "Top 100", "Top 1"};
    for (int i = 0; i < 3; ++i) {
        auto spr = ButtonSprite::create(names[i], "bigFont.fnt", "GJ_button_01.png", .8f);
        spr->setScale(.7f);
        m_presetSprites[i] = spr;
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SkillMenu::onPreset));
        btn->setTag(i);
        m_buttonMenu->addChildAtPosition(btn, Anchor::Center, at(W / 2.f + (i - 1) * 100.f, H - 88.f));
    }
    m_presetLabel = label("", .45f, at(W / 2.f, H - 118.f));
    m_blurbLabel  = label("", .33f, at(W / 2.f, H - 134.f));

    // look-ahead frames
    m_lookLabel = label("", .4f, at(W / 2.f, H - 156.f));
    m_lookSlider = Slider::create(this, menu_selector(SkillMenu::onLook), .7f);
    m_buttonMenu->addChildAtPosition(m_lookSlider, Anchor::Center, at(W / 2.f, H - 174.f));

    // death rate
    m_deathLabel = label("", .4f, at(W / 2.f, H - 196.f));
    m_deathSlider = Slider::create(this, menu_selector(SkillMenu::onDeath), .7f);
    m_buttonMenu->addChildAtPosition(m_deathSlider, Anchor::Center, at(W / 2.f, H - 214.f));

    // check toggles
    m_toggles[1] = toggler(1, "Hazards", 26.f,  H - 240.f);
    m_toggles[2] = toggler(2, "Saws",    130.f, H - 240.f);
    m_toggles[3] = toggler(3, "Solids",  220.f, H - 240.f);

    m_statsLabel = label("", .3f, at(W / 2.f, 12.f));

    this->refresh();
    return true;
}

void SkillMenu::refresh() {
    auto& c = skill::settings();

    for (int i = 0; i < 3; ++i) {
        bool active = static_cast<int>(c.preset) == i;
        m_presetSprites[i]->updateBGImage(active ? "GJ_button_02.png" : "GJ_button_01.png");
    }

    m_presetLabel->setString(fmt::format("Active: {}", skill::presetName(c.preset)).c_str());
    m_blurbLabel->setString(skill::presetBlurb(c.preset));
    m_lookLabel->setString(fmt::format("Look-ahead: {} frames", c.lookahead).c_str());
    m_deathLabel->setString(fmt::format("Death rate: {:.0f}%", c.deathRate).c_str());

    m_lookSlider->setValue((c.lookahead - 1) / 59.f);
    m_deathSlider->setValue(c.deathRate / 100.f);

    m_toggles[0]->toggle(c.enabled);
    m_toggles[1]->toggle(c.checkHazards);
    m_toggles[2]->toggle(c.checkSaws);
    m_toggles[3]->toggle(c.checkSolids);

    auto& st = skill::engine::stats();
    float avg = st.delayed ? static_cast<float>(st.totalDelay) / st.delayed : 0.f;
    m_statsLabel->setString(
        fmt::format("This level: {} clicks | {} moved | avg shift {:.1f}f", st.clicks, st.delayed, avg).c_str());
}

void SkillMenu::markCustom() {
    skill::settings().preset = skill::PresetId::Custom;
    skill::saveSettings();
    this->refresh();
}

void SkillMenu::onPreset(CCObject* sender) {
    skill::applyPreset(static_cast<skill::PresetId>(sender->getTag()));
    this->refresh();
}

void SkillMenu::onToggle(CCObject* sender) {
    auto t = static_cast<CCMenuItemToggler*>(sender);
    bool on = !t->isToggled();   // callback fires before the toggler flips
    auto& c = skill::settings();
    switch (t->getTag()) {
        case 0: c.enabled = on; skill::saveSettings(); this->refresh(); return;
        case 1: c.checkHazards = on; break;
        case 2: c.checkSaws = on; break;
        case 3: c.checkSolids = on; break;
    }
    this->markCustom();
}

void SkillMenu::onLook(CCObject* sender) {
    float v = static_cast<SliderThumb*>(sender)->getValue();
    skill::settings().lookahead = 1 + static_cast<int>(std::round(v * 59.f));
    this->markCustom();
}

void SkillMenu::onDeath(CCObject* sender) {
    float v = static_cast<SliderThumb*>(sender)->getValue();
    skill::settings().deathRate = std::round(v * 100.f);
    this->markCustom();
}

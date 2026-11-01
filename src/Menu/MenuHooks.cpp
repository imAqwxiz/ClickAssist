#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "SkillIcon.hpp"
#include "SkillMenu.hpp"

using namespace geode::prelude;

// Main menu: static button in the bottom row.
class $modify(SkillMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto btn = CCMenuItemSpriteExtra::create(
            skillIconSprite(1.f), this, menu_selector(SkillMenuLayer::onSkillIcon));
        btn->setID("skill-assist-button"_spr);

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        }
        return true;
    }

    void onSkillIcon(CCObject*) {
        SkillMenu::toggle();
    }
};

// Pause menu: draggable icon.
class $modify(SkillPause, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        if (auto icon = SkillIcon::create()) {
            icon->setID("skill-assist-icon"_spr);
            this->addChild(icon, 100);
        }
    }
};

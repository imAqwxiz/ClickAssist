#include "SkillIcon.hpp"
#include "SkillMenu.hpp"
#include <algorithm>

using namespace geode::prelude;

CCSprite* skillIconSprite(float scale) {
    auto spr = CircleButtonSprite::createWithSpriteFrameName(
        "GJ_sMagicIcon_001.png", 1.f, CircleBaseColor::Green, CircleBaseSize::Medium);
    spr->setScale(scale);
    return spr;
}

SkillIcon* SkillIcon::create() {
    auto ret = new SkillIcon();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool SkillIcon::init() {
    if (!CCLayer::init()) return false;

    m_sprite = skillIconSprite(.75f);
    auto base = m_sprite->getContentSize();
    CCSize size{base.width * m_sprite->getScale(), base.height * m_sprite->getScale()};

    this->setContentSize(size);
    this->ignoreAnchorPointForPosition(false);
    this->setAnchorPoint({.5f, .5f});
    m_sprite->setPosition({size.width / 2.f, size.height / 2.f});
    m_sprite->setOpacity(200);
    this->addChild(m_sprite);

    auto win = CCDirector::get()->getWinSize();
    auto mod = Mod::get();
    this->setPosition({
        mod->getSavedValue<float>("icon-x", .94f) * win.width,
        mod->getSavedValue<float>("icon-y", .84f) * win.height
    });

    this->setTouchMode(kCCTouchesOneByOne);
    this->setTouchEnabled(true);
    return true;
}

void SkillIcon::registerWithTouchDispatcher() {
    // very negative = above the pause buttons
    CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, -600, true);
}

bool SkillIcon::ccTouchBegan(CCTouch* touch, CCEvent*) {
    if (!this->isVisible() || !this->getParent()) return false;
    auto local = this->convertToNodeSpace(touch->getLocation());
    auto size = this->getContentSize();
    if (local.x < 0.f || local.y < 0.f || local.x > size.width || local.y > size.height) return false;

    m_down = true;
    m_moved = false;
    m_startLoc = touch->getLocation();
    m_grab = this->getParent()->convertToNodeSpace(touch->getLocation()) - this->getPosition();
    m_sprite->setOpacity(255);
    return true;
}

void SkillIcon::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (!m_down) return;
    auto loc = touch->getLocation();
    if (!m_moved) {
        float dx = loc.x - m_startLoc.x, dy = loc.y - m_startLoc.y;
        if (dx * dx + dy * dy < 64.f) return;   // 8px dead zone so taps don't drag
        m_moved = true;
    }
    auto win = CCDirector::get()->getWinSize();
    auto size = this->getContentSize();
    auto pos = this->getParent()->convertToNodeSpace(loc) - m_grab;
    pos.x = std::clamp(pos.x, size.width / 2.f, win.width - size.width / 2.f);
    pos.y = std::clamp(pos.y, size.height / 2.f, win.height - size.height / 2.f);
    this->setPosition(pos);
}

void SkillIcon::ccTouchEnded(CCTouch*, CCEvent*) {
    if (!m_down) return;
    m_down = false;
    m_sprite->setOpacity(200);
    if (m_moved) {
        this->savePosition();
    } else {
        SkillMenu::toggle();
    }
}

void SkillIcon::ccTouchCancelled(CCTouch*, CCEvent*) {
    m_down = false;
    m_sprite->setOpacity(200);
    if (m_moved) this->savePosition();
}

void SkillIcon::savePosition() {
    auto win = CCDirector::get()->getWinSize();
    auto pos = this->getPosition();
    Mod::get()->setSavedValue("icon-x", pos.x / win.width);
    Mod::get()->setSavedValue("icon-y", pos.y / win.height);
}

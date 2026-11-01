#pragma once
#include <Geode/Geode.hpp>

// Round icon used by both the main-menu button and the draggable pause icon.
cocos2d::CCSprite* skillIconSprite(float scale = 1.f);

// Draggable icon: drag to move (position is saved), tap to toggle the menu.
class SkillIcon : public cocos2d::CCLayer {
protected:
    cocos2d::CCSprite* m_sprite = nullptr;
    cocos2d::CCPoint m_grab;
    cocos2d::CCPoint m_startLoc;
    bool m_down = false;
    bool m_moved = false;

    bool init() override;
    void registerWithTouchDispatcher() override;
    bool ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) override;
    void savePosition();

public:
    static SkillIcon* create();
};

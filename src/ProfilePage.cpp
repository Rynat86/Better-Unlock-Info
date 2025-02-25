#include "Includes.hpp"
#include <Geode/modify/ProfilePage.hpp>


//adds jetpack and death effect to profile and makes icons into buttons that show unlock popup, fps warning popoup if funny
class $modify(MyProfilePage, ProfilePage) 
{
    static void onModify(auto& self)
    {
        //idk why this is here anymore xd
        //(void) self.setHookPriority("ProfilePage::loadPageFromUserInfo", -1000);
    }
    
	void loadPageFromUserInfo(GJUserScore* p0) 
    {
		ProfilePage::loadPageFromUserInfo(p0);
        
        auto playerMenu = m_mainLayer->getChildByID("player-menu");
        if (playerMenu == nullptr) return;
        
        //check if broken glow
        fixGlow(playerMenu);
        
        //replaces icons with buttons
        std::vector<std::string> ids;
        for (auto node : CCArrayExt<CCNode*>(playerMenu->getChildren()))
            ids.push_back(node->getID());
        for (auto id : ids)
            replacePlayerIconNodeWithButton(id, playerMenu);
        
        //adds jetpack and death
        if (Mod::get()->getSettingValue<bool>("jetpackToggle"))
            addJetpack(playerMenu);
        
        if (Mod::get()->getSettingValue<bool>("deathEffectToggle"))
            addDeathEffect(playerMenu);
        
        //update layout
        playerMenu->updateLayout();
        
        //warning for fps drops
        int profileCount = 0;
        for (auto node : CCArrayExt<CCNode*>(CCScene::get()->getChildren()))
            if (typeinfo_cast<ProfilePage*>(node) != nullptr)
                profileCount++;
        
        if (profileCount == 3) 
            if (!Mod::get()->setSavedValue("shown-profile-fps-popup", true))
            {
                auto popup = FLAlertLayer::create("Note", "If you continue doing this your fps will drop a lot so be careful", "OK");
                popup->m_scene = this;
                popup->show();
            }
            
        //if no jetpack    
        if (!Mod::get()->getSettingValue<bool>("jetpackToggle") && !Mod::get()->getSavedValue<bool>("shown-no-jetpack"))
        { 
            auto popup = geode::createQuickPopup(
            "No Jetpack", "Without the Jetpack enabled you won't be able to see the Jetpack at all, not even by clicking the ship icon",
            "OK", "Don't show agian",
            [](auto, bool btn2)
            { 
                if (btn2)
                    Mod::get()->setSavedValue<bool>(("shown-no-jetpack"), true);
            }, false);
            
            popup->m_scene = this;
            popup->show();
        }
	}
    
    void addJetpack(CCNode* playerMenu)
    {
        //set up icon
        SimplePlayer* jetpackPlayer = SimplePlayer::create(0);
     	jetpackPlayer->updatePlayerFrame(m_score->m_playerJetpack, IconType::Jetpack);
        jetpackPlayer->setID("player-jetpack");
        jetpackPlayer->setScale(0.95f);

        //set colors
        auto GM = GameManager::get();
        jetpackPlayer->setColor(GM->colorForIdx(m_score->m_color1));
        jetpackPlayer->setSecondColor(GM->colorForIdx(m_score->m_color2));
        if (m_score->m_glowEnabled) jetpackPlayer->setGlowOutline(GM->colorForIdx(m_score->m_color3));

        //add and set up button
        auto jetpackButton = CCMenuItemSpriteExtra::create(jetpackPlayer, this, menu_selector(MyProfilePage::onIconClick));
        jetpackButton->setID("player-jetpack");
        jetpackButton->setAnchorPoint(CCPoint(0.5f, 0.5f));
        jetpackButton->setContentSize(CCSize(42.6f, 42.6f));
		jetpackButton->getChildByID("player-jetpack")->setPosition(CCPoint(17.3f, 21.3f));
		
        //add to row
        playerMenu->addChild(jetpackButton);
    }
    
    void addDeathEffect(CCNode* playerMenu)
    {
        //invalid check
        if(m_score->m_playerExplosion <= 0 || m_score->m_playerExplosion > 20) return;
        
        //set up icon
        SimplePlayer* deathEffectPlayer = SimplePlayer::create(0);
        deathEffectPlayer->removeAllChildren();
        
        auto deathEffectSprite = GJItemIcon::createBrowserItem(UnlockType::Death, m_score->m_playerExplosion)->getChildByType<CCSprite>(0);
        deathEffectSprite->setPosition(CCPoint(0, 0));
        deathEffectSprite->setScale(1.1);
        
        deathEffectPlayer->addChild(deathEffectSprite);
        deathEffectPlayer->setID("player-deathEffect");
        deathEffectPlayer->setScale(0.95f);

        //add and set up button
        auto deathEffectButton = CCMenuItemSpriteExtra::create(deathEffectPlayer, this, menu_selector(MyProfilePage::onIconClick));
        deathEffectButton->setID("player-deathEffect");
        deathEffectButton->setAnchorPoint(CCPoint(0.5f, 0.5f));
        deathEffectButton->setContentSize(CCSize(42.6f, 42.6f));
        deathEffectButton->getChildByID("player-deathEffect")->setPosition(CCPoint(18.3f, 21.3f));
        
        //add to row
        playerMenu->addChild(deathEffectButton);
    }
    
    void replacePlayerIconNodeWithButton(std::string nodeId, CCNode* playerMenu)
    {
        //creates button from icon        
        auto iconPlayer = playerMenu->getChildByID(nodeId)->getChildByType<SimplePlayer>(0);
        auto iconButton = CCMenuItemSpriteExtra::create(iconPlayer, this, menu_selector(MyProfilePage::onIconClick));
        
        //swaps with original so its on correct place
        playerMenu->addChild(iconButton);
        swapChildIndices(playerMenu->getChildByID(nodeId), iconButton);
        playerMenu->removeChildByID(nodeId);
        
        //conflict animated profiles
        iconButton->removeAllChildren();
        iconButton->addChild(iconPlayer);
        
        //size
        iconButton->setContentSize(CCSize(42.6f, 42.6f));
        if (nodeId == "player-wave")
            iconButton->setContentSize(CCSize(36.6f, 42.6f));
        
        //fix pos
        iconPlayer->setPosition(CCPoint(21.3f, 21.3f));
        
        iconButton->setID(nodeId);
    }

    IconObject getUnlockData(std::string buttonID, GJUserScore* m_score)
    {
        std::unordered_map<std::string, IconObject> buttonToIconObject = 
        {
            {"player-icon", {UnlockType::Cube, m_score->m_playerCube}},
            {"player-ship", {UnlockType::Ship, m_score->m_playerShip}},
            {"player-ball", {UnlockType::Ball, m_score->m_playerBall}},
            {"player-ufo", {UnlockType::Bird, m_score->m_playerUfo}},
            {"player-wave", {UnlockType::Dart, m_score->m_playerWave}},
            {"player-robot", {UnlockType::Robot, m_score->m_playerRobot}},
            {"player-spider", {UnlockType::Spider, m_score->m_playerSpider}},
            {"player-swing", {UnlockType::Swing, m_score->m_playerSwing}},
            {"player-jetpack", {UnlockType::Jetpack, m_score->m_playerJetpack}},
            {"player-deathEffect", {UnlockType::Death, m_score->m_playerExplosion}}
        };
    
        return buttonToIconObject[buttonID];
    }

	void onIconClick(CCObject* sender)
    {
        //shows popup
        CCMenuItemSpriteExtra* button = as<CCMenuItemSpriteExtra*>(sender);
        IconObject icon = getUnlockData(button->getID(), m_score);
        ItemInfoPopup::create(icon.iconId, icon.unlockType)->show();
	}
    
    void fixGlow(CCNode* playerMenu)
    {
        auto simpleIcon = typeinfo_cast<SimplePlayer*>(playerMenu->getChildByID("player-icon")->getChildByID("player-icon"));
        if (!simpleIcon->m_hasCustomGlowColor)
            m_score->m_color3 = m_score->m_color2;
    }
};
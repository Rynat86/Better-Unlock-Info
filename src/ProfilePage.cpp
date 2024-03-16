#include "Includes.hpp"
#include <Geode/modify/ProfilePage.hpp>


//adds jetpack and death effect to profile and makes icons into buttons that show unlock popup, fps warning popoup if funny
class $modify(MyProfilePage, ProfilePage) 
{
	TodoReturn getUserInfoFinished(GJUserScore* p0) 
    {
		ProfilePage::getUserInfoFinished(p0);
        
        if (Mod::get()->getSettingValue<bool>("jetpackToggle"))
            addJetpack();
        
        if (Mod::get()->getSettingValue<bool>("deathEffectToggle"))
            addDeathEffect();
        
        
        //replaces icons with buttons
        std::list<std::string> ids;
        for (auto node : CCArrayExt<CCNode*>(m_mainLayer->getChildByID("player-menu")->getChildren()))
            ids.push_back(node->getID());
        for (auto id : ids)
            replacePlayerIconNodeWithButton(id);


        m_mainLayer->getChildByID("player-menu")->updateLayout();
	}
    
    void addJetpack()
    {
        //set up icon
        SimplePlayer* jetpackPlayer = SimplePlayer::create(0);
     	jetpackPlayer->updatePlayerFrame(m_score->m_playerJetpack, IconType::Jetpack);
        jetpackPlayer->setID("player-jetpack");
        jetpackPlayer->setScale(0.95f);

        //set colors
        auto mainColor = getChildOfType<CCSprite>(GJItemIcon::createBrowserItem(UnlockType::Col1, m_score->m_color1), 0);
        auto secondColor = getChildOfType<CCSprite>(GJItemIcon::createBrowserItem(UnlockType::Col2, m_score->m_color2), 0);
        auto glowColor = getChildOfType<CCSprite>(GJItemIcon::createBrowserItem(UnlockType::Col2, m_score->m_color3), 0);
        jetpackPlayer->setColor(mainColor->getColor());
        jetpackPlayer->setSecondColor(secondColor->getColor());
        if (m_score->m_glowEnabled) jetpackPlayer->setGlowOutline(glowColor->getColor());

        //add and set up button
        auto jetpackButton = CCMenuItemSpriteExtra::create(jetpackPlayer, this, menu_selector(MyProfilePage::onIconClick));
        jetpackButton->setID("player-jetpack");
        jetpackButton->setAnchorPoint(CCPoint(0.5f, 0.5f));
        jetpackButton->setContentSize(CCSize(42.6f, 42.6f));
		jetpackButton->getChildByID("player-jetpack")->setPosition(CCPoint(17.3f, 21.3f));
		
        //add to row
        m_mainLayer->getChildByID("player-menu")->addChild(jetpackButton);
    }
    
    void addDeathEffect()
    {
        //set up icon
        SimplePlayer* deathEffectPlayer = SimplePlayer::create(0);
        deathEffectPlayer->removeAllChildren();
        
        auto deathEffectSprite = getChildOfType<CCSprite>(GJItemIcon::createBrowserItem(UnlockType::Death, m_score->m_playerExplosion), 0);
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
        m_mainLayer->getChildByID("player-menu")->addChild(deathEffectButton);
    }
    
    void replacePlayerIconNodeWithButton(const std::string &nodeId)
    {
        //jetpack, death effect is already set
        if (nodeId == "player-jetpack" || nodeId == "player-deathEffect") return;
    
        //creates button from icon        
        auto iconPlayer = getChildOfType<SimplePlayer>(m_mainLayer->getChildByIDRecursive(nodeId), 0);
        auto iconButton = CCMenuItemSpriteExtra::create(iconPlayer, this, menu_selector(MyProfilePage::onIconClick));
        iconButton->setContentSize(CCSize(42.6f, 42.6f));
        if (nodeId == "player-wave") iconButton->setContentSize(CCSize(36.6f, 42.6f));
        
        //adds to row
        m_mainLayer->getChildByID("player-menu")->addChild(iconButton);
        
        //swaps witch original so its on correct place
        swapChildIndices(m_mainLayer->getChildByIDRecursive(nodeId), iconButton);
        m_mainLayer->getChildByID("player-menu")->removeChildByID(nodeId);
        
        //fix
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
        //shows popup and sets tag for later
        CCMenuItemSpriteExtra* button = static_cast<CCMenuItemSpriteExtra*>(sender);
        
        if (button->getID() != "player-deathEffect")
            button->setTag(1);
        
        IconObject icon = getUnlockData(button->getID(), m_score);
        ItemInfoPopup::create(icon.iconId, icon.unlockType)->show();
        
        //warning for fps drops
        int profileCount = 0;
        for (auto node : CCArrayExt<CCNode*>(CCScene::get()->getChildren()))
            if (typeinfo_cast<ProfilePage*>(node) != nullptr)
                profileCount++;
        if (profileCount == 3) 
            if (!Mod::get()->setSavedValue("shown-profile-fps-popup", true))
                FLAlertLayer::create("Note", "If you continue doing this your fps will drop a lot so be careful", "OK")->show();
	}
};
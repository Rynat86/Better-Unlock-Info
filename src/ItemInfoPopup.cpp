#include "Includes.hpp"
#include <Geode/modify/ItemInfoPopup.hpp>
#include <matjson.hpp>


//replaces grayscale icon with users, adds colors and rest aka lazy to write - in about.md
class $modify(MyItemInfoPopup, ItemInfoPopup) 
{
    std::list<ProfilePage*> profileList;
    
    bool init(int IconId, UnlockType UnlockType) 
    {
        if (!ItemInfoPopup::init(IconId, UnlockType)) return false; 
        
        addDetailButton(IconId, UnlockType);
        if (UnlockType == UnlockType::ShipFire || UnlockType == UnlockType::GJItem) return true; //note, icon type for isIconUnlocked()
        {
            if (!(Mod::get()->getSettingValue<bool>("equipToggle")))
                addEquipButton(IconId, UnlockType);
            
            if (!(Mod::get()->getSettingValue<bool>("progressToggle")))
                addCompletionProgress(IconId, UnlockType);
            else
                addCompletionIconOnly(IconId, UnlockType);
        }
        
        //moves credit
        for (auto button : CCArrayExt<CCMenuItemSpriteExtra*>(getChildOfType<CCMenu>(m_mainLayer, 0)->getChildren()))
            if (typeinfo_cast<CCLabelBMFont*>(button->getChildByTag(1)) != nullptr)
            {
                if (button->getID()[0] == 'c')
                {
                    button->removeFromParent(); //sorry cvolton
                    continue;
                }
                button->setPosition(CCPoint(0, 173));
                button->m_baseScale = 0.8f;
                button->setScale(0.8f);
                
                auto originalIcon = getChildOfType<GJItemIcon>(m_mainLayer, 0);
                originalIcon->setTag(1);
                originalIcon->setPositionY(originalIcon->getPositionY() - 6);
            }

        
        //if this is unlock, dont add colors related stuff
        int badUnlocks[] = {2, 3, 10, 11, 12, 15};
        bool exists = std::find(std::begin(badUnlocks), std::end(badUnlocks), static_cast<int>(UnlockType)) != std::end(badUnlocks);
        if (exists) return true;
        
        iconSwap(IconId, UnlockType, false);
        
        if (!(Mod::get()->getSettingValue<bool>("useMyColorsToggle")))   
        adduseMyColorsCheckBox();
        
        
        
        //profile check
        if (CCScene::get()->getChildByID("ProfilePage") == nullptr) return true;
        
        //finds newest profile (prob could be better ngl)
        m_fields->profileList.clear();
        for (auto node : CCArrayExt<CCNode*>(CCScene::get()->getChildren()))
            if (typeinfo_cast<ProfilePage*>(node) != nullptr)
                m_fields->profileList.push_back(static_cast<ProfilePage*>(node));        

        iconSwap(IconId, UnlockType, true);
        addColors();

        return true;
    }
    
    void iconSwap(int iconId, UnlockType unlockType, bool OnProfile)
    {
        //adds tag to original icon
        auto originalIcon = m_mainLayer->getChildByTag(1);
        if (originalIcon == nullptr)
        {
            originalIcon = getChildOfType<GJItemIcon>(m_mainLayer, 0);
            originalIcon->setTag(1);
        }
        
        if (OnProfile)
        {
            //finds clicked icon
            auto profile = m_fields->profileList.back();
            auto userIconButton = profile->getChildByIDRecursive("player-menu")->getChildByTag(1);
            userIconButton->setTag(-1);
        
            //copies icon from profile
            auto userIcon = SimplePlayer::create(0);
            
            //conflict animated profiles
            if (Loader::get()->isModLoaded("thesillydoggo.animatedprofiles") && unlockType == UnlockType::Robot || unlockType == UnlockType::Spider)
            {
                userIcon->updatePlayerFrame(iconId, UnlockToIcon(unlockType));
                
                auto GM = GameManager::get();
                userIcon->setColor(GM->colorForIdx(profile->m_score->m_color1));
                userIcon->setSecondColor(GM->colorForIdx(profile->m_score->m_color2));
                if (profile->m_score->m_glowEnabled) userIcon->setGlowOutline(GM->colorForIdx(profile->m_score->m_color3));
                
                if (unlockType == UnlockType::Robot)
                {
                    userIcon->createRobotSprite(iconId);
                    userIcon->m_robotSprite->runAnimation("idle01");
                }
                if (unlockType == UnlockType::Spider)
                {
                    userIcon->createSpiderSprite(iconId);
                    userIcon->m_spiderSprite->runAnimation("idle01");
                }
            }
            else
            {
                userIcon->removeAllChildren();
                userIcon->addChild(getChildOfType<CCSprite>(userIconButton->getChildByTag(1), 0));
            }
            userIcon->setScale(1);
            userIcon->setPosition(CCPoint(15, 15));

            //replaces black and white version
            originalIcon->removeAllChildren();
            originalIcon->addChild(userIcon);
			return;
        }

        
        //adds useMyColorsToggle icon
        auto myColorIcon = GJItemIcon::createBrowserItem(unlockType, iconId);
        myColorIcon->setID("useMyColorsToggle");
        myColorIcon->setTag(2);
        myColorIcon->setVisible(false);
        
        auto GM = GameManager::get();
        auto myColorIconPlayer = getChildOfType<SimplePlayer>(myColorIcon, 0);
        myColorIconPlayer->setColor(GM->colorForIdx(GM->getPlayerColor()));
        myColorIconPlayer->setSecondColor(GM->colorForIdx(GM->getPlayerColor2()));
        myColorIconPlayer->setGlowOutline(GM->colorForIdx(GM->getPlayerGlowColor()));
        if (!GM->m_playerGlow) myColorIconPlayer->disableGlowOutline();
        
        //conflict animated profiles
        if (Loader::get()->isModLoaded("thesillydoggo.animatedprofiles"))
        {
            if (unlockType == UnlockType::Robot)
            {
                myColorIconPlayer->createRobotSprite(iconId);
                myColorIconPlayer->m_robotSprite->runAnimation("idle01");
            }
            if (unlockType == UnlockType::Spider)
            {
                myColorIconPlayer->createSpiderSprite(iconId);
                myColorIconPlayer->m_spiderSprite->runAnimation("idle01");
            }
        }
        
        CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        myColorIcon->setPosition(CCPoint(screenSize.width / 2, originalIcon->getPositionY()));
        myColorIcon->setScale(1.25f);
        myColorIcon->setZOrder(13);
            
        originalIcon->setScale(1.1f);
        myColorIcon->setScale(1.1f);
            
        m_mainLayer->addChild(myColorIcon);
    }
    
    void addColors()
    {
        CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        CCMenu* originalMenu = getChildOfType<CCMenu>(m_mainLayer, 0);
        
        //creates menus for colors and text
        auto buttonColorMenu = CCMenu::create();
        buttonColorMenu->setID("button-color-menu");
        buttonColorMenu->setPosition(CCPoint(screenSize.width / 2 - 130, originalMenu->getPositionY() - 3));
        buttonColorMenu->setContentSize(CCSize(100, 45));
        buttonColorMenu->setAnchorPoint(CCPoint(0, 0.5f));
        buttonColorMenu->setLayout(RowLayout::create()->setAutoScale(false)->setGap(8));
        
        auto textColorMenu = CCMenu::create();
        textColorMenu->setID("text-color-menu");
        textColorMenu->setPosition(CCPoint(screenSize.width / 2 - 130, originalMenu->getPositionY() + 16));
        textColorMenu->setContentSize(CCSize(100, 45));
        textColorMenu->setAnchorPoint(CCPoint(0, 0.5f));
        textColorMenu->setLayout(RowLayout::create()->setAutoScale(false)->setGap(7));
        handleTouchPriority(this);
        
        //creates color btn with text
        for (int i = 1; i < 4; i++)
        {
            UnlockType unlockType;
            int iconId;
            CCLabelBMFont* colorText;
            std::string id;
            
            switch (i)
            {
            case 1:
                unlockType = UnlockType::Col1;
                iconId = m_fields->profileList.back()->m_score->m_color1;
                colorText = CCLabelBMFont::create("Col1", "bigFont.fnt");
                id = "color1";
            break;
            case 2:
                unlockType = UnlockType::Col2;
                iconId = m_fields->profileList.back()->m_score->m_color2;
                colorText = CCLabelBMFont::create("Col2", "bigFont.fnt");
                id = "color2";
            break;
            case 3:
                unlockType = UnlockType::Col2;
                iconId = m_fields->profileList.back()->m_score->m_color3;
                colorText = CCLabelBMFont::create("Glow", "bigFont.fnt");
                id = "color3";
            break;
            default:
            break;
            }
            
            auto colorButton = CCMenuItemSpriteExtra::create(GJItemIcon::createBrowserItem(unlockType, iconId), this, menu_selector(MyItemInfoPopup::onColorClick));
            if (i == 3 && !m_fields->profileList.back()->m_score->m_glowEnabled) 
                colorButton = CCMenuItemSpriteExtra::create(GJItemIcon::createBrowserItem(UnlockType::ShipFire, 1), this, nullptr);
            colorButton->setUserObject(new BetterUnlockInfo_Params(iconId, unlockType));
            colorButton->m_baseScale = 0.8f;
            colorButton->setScale(0.8f);
            colorButton->setID(id);
            buttonColorMenu->addChild(colorButton);
        
            colorText->setScale(0.3f);
            textColorMenu->addChild(colorText);
        }
        
        buttonColorMenu->updateLayout();
        textColorMenu->updateLayout();
        m_mainLayer->addChild(buttonColorMenu);
        m_mainLayer->addChild(textColorMenu);
    }
    
    void onColorClick(CCObject* sender)
    {
        //shows popup
        for (auto button : CCArrayExt<CCMenuItemSpriteExtra*>(m_mainLayer->getChildByID("button-color-menu")->getChildren()) )
            button->setTag(-1);
        auto parameters = static_cast<BetterUnlockInfo_Params*>(static_cast<CCNode*>(sender)->getUserObject());
        sender->setTag(1);
        ItemInfoPopup::create(parameters->m_IconId, parameters->m_UnlockType)->show();
    }
    
    void adduseMyColorsCheckBox()
    {
        //adds menu
        auto menu = CCMenu::create();
        menu->setID("checkbox-menu");
        menu->setScale(0.35f);
        handleTouchPriority(this);
        
        auto check = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(MyItemInfoPopup::onUseMyColorsToggle), 1);
    
        CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        menu->setPosition(CCPoint(150 + screenSize.width / 2 * 0.35f - 20, 115 + screenSize.height / 2 * 0.35f - (check->getContentSize().height * 0.35) -  6));
        
        //info popup
        auto infoButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(MyItemInfoPopup::onUseMyColors));
        infoButton->m_baseScale = 0.7f;
        infoButton->setScale(0.7f);
        infoButton->setPosition(CCPoint(28, 16));
        
        menu->addChild(check);
        menu->addChild(infoButton);
        m_mainLayer->addChild(menu);
    }
    
    void onUseMyColors(CCObject* sender)
    {
        FLAlertLayer::create("Use my colors", "Shows the icon in colors you are using", "OK")->show();
    }

    void onUseMyColorsToggle(CCObject* sender)
    {
        //switches between icons
        bool show = static_cast<CCMenuItemToggler*>(sender)->isOn();
        m_mainLayer->getChildByID("useMyColorsToggle")->setVisible(!show);
        m_mainLayer->getChildByTag(1)->setVisible(show);
    }
    
    void addDetailButton(int iconId, UnlockType unlockType)
    {
        std::string labelText = textFromArea();

        //doesnt add the button if text contains "unlock" except for secrets
        if (labelText.find("unlock") != std::string::npos && labelText.find("secret is required") == std::string::npos && labelText.find("treasure") == std::string::npos) return;
        
        auto originalMenu = getChildOfType<CCMenu>(m_mainLayer, 0);
        auto infoButton = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png"), this, menu_selector(MyItemInfoPopup::onDetailButtonClick));
        infoButton->setID("infoButton");
        infoButton->m_baseScale = 0.7f;
        infoButton->setScale(0.7f);
        infoButton->setPosition(CCPoint(118, 102));
        infoButton->setUserObject(new BetterUnlockInfo_Params(iconId, unlockType));
        originalMenu->addChild(infoButton);
    }
    
    void onDetailButtonClick(CCObject* sender)
    {
        std::string labelText = textFromArea();

        auto parameters = static_cast<BetterUnlockInfo_Params*>(static_cast<CCNode*>(sender)->getUserObject());
        
        //creates a dummy node for info for "refreshing" garage layer
        if (labelText.find("buy") != std::string::npos) 
        {
            std::ifstream file(Mod::get()->getResourcesDir() / "shops.json");
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto object = matjson::parse(buffer.str());
            
            //if owns
            if (GameStatsManager::sharedState()->isItemUnlocked(parameters->m_UnlockType, parameters->m_IconId))
            {
                FLAlertLayer::create("Owned!", "You already own this item", "OK")->show();
                return;
            }
            
            int shoptype;
            int price;
            bool notExists = true;
            
            //find clicked item in json
            for (int i = 0; i < object.as_array().size(); i++)
                if (object[i]["UnlockType"].as_int() == static_cast<int>(parameters->m_UnlockType))
                    for (int j = 0; j < object[i]["items"].as_array().size(); j++)
                        if (object[i]["items"][j]["IconId"].as_int() == parameters->m_IconId)
                        {
                            auto item = object[i]["items"][j];
                            shoptype = item["ShopType"].as_int();
                            price = item["Price"].as_int();
                            notExists = false;
                            
                            //if can't afford
                            if (shoptype == 4)
                            {
                                if (item["Price"] > GameStatsManager::sharedState()->getStat("29"))
                                {
                                    FLAlertLayer::create("Too expensive!", "You can't afford this item", "OK")->show();
                                    return;
                                }
                            }
                            if (item["Price"] > GameStatsManager::sharedState()->getStat("14"))
                            {
                                FLAlertLayer::create("Too expensive!", "You can't afford this item", "OK")->show();
                                return;
                            }
                            
                            //the dummy node
                            GJGarageLayer* garage = getChildOfType<GJGarageLayer>(CCScene::get(), 0);
                            if (garage != nullptr)
                            {
                                CCNode* dummy = CCNode::create();
                                dummy->setID("dummyInfoNode");
                                dummy->setUserObject(new BetterUnlockInfo_Params(
                                    item["IconId"].as_int(), 
                                    static_cast<UnlockType>(item["UnlockType"].as_int()), 
                                    item["Price"].as_int(),
                                    item["ShopType"].as_int()
                                ));
                                garage->addChild(dummy);
                            }
                            
                            //buy popup
                            PurchaseItemPopup::create(
                                GJStoreItem::create(
                                    item["ShopItemId"].as_int(),
                                    item["IconId"].as_int(),
                                    item["UnlockType"].as_int(),
                                    item["Price"].as_int(),
                                    static_cast<ShopType>(item["ShopType"].as_int())
                                )
                            )->show();
                            break;
                        }
            
            if (notExists)
            {
                //note if doesnt exist
                FLAlertLayer::create("Oh no!", "the item you clicked isn't in json, if you see this report it to @rynat on discord", "OK")->show();
                return;
            }
            
            //shows how many orbs you have
            auto buypopup = getChildOfType<PurchaseItemPopup>(CCScene::get(), 0);
            
            CCSprite* currentOrbsSpr = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
            currentOrbsSpr->setID("CurrentSpr");
            
            CCSprite* afterOrbsSpr = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
            afterOrbsSpr->setID("AfterSpr");
            
            int currentOrbs = GameStatsManager::sharedState()->getStat("14");
            
            if (shoptype == 4) {
                currentOrbsSpr = CCSprite::createWithSpriteFrameName("currencyDiamondIcon_001.png");
                afterOrbsSpr = CCSprite::createWithSpriteFrameName("currencyDiamondIcon_001.png");
                currentOrbs = GameStatsManager::sharedState()->getStat("29");
            }
            CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
            currentOrbsSpr->setPosition(CCPoint(screenSize.width - 20, screenSize.height - 15.5f));
            afterOrbsSpr->setPosition(CCPoint(screenSize.width - 20, screenSize.height - 15.5f - 40));
            
            CCLabelBMFont* currentCount = CCLabelBMFont::create(std::to_string(currentOrbs).c_str(), "bigFont.fnt");
            currentCount->setID("currentCount");
            currentCount->setScale(0.6f);
            currentCount->setAnchorPoint(CCPoint(1, 0.5f));
            currentCount->setPosition(CCPoint(screenSize.width - 35, screenSize.height - 15));
            
            std::string minus = "-";
            minus.append(std::to_string(price));
            CCLabelBMFont* subCount = CCLabelBMFont::create(minus.c_str(), "bigFont.fnt");
            subCount->setID("subCount");
            subCount->setScale(0.6f);
            subCount->setAnchorPoint(CCPoint(1, 0.5f));
            subCount->setPosition(CCPoint(screenSize.width - 35, screenSize.height - 15 - 20));
            subCount->setColor(ccColor3B(0xFF, 0x37, 0x37));
            
            CCLabelBMFont* afterCount = CCLabelBMFont::create(std::to_string(currentOrbs-price).c_str(), "bigFont.fnt");
            afterCount->setID("afterCount");
            afterCount->setScale(0.6f);
            afterCount->setAnchorPoint(CCPoint(1, 0.5f));
            afterCount->setPosition(CCPoint(screenSize.width - 35, screenSize.height - 15 - 40));
            
            
            buypopup->addChild(currentOrbsSpr);
            buypopup->addChild(afterOrbsSpr);
            buypopup->addChild(currentCount);
            buypopup->addChild(subCount);
            buypopup->addChild(afterCount);
        }

        if (labelText.find("secret chest") != std::string::npos)
        {
            std::ifstream file(Mod::get()->getResourcesDir() / "chests.json");
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto object = matjson::parse(buffer.str());
            
            //find clicked item in json
            for (int i = 0; i < object.as_array().size(); i++)
                if (object[i]["UnlockType"].as_int() == static_cast<int>(parameters->m_UnlockType))
                    for (int j = 0; j < object[i]["chests"].as_array().size(); j++)
                    {
                        if (object[i]["chests"][j]["IconId"].as_int() == parameters->m_IconId)
                        {
                            int chestType = object[i]["chests"][j]["GJRewardType"].as_int();
                            std::string desc("You can find this item in a ");
                            desc.append(getSecretChestDesc(chestType));
                            FLAlertLayer::create(nullptr, "Which one? This one!", desc, "OK", nullptr, 300)->show();
                            return;
                        }  
                    }
                        
                        
        }

        if (labelText.find("special chest") != std::string::npos)
        {
            std::ifstream file(Mod::get()->getResourcesDir() / "special.json");
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto object = matjson::parse(buffer.str());
            
            //find clicked item in json
            for (int i = 0; i < object.as_array().size(); i++)
                if (object[i]["UnlockType"].as_int() == static_cast<int>(parameters->m_UnlockType))
                    for (int j = 0; j < object[i]["chests"].as_array().size(); j++)
                        if (object[i]["chests"][j]["IconId"].as_int() == parameters->m_IconId)
                        {
                            //ChestId - removed "" and leading 0
                            int chestType = object[i]["chests"][j]["ChestId"].as_int();
                            int size = 300;
                            if (chestType >= 4 && chestType <= 6 || chestType >= 12 && chestType <= 21) //12-21 temp
                                size = 350;
                            std::string desc("You can unlock this item ");
                            desc.append(getSpecialChestDesc(chestType));
                            FLAlertLayer::create(nullptr, "Which one? This one!", desc, "OK", nullptr, size)->show();
                            return;
                        }
            
        }

        //note, coins from vaults
        if (labelText.find("secret is required") != std::string::npos || labelText.find("treasure") != std::string::npos)
        {
            std::ifstream file(Mod::get()->getResourcesDir() / "secrets.json");
            std::stringstream buffer;
            buffer << file.rdbuf();
            auto object = matjson::parse(buffer.str());
            
            //find clicked item in json
            for (int i = 0; i < object.as_array().size(); i++)
                if (object[i]["UnlockType"].as_int() == static_cast<int>(parameters->m_UnlockType))
                    for (int j = 0; j < object[i]["items"].as_array().size(); j++)
                        if (object[i]["items"][j]["IconId"].as_int() == parameters->m_IconId)
                        {
                            auto icon = object[i]["items"][j];
                            std::string desc("You can unlock this item ");
                            
                            //0 = destry icons
                            //-1 = master detective
                            //-2 = destroy specific icons
                            //1 = vault
                            //2 = vault of secrets
                            //3 = chamber of time
                            switch (icon["Vault"].as_int())
                            {
                            case 0:
                            desc.append("at the <cj>Main Menu</c> by <cr>Destroying</c> the moving <cg>Icons</c>");
                            break;
                            
                            case -1:
                            desc.append("at the <cj>Main Levels Selection Screen</c> by <cy>Scrolling</c> past all the levels 3 times and <cy>Clicking</c> the <cy>Secret Coin</c> at the <cg>Comming Soon!</c> page");
                            break;
                            
                            case -2:
                            desc.append("at the <cj>Main Menu</c> by <cr>Destroying</c> <cy>This Icon</c> when it'll be moving");
                            break;
                            
                            case 1:
                            desc.append("at <cj>The Vault</c> by entering the code <cg>");
                            if (parameters->m_IconId == 64) desc.append(GameManager::sharedState()->m_playerName);
                            else desc.append(icon["Code"].as_string());
                            desc.append("</c>");
                            break;
                            
                            case 2:
                            desc.append("at the <cj>Vault of Secrets</c> by entering the code <cg>");
                            if (parameters->m_IconId == 76) desc.append(std::to_string(GameStatsManager::sharedState()->getStat("6")));
                            else if (parameters->m_IconId == 78) //uberhacker
                            {
                                int code = GameManager::sharedState()->m_secretNumber.value()*-1;
                                if (code == 0) 
                                {
                                    desc.append(icon["Code"].as_string());
                                    desc.append("</c>");
                                    desc.append(" and then checking this pop up again for the answer");
                                    break;
                                }
                                desc.append(std::to_string(code));
                            }
                            else desc.append(icon["Code"].as_string());
                            desc.append("</c>");
                            break;
                            
                            case 3:
                            desc.append("at the <co>Chamber of Time</c> by entering the code <cg>");
                            desc.append(icon["Code"].as_string());
                            desc.append("</c>");
                            break;
                            
                            default:
                            // "sparky" in vault 1 for coin
                            // "glubfub" in vault 2 for coin (sparky must be obtained)
                            break;
                            }
                            
                            FLAlertLayer::create(nullptr, "No longer a secret :)", desc, "OK", nullptr, 350)->show();
                            return;
                        }
        }
                
    }
    
    std::string textFromArea()
    {
        TextArea* textArea = getChildOfType<TextArea>(m_mainLayer, 0);
        
        //conflict MH
        if (textArea == nullptr) return std::string("unlock 2.21");
        
        std::string labelText = "";
        for (CCLabelBMFont* label : CCArrayExt<CCLabelBMFont*>(textArea->m_label->getChildren()) )
            labelText.append(label->getString());
            
        return labelText;
    }

    const char* getSecretChestDesc(int iGJRewardType)
    {
        switch (iGJRewardType)
        {
        case 1: return "<cy>4 hour? Chest</c>"; break;
        case 2: return "<cy>24 hour? Chest</c>"; break;
        case 3: return "<cy>1 Key Chest</c>"; break;
        case 4: return "<cy>5 Key Chest</c>"; break;
        case 5: return "<cy>10 Key Chest</c>"; break;
        case 6: return "<cy>25 Key Chest</c>"; break;
        case 7: return "<cy>50 Key Chest</c>"; break;
        case 8: return "<cy>100 Key Chest</c>"; break;
        default: return "null"; break;
        }
    }

    const char* getSpecialChestDesc(int iChestId)
    {
        //note, ad chest only on mobile idk if save load 
        switch (iChestId)
        {
        case 1: return "by completing <cr>The Challenge</c>"; break;
        case 2: return "by releasing the <cg>Demon Guardian</c>"; break;
        case 3: return "at the <co>Chamber of Time</c>"; break;
        case 4: return "by opening <cy>50 Chests</c>"; break;
        case 5: return "by opening <cy>100 Chests</c>"; break;
        case 6: return "by opening <cy>200 Chests</c>"; break;
        case 7: return "by opening the free <cp>YouTube Chest</c>"; break;
        case 8: return "by opening the free <cp>Twitter Chest</c>"; break;
        case 9: return "by opening the free <cp>Facebook Chest</c>"; break;
        case 10: return "by opening the free <cp>Twitch Chest</c>"; break;
        case 11: return "by opening the free <cp>Discord Chest</c>"; break;
        case 24: return "by opening the free <cp>Reddit Chest</c>"; break;
        case 12: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 13: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 14: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 15: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 16: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 17: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 18: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 19: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 20: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 21: return "by opening an free <cy>Ad Chest</c> (currently unobtainable on pc)"; break;
        case 22: return "by repeatedly clicking <cj>The Shopkeeper</c>"; break;
        case 23: return "by repeatedly opening and closing the <cb>Help Page</c> (you must be logged into an account)"; break;
        default: return "null"; break;
        }
    }

    void addCompletionProgress(int iconId, UnlockType unlockType) 
    {
        if (textFromArea().find("2.21") != std::string::npos) return; //note, 2.21
        
        CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        
        CCMenu* menu = CCMenu::create();
        menu->setID("completionMenu");
        
        Slider* slider = Slider::create(this, nullptr);
        slider->setVisible(false);
        CCSprite* sliderSprite = getChildOfType<CCSprite>(slider, 0);
        sliderSprite->setID("completionSlider");
        sliderSprite->setAnchorPoint(CCPoint(0, 0.5f));
        sliderSprite->setScale(0.45f);
        
        /*
        auto bar = getChildOfType<CCSprite>(getChildOfType<CCSprite>(slider, 0), 0);
        auto texture = CCSprite::create("sliderBar2.png")->getTexture();
        texture->setTexParameters(new ccTexParams{GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT});
        bar->setTexture(texture);*/

        CCSprite* icon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
        icon->setPosition(CCPoint(129, -84));
        
        if (trueIsItemUnlocked(iconId, unlockType))
        {
            icon = CCSprite::createWithSpriteFrameName("GJ_achImage_001.png");
            icon->setPosition(CCPoint(130, -85));
        }
        icon->setID("completionIcon");
        icon->setScale(0.45f);
        
        
        //progress bar
        int currentValue = 0;
        int maxValue = 1;
        
        std::string achiLong = "";
        std::string achiShort = "";
        
        std::ifstream achifile(Mod::get()->getResourcesDir() / "achievements.json");
        std::stringstream achibuffer;
        achibuffer << achifile.rdbuf();
        auto achi = matjson::parse(achibuffer.str());
        
        for (int i = 0; i < achi.as_array().size(); i++)
            if (achi[i]["UnlockType"].as_int() == static_cast<int>(unlockType))
                for (int j = 0; j < achi[i]["items"].as_array().size(); j++)
                {
                    if (achi[i]["items"][j]["IconId"].as_int() == iconId)
                    {
                        maxValue = achi[i]["items"][j]["MaxValue"].as_int();
                        achiLong = achi[i]["items"][j]["AchievementIdFull"].as_string();
                        achiShort = achi[i]["items"][j]["AchievementIdShort"].as_string();
                        break;
                    }  
                }
                
        
        std::ifstream achitypefile(Mod::get()->getResourcesDir() / "achievementsTypes.json");
        std::stringstream achitypebuffer;
        achitypebuffer << achitypefile.rdbuf();
        auto achitypes = matjson::parse(achitypebuffer.str());
        
        int currentValueFrom = 0;
        std::string sprite = "";
        float scale = 0.5f;
        float moveY = 0;
        
        auto achitypeObject = achitypes[achiShort].as_object();
        if (achitypeObject.size() > 0)
        {
            currentValueFrom = achitypeObject["CurrentValueFrom"].as_int();
            sprite = achitypeObject["Sprite"].as_string();
            scale = achitypeObject["Scale"].as_double(); 
            moveY = achitypeObject["MoveY"].as_double();
        }
            
        GameStatsManager* GSM = GameStatsManager::sharedState();
        GameLevelManager* GLM = GameLevelManager::sharedState();
        
        //legends table is private lol
        switch (currentValueFrom)
        {
        case -1 : currentValue = -1; break;
        case -2 : currentValue = trueIsItemUnlocked(iconId, unlockType); break;
        case -3 : currentValue = GLM->m_followedCreators->count(); break;
        case -4 : 
            switch (iconId)
            {
            case 59:
            currentValue = GSM->getCollectedCoinsForLevel(static_cast<GJGameLevel*>(GLM->m_mainLevels->objectForKey("20")));
            break;
            
            case 60:
            currentValue = GSM->getCollectedCoinsForLevel(static_cast<GJGameLevel*>(GLM->m_mainLevels->objectForKey("18")));
            break;
            
            case 14:
            currentValue = GSM->getCollectedCoinsForLevel(static_cast<GJGameLevel*>(GLM->m_mainLevels->objectForKey("14")));
            break;
            
            default: currentValue = -1; break;
            }
            break;
        case -5 : 
            if (achiLong.back() == 'a')
            {
                currentValue = static_cast<GJGameLevel*>(GLM->m_mainLevels->objectForKey(std::to_string(std::atoi(achiLong.substr(18, 2).c_str()))))->m_practicePercent / 100;
                sprite = "checkpoint_01_001.png";
            }
            else
                currentValue = GSM->hasCompletedMainLevel(std::atoi(achiLong.substr(18, 2).c_str()));
            break;
        case -6 :
            if (maxValue > 1) currentValue = GSM->getStat("9");
            else currentValue = trueIsItemUnlocked(iconId, unlockType);
            break;
        case -7 :
        {
            int shards[5] = {GSM->getStat("16"), GSM->getStat("17"), GSM->getStat("18"), GSM->getStat("19"), GSM->getStat("20")};
            int smallest = INT_MAX;
            for (int i = 0; i < 4; i++)
                if (smallest > shards[i])
                    smallest = shards[i];
            currentValue = smallest;
            break;
        }
        case -8 : 
        {   
            int shards[5] = {GSM->getStat("23"), GSM->getStat("24"), GSM->getStat("25"), GSM->getStat("26"), GSM->getStat("27")};
            int smallest = INT_MAX;
            for (int i = 0; i < 4; i++)
                if (smallest > shards[i])
                    smallest = shards[i];
            currentValue = smallest;
            break;
        }
        case -9 : 
            if (achiLong.back() == '1')
                for (int i = 1; i <= 3; i++)
                    currentValue += GSM->hasCompletedMainLevel(i);
            else currentValue = GSM->hasCompletedMainLevel(14);
            break;
        case -10 : currentValue = GSM->hasCompletedMainLevel(achiLong.at(19) - '0' + 5000); break;
        case -11 : currentValue = GSM->getCollectedCoinsForLevel(static_cast<GJGameLevel*>(GLM->m_mainLevels->objectForKey(std::string("50").append(achiLong.substr(18, 2))))); break;
        case -12 :
        {
            FriendsProfilePage* friends = FriendsProfilePage::create(UserListType::Friends);
            for (auto node : CCArrayExt<CCNode*>(friends->m_mainLayer->getChildren()))
                if (typeinfo_cast<CCLabelBMFont*>(node) != nullptr)
                {
                    std::string nodetext = static_cast<CCLabelBMFont*>(node)->getString();
                    std::string number = nodetext.substr(nodetext.find(":") + 2, nodetext.length() - 1 - nodetext.find(":") + 2);
                    int num = std::atoi(number.c_str());
                    if (num > 0) currentValue = num;
                }
            friends->release(); //idk if works, everytime you click this it still adds 2mb to memory and doesnt go down really
            break;
        }
        default: 
            if (currentValueFrom >= 30 && currentValueFrom <= 39) 
                currentValue = GSM->getStat(std::to_string(currentValueFrom).c_str()) / 100; //paths
            else if (currentValueFrom >= 1)
                currentValue = GSM->getStat(std::to_string(currentValueFrom).c_str());
            else
            {
                currentValue = trueIsItemUnlocked(iconId, unlockType);
                std::string labelText = textFromArea();
                
                if(labelText.find("buy") != std::string::npos)
                {
                    sprite = "currencyOrbIcon_001.png";
                    scale = 0.7f;
                }
                else if (labelText.find("secret chest") != std::string::npos)
                {
                    sprite = "chest_03_02_001.png";
                    scale = 0.15f;
                }
                else if (labelText.find("special chest") != std::string::npos)
                {
                    sprite = "chest_02_02_001.png";
                    scale = 0.15f;
                }
                else
                {
                    
                    std::ifstream specialfile(Mod::get()->getResourcesDir() / "special.json");
                    std::stringstream specialbuffer;
                    specialbuffer << specialfile.rdbuf();
                    auto special = matjson::parse(specialbuffer.str());
                    
                    std::string chestId = "";
                    
                    for (int i = 0; i < special.as_array().size(); i++)
                        if (special[i]["UnlockType"].as_int() == static_cast<int>(unlockType))
                            for (int j = 0; j < special[i]["chests"].as_array().size(); j++)
                                if (special[i]["chests"][j]["IconId"].as_int() == iconId)
                                    chestId = special[i]["chests"][j]["ChestId"].as_string();
                    
                    if (labelText.find("gauntlet") != std::string::npos)
                    {
                        CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("GauntletSheet.plist", "GauntletSheet.png");
                        std::string number = chestId.substr(chestId.find("_") + 1, chestId.length() - 1 - chestId.find("_") + 1);
                        int num = std::atoi(number.c_str());
                        
                        switch (num)
                        {
                        case 1: sprite = "island_fire_001.png"; break;
                        case 2: sprite = "island_ice_001.png"; break;
                        case 3: sprite = "island_poison_001.png"; break;
                        case 4: sprite = "island_shadow_001.png"; break;
                        case 5: sprite = "island_lava_001.png"; break;
                        case 6: sprite = "island_bonus_001.png"; break;
                        case 7: sprite = "island_chaos_001.png"; break;
                        case 8: sprite = "island_demon_001.png"; break;
                        case 9: sprite = "island_time_001.png"; break;
                        case 10: sprite = "island_crystal_001.png"; break;
                        case 11: sprite = "island_magic_001.png"; break;
                        case 12: sprite = "island_spike_001.png"; break;
                        case 13: sprite = "island_monster_001.png"; break;
                        case 14: sprite = "island_doom_001.png"; break;
                        case 15: sprite = "island_death_001.png"; break;
                        default:
                            std::string numtext = "";
                            if (num - 15 < 10) numtext = "0";
                            numtext.append(std::to_string(num-15));
                            sprite = std::string("island_new").append(numtext).append("_001.png");
                            break;
                        }
                        scale = 0.2f;
                        moveY = -3;
                    }
                    if (labelText.find("Complete the Path") != std::string::npos)
                    {
                        std::string number = chestId.substr(chestId.find("_") + 1, chestId.length() - 1 - chestId.find("_") + 1);
                        int num = std::atoi(number.c_str());
                        
                        std::string numtext = "";
                        if (num != 10) numtext = "0";
                        numtext.append(number);
                        sprite = std::string("pathIcon_").append(numtext).append("_001.png");
                        
                        scale = 0.35f;
                    }
                }
            }
            break;
        }
        
        
        if (currentValue == -1) return; //if like stuff is sent from servers
        
        slider->setValue(static_cast<float>(currentValue) / static_cast<float>(maxValue));
        sliderSprite->setPosition(CCPoint(32, -85));
        //bar->setColor(ccColor3B({255, 0, 0})); //color
        

        //create labels
        CCLabelBMFont* labelCount = CCLabelBMFont::create(std::to_string(currentValue).c_str(), "bigFont.fnt");
        CCLabelBMFont* labelSlash = CCLabelBMFont::create("/", "bigFont.fnt");
        CCLabelBMFont* labelMax = CCLabelBMFont::create(std::to_string(maxValue).c_str(), "bigFont.fnt");
        
        if (currentValue >= 1000)
        {
            float newCurrent = std::round((static_cast<float>(currentValue) / 1000) * 10) / 10;
            
            labelCount = CCLabelBMFont::create(fmt::format("{}", newCurrent).append("K").c_str(), "bigFont.fnt");
            if (currentValue >= 1000000)
            {
                newCurrent = std::round((static_cast<float>(currentValue) / 1000000) * 10) / 10;
                labelCount = CCLabelBMFont::create(fmt::format("{}", newCurrent).append("M").c_str(), "bigFont.fnt");
            }
        }
        if (maxValue >= 1000)
        {
            float newMax = std::round((static_cast<float>(maxValue) / 1000) * 10) / 10;
            labelMax = CCLabelBMFont::create(fmt::format("{}", newMax).append("K").c_str(), "bigFont.fnt");
            if (maxValue >= 1000000)
            {
                newMax = std::round((static_cast<float>(maxValue) / 1000000) * 10) / 10;
                labelMax = CCLabelBMFont::create(fmt::format("{}", newMax).append("M").c_str(), "bigFont.fnt");
            }
        }
            
        labelCount->setID("completionlabelCount");
        labelSlash->setID("completionlabelSlash");
        labelMax->setID("completionlabelMax");
        
        labelCount->setScale(0.35f);
        labelSlash->setScale(0.35f);
        labelMax->setScale(0.35f);
        
        labelCount->setPosition(CCPoint(77, -97));
        labelSlash->setPosition(CCPoint(80, -97));
        labelMax->setPosition(CCPoint(83, -97));
        
        labelCount->setAnchorPoint(CCPoint(1, 0.5f));
        labelMax->setAnchorPoint(CCPoint(0, 0.5f));
        
        if (sprite != "")
        {
            if (achiShort == "gauntlets")
                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("GauntletSheet.plist", "GauntletSheet.png");
                
            CCSprite* groupIcon = CCSprite::createWithSpriteFrameName(sprite.c_str());
            if (sprite[0] == '_') groupIcon = CCSprite::createWithSpriteFrameName(fmt::format("{}/{}", Mod::get()->getID(), sprite).c_str());
            if (sprite == "GJ_downloadsIcon_001.png") groupIcon->setRotation(180);
            groupIcon->setID("completionGroupIcon");
            groupIcon->setPosition(CCPoint(labelMax->getPositionX() + labelMax->getContentSize().width * 0.35f + groupIcon->getContentSize().width/(2.f/scale) + 1, -97.5f + moveY));
            groupIcon->setScale(scale);
            menu->addChild(groupIcon);
        }
        
        menu->addChild(sliderSprite);
        menu->addChild(icon);
        menu->addChild(labelCount);
        menu->addChild(labelSlash);
        menu->addChild(labelMax);
        m_mainLayer->addChild(menu);
        
        CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile("GauntletSheet.plist");
    }
    
    void addCompletionIconOnly(int iconId, UnlockType unlockType)
    {
        if (textFromArea().find("2.21") != std::string::npos) return; //note, 2.21
        
        CCMenu* originalMenu = getChildOfType<CCMenu>(m_mainLayer, 0);
        CCSprite* icon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
        icon->setID("completionIcon");
        icon->setScale(0.6f);
        
        if (trueIsItemUnlocked(iconId, unlockType))
        {
            icon = CCSprite::createWithSpriteFrameName("GJ_achImage_001.png");
            icon->setScale(0.55f);
        }
        
        originalMenu->addChild(icon);
        icon->setPosition(CCPoint(118, 35));
    }
    
    IconType UnlockToIcon(UnlockType unlockType)
    {
        
        switch (unlockType)
        {
        case UnlockType::Cube: return IconType::Cube;
        case UnlockType::Ship: return IconType::Ship;
        case UnlockType::Ball: return IconType::Ball;
        case UnlockType::Bird: return IconType::Ufo;
        case UnlockType::Dart: return IconType::Wave;
        case UnlockType::Robot: return IconType::Robot;
        case UnlockType::Spider: return IconType::Spider;
        case UnlockType::Swing: return IconType::Swing;
        case UnlockType::Jetpack: return IconType::Jetpack;
        case UnlockType::Death: return IconType::DeathEffect;
        case UnlockType::Streak: return IconType::Special;
        default:break;
        }
        
        return IconType::Cube;
    }
    
    void addEquipButton(int iconId, UnlockType unlockType)
    {
        if (!trueIsItemUnlocked(iconId, unlockType)) return;
        
        CCMenu* originalMenu = getChildOfType<CCMenu>(m_mainLayer, 0);
        auto spr = CircleButtonSprite::create(CCLabelBMFont::create("Use", "bigFont.fnt"));
        auto equipButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyItemInfoPopup::onEquipButtonClick));
        equipButton->setID("equipButton");
        equipButton->m_baseScale = 0.6f;
        equipButton->setScale(0.6f);
        
        // the popup is always 300x230, X = half of sprite + 6 for space, Y = calc center of screen from menu + same as X
        CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        equipButton->setPosition(CCPoint( 
            -150 + equipButton->getContentSize().width * 0.6 / 2 + 6,
            screenSize.height / 2 - originalMenu->getPositionY() + 115 - equipButton->getContentSize().height * 0.6 / 2 - 6
        ));
        
        equipButton->setUserObject(new BetterUnlockInfo_Params(iconId, unlockType));
        originalMenu->addChild(equipButton);
    }
    
    void onEquipButtonClick(CCObject* sender) 
    {
        auto parameters = static_cast<BetterUnlockInfo_Params*>(static_cast<CCNode*>(sender)->getUserObject());     
        auto GM = GameManager::sharedState();      
        
        switch (static_cast<int>(parameters->m_UnlockType))
        {
        case 1: GM->setPlayerFrame(parameters->m_IconId); break;
        case 2: GM->setPlayerColor(parameters->m_IconId); break;
        case 3:
        {   
            auto button = getChildOfType<ItemInfoPopup>(CCScene::get(), 0)->m_mainLayer->getChildByID("button-color-menu")->getChildByTag(1);
            if (button->getID() == "color3")
                GM->setPlayerColor3(parameters->m_IconId);
            else
                GM->setPlayerColor2(parameters->m_IconId); 
        }
        break;
        case 4: GM->setPlayerShip(parameters->m_IconId); break;
        case 5: GM->setPlayerBall(parameters->m_IconId); break;
        case 6: GM->setPlayerBird(parameters->m_IconId); break;
        case 7: GM->setPlayerDart(parameters->m_IconId); break;
        case 8: GM->setPlayerRobot(parameters->m_IconId); break;
        case 9: GM->setPlayerSpider(parameters->m_IconId); break;
        case 10: GM->setPlayerStreak(parameters->m_IconId); break;
        case 11: GM->setPlayerDeathEffect(parameters->m_IconId); break;
        case 13: GM->setPlayerSwing(parameters->m_IconId); break;
        case 14: GM->setPlayerJetpack(parameters->m_IconId); break;
        case 15: GM->setPlayerShipStreak(parameters->m_IconId); break;
        default: break;
        }
        
        
        if (!Mod::get()->setSavedValue("shown-equip-restart-popup", true))
            FLAlertLayer::create("Note", "By equiping icon this way it won't update on your profile until you restart your game", "OK")->show();
        
        if (getChildOfType<geode::Notification>(CCScene::get(), 0) == nullptr)
            geode::Notification::create("Equipped", CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
    }
    
    bool trueIsItemUnlocked(int iconId, UnlockType unlockType)
    {
        if (unlockType == UnlockType::Col1 || unlockType == UnlockType::Col2)
            return GameManager::get()->isColorUnlocked(iconId, unlockType);
      
        return GameManager::get()->isIconUnlocked(iconId, UnlockToIcon(unlockType));
    }
};
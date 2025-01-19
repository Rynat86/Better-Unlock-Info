#include "Includes.hpp"
#include <Geode/modify/ItemInfoPopup.hpp>
#include <matjson.hpp>


//replaces grayscale icon with users, adds colors and rest aka lazy to write - in about.md
class $modify(MyItemInfoPopup, ItemInfoPopup)
{
    struct Fields
    {
        std::vector<ProfilePage*> profileList;
    };
    
    
    static void onModify(auto& self)
    {
        (void) self.setHookPriority("ItemInfoPopup::init", -1000);
    }
    
    
    bool init(int IconId, UnlockType UnlockType) 
    {
        if (!ItemInfoPopup::init(IconId, UnlockType)) return false;
        
        //gj is broken when toggle no idea why, error
        if (UnlockType == UnlockType::GJItem) return true; //note, icon type for isIconUnlocked()
        
        moveCredit();
        addDetailButton(IconId, UnlockType);
        
        if (!(Mod::get()->getSettingValue<bool>("progressToggle")))
            addCompletionProgress(IconId, UnlockType);
        else
            addCompletionIconOnly(IconId, UnlockType);
        
        
        
        //if this is unlock, dont add colors related stuff
        int badUnlocks[] = {2, 3, 10, 11, 12, 15};
        bool isBad = std::find(std::begin(badUnlocks), std::end(badUnlocks), as<int>(UnlockType)) != std::end(badUnlocks);
        if (isBad)
        {
            //is bad but add equip on profile
            if (CCScene::get()->getChildByID("ProfilePage") != nullptr)
                if (!(Mod::get()->getSettingValue<bool>("equipToggle")))
                    addEquipButton(IconId, UnlockType);
            return true;
        }
        
        
        
        //add color swap
        if (!(Mod::get()->getSettingValue<bool>("useMyColorsToggle")))
            addUseMyColorsCheckBox();
        
        
        
        //profile check
        if (CCScene::get()->getChildByID("ProfilePage") == nullptr) return true;
        
        //finds newest profile (prob could be better ngl)
        m_fields->profileList.clear();
        for (auto node : CCArrayExt<CCNode*>(CCScene::get()->getChildren()))
            if (typeinfo_cast<ProfilePage*>(node) != nullptr)
                m_fields->profileList.push_back(as<ProfilePage*>(node));

        updateIconColorsOnProfile();
        addColors();
        
        if (!(Mod::get()->getSettingValue<bool>("equipToggle")))
            addEquipButton(IconId, UnlockType);
        
        //fix android touch - thx devtools :yep:
        if (auto delegate = typeinfo_cast<CCTouchDelegate*>(m_fields->profileList.back()))
            if (auto handler = CCTouchDispatcher::get()->findHandler(delegate))
                if (auto delegate2 = typeinfo_cast<CCTouchDelegate*>(as<CCMenu*>(m_fields->profileList.back()->m_mainLayer->getChildByID("player-menu"))))
                    if (auto handler2 = CCTouchDispatcher::get()->findHandler(delegate2))
                        CCTouchDispatcher::get()->setPriority(handler->getPriority()-1, handler2->getDelegate());
        
        return true;
    }
    
    //user colors checkbox
    void addUseMyColorsCheckBox()
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
        
        if (Mod::get()->getSettingValue<bool>("garageColorsToggle") && CCScene::get()->getChildByType<GJGarageLayer>(0) != nullptr)
            check->toggleWithCallback(true);   
    }
    
    void updateIconColorsOnProfile()
    {
        auto checkboxMenu = m_mainLayer->getChildByID("checkbox-menu");
        if (checkboxMenu != nullptr)
        {
            auto checkbox = checkboxMenu->getChildByType<CCMenuItemToggler>(0);
            checkbox->setUserObject(CCNode::create());
            checkbox->toggle(true);
            checkbox->toggleWithCallback(false);
            return;
        }
        
        auto GM = GameManager::get();
        auto icon = m_mainLayer->getChildByID("item-icon")->getChildByType<SimplePlayer>(0);
        auto profile = m_fields->profileList.back();
        
        icon->setColor(GM->colorForIdx(profile->m_score->m_color1));
        icon->setSecondColor(GM->colorForIdx(profile->m_score->m_color2));
        if (profile->m_score->m_glowEnabled) icon->setGlowOutline(GM->colorForIdx(profile->m_score->m_color3));
    }
    
    void onUseMyColors(CCObject* sender)
    {
        FLAlertLayer::create("Use my colors", "Shows the icon in colors you are using", "OK")->show();
    }

    void onUseMyColorsToggle(CCObject* sender)
    {
        //switches between icons
        SimplePlayer* icon = nullptr;
        
        //animated profiles fix
        if (Loader::get()->isModLoaded("thesillydoggo.animatedprofiles"))
            for (auto node : CCArrayExt<CCNode*>(m_mainLayer->getChildren()))
                if (auto menu = typeinfo_cast<CCMenu*>(node))
                    if (menu->getID() == "")
                        for (auto menuNode : CCArrayExt<CCNode*>(menu->getChildren()))
                            if (auto button = typeinfo_cast<CCMenuItemSpriteExtra*>(menuNode))
                                if (auto gjicon = button->getChildByType<GJItemIcon>(0))
                                    icon = gjicon->getChildByType<SimplePlayer>(0);
        
        if (icon == nullptr)
            icon = m_mainLayer->getChildByID("item-icon")->getChildByType<SimplePlayer>(0);
        
        icon->setColor(ccColor3B(175,175,175));
        icon->setSecondColor(ccColor3B(255,255,255));
        icon->disableGlowOutline();
        
        
        auto GM = GameManager::get();
        auto checkbox = as<CCMenuItemToggler*>(sender);
        
        //profile check
        if (checkbox->getUserObject() != nullptr)
        {
            auto profile = m_fields->profileList.back();
            
            icon->setColor(GM->colorForIdx(profile->m_score->m_color1));
            icon->setSecondColor(GM->colorForIdx(profile->m_score->m_color2));
            if (profile->m_score->m_glowEnabled) icon->setGlowOutline(GM->colorForIdx(profile->m_score->m_color3));
        }
        
        if (!checkbox->isOn())
        {
            icon->setColor(GM->colorForIdx(GM->getPlayerColor()));
            icon->setSecondColor(GM->colorForIdx(GM->getPlayerColor2()));
            if (GM->m_playerGlow) icon->setGlowOutline(GM->colorForIdx(GM->getPlayerGlowColor()));    
        }
    }
    
    //profile persons colors
    void addColors()
    {
        CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        CCMenu* originalMenu = as<CCMenu*>(m_mainLayer->getChildByID("button-menu"));
        
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
        auto parameters = as<BetterUnlockInfo_Params*>(as<CCNode*>(sender)->getUserObject());
        
        //for equiping cuz glow uses col2
        sender->setTag(1);
        ItemInfoPopup::create(parameters->m_IconId, parameters->m_UnlockType)->show();
    }
    
    //detail button
    void addDetailButton(int iconId, UnlockType unlockType)
    {
        std::string labelText = textFromArea();

        //doesnt add the button if text contains "unlock" except for secrets
        if (labelText.find("unlock") != std::string::npos && labelText.find("secret is required") == std::string::npos && labelText.find("hidden treasure") == std::string::npos) return;
        
        auto originalMenu = as<CCMenu*>(m_mainLayer->getChildByID("button-menu"));
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

        auto parameters = as<BetterUnlockInfo_Params*>(as<CCNode*>(sender)->getUserObject());
        
        
        if (labelText.find("buy") != std::string::npos) 
        {
            //if owns
            if (trueIsItemUnlocked(parameters->m_IconId, parameters->m_UnlockType))
            {
                FLAlertLayer::create("Owned!", "You already own this item", "OK")->show();
                return;
            }
            
            std::ifstream file(Mod::get()->getResourcesDir() / "shops.json");
            matjson::Value json = matjson::parse(file).unwrap();
            
            int shoptype;
            int price;
            
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(parameters->m_UnlockType))
                    for (auto item : list["items"])
                        if (item["IconId"].asInt().unwrap() == parameters->m_IconId)
                        {
                            shoptype = item["ShopType"].asInt().unwrap();
                            price = item["Price"].asInt().unwrap();
                            
                            //if can't afford
                            if (shoptype == 4)
                            {
                                if (item["Price"] > GameStatsManager::sharedState()->getStat("29"))
                                {
                                    FLAlertLayer::create("Too expensive!", "You can't afford this item", "OK")->show();
                                    return;
                                }
                            }
                            else
                            {
                                if (item["Price"] > GameStatsManager::sharedState()->getStat("14"))
                                {
                                    FLAlertLayer::create("Too expensive!", "You can't afford this item", "OK")->show();
                                    return;
                                }
                            }
                            
                            
                            //buy data into popup, so can update garage
                            if (CCScene::get()->getChildByID("GJGarageLayer") != nullptr)
                            {
                                this->setUserObject(new BetterUnlockInfo_Params(
                                    item["IconId"].asInt().unwrap(),
                                    parameters->m_UnlockType, 
                                    item["Price"].asInt().unwrap(),
                                    item["ShopType"].asInt().unwrap()
                                ));
                            }
                            
                            //buy popup
                            PurchaseItemPopup::create(
                                GJStoreItem::create(
                                    item["ShopItemId"].asInt().unwrap(),
                                    item["IconId"].asInt().unwrap(),
                                    as<int>(parameters->m_UnlockType),
                                    item["Price"].asInt().unwrap(),
                                    as<ShopType>(item["ShopType"].asInt().unwrap())
                                )
                            )->show();
                            goto buyLoopEnd;
                        }
            //goto right above
            buyLoopEnd:
            
            //shows how many orbs you have
            auto buypopup = CCScene::get()->getChildByType<PurchaseItemPopup>(0);
            
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
            
            std::string minus = "-" + std::to_string(price);
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
            matjson::Value json = matjson::parse(file).unwrap();
            
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(parameters->m_UnlockType))
                    for (auto item : list["chests"])
                        if (item["IconId"].asInt().unwrap() == parameters->m_IconId)
                        {
                            int chestType = item["GJRewardType"].asInt().unwrap();
                            std::string desc = "You can find this item in a " + std::string(getSecretChestDesc(chestType));
                            
                            int size = 300;
                            if (chestType == 9)
                                size = 350;
                            
                            FLAlertLayer::create(nullptr, "Which one? This one!", desc, "OK", nullptr, size)->show();
                            return;
                        }
        }

        if (labelText.find("special chest") != std::string::npos)
        {
            std::ifstream file(Mod::get()->getResourcesDir() / "special.json");
            matjson::Value json = matjson::parse(file).unwrap();
            
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(parameters->m_UnlockType))
                    for (auto item : list["chests"])
                        if (item["IconId"].asInt().unwrap() == parameters->m_IconId)
                        {
                            std::string chestType = item["ChestId"].asString().unwrap();
                            int chestTypeInt = std::atoi(chestType.c_str());
                            
                            int size = 300;
                            if (chestTypeInt >= 4 && chestTypeInt <= 6 || chestTypeInt >= 12 && chestTypeInt <= 21)
                                size = 350;
                            
                            std::string desc = "You can unlock this item ";
                            if (chestType[0] == 'w')
                                desc += "at <cj>The Wraith</c> by entering the code <cg>" + item["Code"].asString().unwrap() + "</c>";
                            else
                                desc += std::string(getSpecialChestDesc(chestTypeInt));
                            
                            FLAlertLayer::create(nullptr, "Which one? This one!", desc, "OK", nullptr, size)->show();
                            return;
                        }
            
        }
        
        //note, coins from vaults
        if (labelText.find("secret is required") != std::string::npos || labelText.find("treasure") != std::string::npos)
        {
            std::ifstream file(Mod::get()->getResourcesDir() / "secrets.json");
            matjson::Value json = matjson::parse(file).unwrap();
            
            //find clicked item in json
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(parameters->m_UnlockType))
                    for (auto item : list["items"])
                        if (item["IconId"].asInt().unwrap() == parameters->m_IconId)
                        {
                            std::string desc = "You can unlock this item ";
                            
                            //0 = destry icons
                            //-1 = master detective
                            //-2 = destroy specific icons
                            //1 = vault
                            //2 = vault of secrets
                            //3 = chamber of time
                            switch (item["Vault"].asInt().unwrap())
                            {
                            case 0:
                            desc += "at the <cj>Main Menu</c> by <cr>Destroying</c> the moving <cg>Icons</c>";
                            break;
                            
                            case -1:
                            desc += "at the <cj>Main Levels Selection Screen</c> by <cy>Scrolling</c> past all the levels 3 times and <cy>Clicking</c> the <cy>Secret Coin</c> at the <cg>Comming Soon!</c> page";
                            break;
                            
                            case -2:
                            desc += "at the <cj>Main Menu</c> by <cr>Destroying</c> <cy>This Icon</c> when it'll be moving";
                            break;
                            
                            case 1:
                            desc += "at <cj>The Vault</c> by entering the code <cg>";
                            if (parameters->m_IconId == 64) desc += GameManager::sharedState()->m_playerName;
                            else desc += item["Code"].asString().unwrap();
                            desc += "</c>";
                            break;
                            
                            case 2:
                            desc += "at the <cj>Vault of Secrets</c> by entering the code <cg>";
                            if (parameters->m_IconId == 76) desc += std::to_string(GameStatsManager::sharedState()->getStat("6"));
                            else if (parameters->m_IconId == 78) //uberhacker
                            {
                                int code = GameManager::sharedState()->m_secretNumber.value()*-1;
                                if (code == 0) 
                                {
                                    desc += item["Code"].asString().unwrap() + "</c> and then checking this pop up again for the answer";
                                    break;
                                }
                                desc += std::to_string(code);
                            }
                            else desc += item["Code"].asString().unwrap();
                            desc += "</c>";
                            break;
                            
                            case 3:
                            desc += "at the <co>Chamber of Time</c> by entering the code <cg>" + item["Code"].asString().unwrap() + "</c>";
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
        auto textArea = as<TextArea*>(m_mainLayer->getChildByID("description-area"));
        
        //conflict MH
        if (textArea == nullptr)
            return std::string("unlock 2.21");
        
        std::string labelText = "";
        auto multiline = textArea->getChildByType<MultilineBitmapFont>(0);
        for (auto label : CCArrayExt<CCLabelBMFont*>(multiline->getChildren()) )
            labelText.append(label->getString());
        
        return labelText;
    }

    const char* getSecretChestDesc(int iGJRewardType)
    {
        switch (iGJRewardType)
        {
        case 1: return "<cy>4 hour Chest</c>"; break;
        case 2: return "<cy>24 hour Chest</c>"; break;
        case 3: return "<cy>1 Key Chest</c>"; break;
        case 4: return "<cy>5 Key Chest</c>"; break;
        case 5: return "<cy>10 Key Chest</c>"; break;
        case 6: return "<cy>25 Key Chest</c>"; break;
        case 7: return "<cy>50 Key Chest</c>"; break;
        case 8: return "<cy>100 Key Chest</c>"; break;
        case 9: return "<cy>Gold Wraith Chest</c>"; break;
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

    //progress bar
    typedef struct AchiObj {
        std::string achiLong = "";
        std::string achiShort = "";
        
        int currentValueFrom = 0;
        int currentValue = 0;
        int maxValue = 1;
        
        std::string sprite = "";
        float scale = 0.5f;
        float moveY = 0;
        ccColor3B color = {0, 0, 0};
    } AchiObj;
    
    void addCompletionProgress(int iconId, UnlockType unlockType) 
    {
        if (textFromArea().find("2.21") != std::string::npos) return; //note, 2.21
        
        AchiObj achiobj = {};
        
        std::ifstream achifile(Mod::get()->getResourcesDir() / "achievements.json");
        matjson::Value achi = matjson::parse(achifile).unwrap();
        for (auto list : achi)
            if (list["UnlockType"].asInt().unwrap() == as<int>(unlockType))
                for (auto item : list["items"])
                    if (item["IconId"].asInt().unwrap() == iconId)
                    {
                        achiobj.achiLong = item["AchievementIdFull"].asString().unwrap();
                        achiobj.achiShort = item["AchievementIdShort"].asString().unwrap();
                        achiobj.maxValue = item["MaxValue"].asInt().unwrap();
                        goto achiLoopEnd;
                    }
        achiLoopEnd:
        
        //if isnt achievement icon
        if (achiobj.achiShort == "")
        {
            setCompletionProgressDataNotAchi(iconId, unlockType, &achiobj);
            addCompletionProgressNodes(iconId, unlockType, &achiobj);
            return;
        }
        
        
        std::ifstream achitypefile(Mod::get()->getResourcesDir() / "achievementsTypes.json");
        matjson::Value achitypes = matjson::parse(achitypefile).unwrap();
        achiobj.currentValueFrom = achitypes[achiobj.achiShort]["CurrentValueFrom"].asInt().unwrap();
        achiobj.sprite = achitypes[achiobj.achiShort]["Sprite"].asString().unwrap();
        achiobj.scale = achitypes[achiobj.achiShort]["Scale"].asDouble().unwrap();
        achiobj.moveY = achitypes[achiobj.achiShort]["MoveY"].asDouble().unwrap();
        achiobj.color = ccColor3B(
            achitypes[achiobj.achiShort]["Color"]["R"].asInt().unwrap(),
            achitypes[achiobj.achiShort]["Color"]["G"].asInt().unwrap(),
            achitypes[achiobj.achiShort]["Color"]["B"].asInt().unwrap()
        );
                
        getCompletionProgressData(iconId, unlockType, &achiobj);
        
        if (achiobj.currentValue == -1) return;
        
        addCompletionProgressNodes(iconId, unlockType, &achiobj);
    }
    
    void getCompletionProgressData(int iconId, UnlockType unlockType, AchiObj* achiobj)
    {
        GameStatsManager* GSM = GameStatsManager::sharedState();
        GameLevelManager* GLM = GameLevelManager::sharedState();
        
        //legends table is private lol
        switch (achiobj->currentValueFrom)
        {
        case -1 : achiobj->currentValue = -1; break;
        case -2 : achiobj->currentValue = trueIsItemUnlocked(iconId, unlockType); break;
        case -3 : achiobj->currentValue = GLM->m_followedCreators->count(); break;
        case -4 : 
            switch (iconId)
            {
            case 59:
            achiobj->currentValue = GSM->getCollectedCoinsForLevel(as<GJGameLevel*>(GLM->m_mainLevels->objectForKey("20")));
            break;
            
            case 60:
            achiobj->currentValue = GSM->getCollectedCoinsForLevel(as<GJGameLevel*>(GLM->m_mainLevels->objectForKey("18")));
            break;
            
            case 14:
            achiobj->currentValue = GSM->getCollectedCoinsForLevel(as<GJGameLevel*>(GLM->m_mainLevels->objectForKey("14")));
            break;
            
            default: achiobj->currentValue = -1; break;
            }
            break;
        case -5 : achiobj->currentValue = as<GJGameLevel*>(GLM->m_mainLevels->objectForKey(std::to_string(std::atoi(achiobj->achiLong.substr(18, 2).c_str()))))->m_practicePercent == 100; break;
        case -6 : achiobj->currentValue = GSM->hasCompletedMainLevel(std::atoi(achiobj->achiLong.substr(18, 2).c_str())); break;
        case -7 :
            if (achiobj->maxValue > 1) achiobj->currentValue = GSM->getStat("9");
            else achiobj->currentValue = trueIsItemUnlocked(iconId, unlockType);
            break;
        case -8 :
        {
            int shards[5] = {GSM->getStat("16"), GSM->getStat("17"), GSM->getStat("18"), GSM->getStat("19"), GSM->getStat("20")};
            int smallest = INT_MAX;
            for (int i = 0; i < 4; i++)
                if (smallest > shards[i])
                    smallest = shards[i];
            achiobj->currentValue = smallest;
            break;
        }
        case -9 : 
        {   
            int shards[5] = {GSM->getStat("23"), GSM->getStat("24"), GSM->getStat("25"), GSM->getStat("26"), GSM->getStat("27")};
            int smallest = INT_MAX;
            for (int i = 0; i < 4; i++)
                if (smallest > shards[i])
                    smallest = shards[i];
            achiobj->currentValue = smallest;
            break;
        }
        case -10 : 
            if (achiobj->achiLong.back() == '1')
                for (int i = 1; i <= 3; i++)
                    achiobj->currentValue += GSM->hasCompletedMainLevel(i);
            else achiobj->currentValue = GSM->hasCompletedMainLevel(14);
            break;
        case -11 : achiobj->currentValue = GSM->hasCompletedMainLevel(achiobj->achiLong.at(19) - '0' + 5000); break;
        case -12 : achiobj->currentValue = GSM->getCollectedCoinsForLevel(as<GJGameLevel*>(GLM->m_mainLevels->objectForKey(std::string("50").append(achiobj->achiLong.substr(18, 2))))); break;
        case -13 :
        {
            /*FriendsProfilePage::create error
            FriendsProfilePage* friends = FriendsProfilePage::create(UserListType::Friends);
            for (auto node : CCArrayExt<CCNode*>(friends->m_mainLayer->getChildren()))
                if (typeinfo_cast<CCLabelBMFont*>(node) != nullptr)
                {
                    std::string nodetext = as<CCLabelBMFont*>(node)->getString();
                    std::string number = nodetext.substr(nodetext.find(":") + 2, nodetext.length() - 1 - nodetext.find(":") + 2);
                    int num = std::atoi(number.c_str());
                    if (num > 0) currentValue = num;
                }
            friends->release(); //idk if works, everytime you click this it still adds 2mb to memory and doesnt go down really
            */
            break;
        }
        default: 
            if (achiobj->currentValueFrom >= 30 && achiobj->currentValueFrom <= 39) 
                achiobj->currentValue = GSM->getStat(std::to_string(achiobj->currentValueFrom).c_str()) / 100; //paths
            else if (achiobj->currentValueFrom >= 1)
                achiobj->currentValue = GSM->getStat(std::to_string(achiobj->currentValueFrom).c_str());
            else
                FLAlertLayer::create("You shouldn't see this", "If you see this msg @rynat on discord", "OK")->show();
            break;
        }
    }
    
    void setCompletionProgressDataNotAchi(int iconId, UnlockType unlockType, AchiObj* achiobj)
    {
        achiobj->currentValue = trueIsItemUnlocked(iconId, unlockType);
        std::string labelText = textFromArea();
        
        if(labelText.find("buy") != std::string::npos)
        {
            achiobj->scale = 0.7f;
            achiobj->sprite = "currencyOrbIcon_001.png";
            
            std::ifstream file(Mod::get()->getResourcesDir() / "shops.json");
            matjson::Value json = matjson::parse(file).unwrap();
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(unlockType))
                    for (auto item : list["items"])
                        if (item["IconId"].asInt().unwrap() == iconId)
                            if (item["ShopType"].asInt().unwrap() == 4)
                                achiobj->sprite = "currencyDiamondIcon_001.png";
        }
        else if (labelText.find("secret chest") != std::string::npos)
        {
            achiobj->scale = 0.15f;
            achiobj->sprite = "chest_03_02_001.png";
            
            std::ifstream file(Mod::get()->getResourcesDir() / "chests.json");
            matjson::Value json = matjson::parse(file).unwrap();
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(unlockType))
                    for (auto item : list["chests"])
                        if (item["IconId"].asInt().unwrap() == iconId)
                            achiobj->sprite = "chest_0" + std::to_string(item["GJRewardType"].asInt().unwrap()) + "_02_001.png";
        }
        else if (labelText.find("special chest") != std::string::npos)
        {
            achiobj->scale = 0.15f;
            achiobj->sprite = "chest_02_02_001.png";
            
            std::ifstream file(Mod::get()->getResourcesDir() / "special.json");
            matjson::Value json = matjson::parse(file).unwrap();
            
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(unlockType))
                    for (auto item : list["chests"])
                        if (item["IconId"].asInt().unwrap() == iconId)
                            if ((item["ChestId"].asString().unwrap())[0] == 'w')
                            {
                                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("SecretSheet.plist", "SecretSheet.png");
                                achiobj->sprite = "GJ_secretLock4_001.png";
                            }
        }
        else
        {
            std::ifstream file(Mod::get()->getResourcesDir() / "special.json");
            matjson::Value json = matjson::parse(file).unwrap();
            std::string chestId = "";
            
            for (auto list : json)
                if (list["UnlockType"].asInt().unwrap() == as<int>(unlockType))
                    for (auto item : list["chests"])
                        if (item["IconId"].asInt().unwrap() == iconId)
                            chestId = item["ChestId"].asString().unwrap();
            
            if (labelText.find("gauntlet") != std::string::npos)
            {
                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("GauntletSheet.plist", "GauntletSheet.png");
                int num = std::atoi(chestId.substr(2, std::string::npos).c_str());
                
                switch (num)
                {
                case 1: achiobj->sprite = "island_fire_001.png"; break;
                case 2: achiobj->sprite = "island_ice_001.png"; break;
                case 3: achiobj->sprite = "island_poison_001.png"; break;
                case 4: achiobj->sprite = "island_shadow_001.png"; break;
                case 5: achiobj->sprite = "island_lava_001.png"; break;
                case 6: achiobj->sprite = "island_bonus_001.png"; break;
                case 7: achiobj->sprite = "island_chaos_001.png"; break;
                case 8: achiobj->sprite = "island_demon_001.png"; break;
                case 9: achiobj->sprite = "island_time_001.png"; break;
                case 10: achiobj->sprite = "island_crystal_001.png"; break;
                case 11: achiobj->sprite = "island_magic_001.png"; break;
                case 12: achiobj->sprite = "island_spike_001.png"; break;
                case 13: achiobj->sprite = "island_monster_001.png"; break;
                case 14: achiobj->sprite = "island_doom_001.png"; break;
                case 15: achiobj->sprite = "island_death_001.png"; break;
                case 51: achiobj->sprite = "island_ncs01_001.png"; break;
                case 52: achiobj->sprite = "island_ncs02_001.png"; break;
                case 53: achiobj->sprite = "island_space_001.png"; break;
                case 54: achiobj->sprite = "island_cosmos_001.png"; break;
                default:
                    std::string numtext = "";
                    if (num - 15 < 10) numtext = "0";
                    numtext.append(std::to_string(num-15));
                    achiobj->sprite = std::string("island_new").append(numtext).append("_001.png");
                }
                achiobj->scale = 0.2f;
                achiobj->moveY = -3;
            }
            if (labelText.find("Complete the Path") != std::string::npos)
            {
                std::string number = chestId.substr(3, std::string::npos);
                
                std::string path = "path";
                if (number != "10") path.append("0");
                path.append(number);
                
                std::ifstream achitypefile(Mod::get()->getResourcesDir() / "achievementsTypes.json");
                matjson::Value achitypes = matjson::parse(achitypefile).unwrap();
                achiobj->sprite = achitypes[path]["Sprite"].asString().unwrap();
                achiobj->scale = achitypes[path]["Scale"].asDouble().unwrap();
                achiobj->moveY = achitypes[path]["MoveY"].asDouble().unwrap();
                achiobj->color = ccColor3B(
                    achitypes[path]["Color"]["R"].asInt().unwrap(),
                    achitypes[path]["Color"]["G"].asInt().unwrap(),
                    achitypes[path]["Color"]["B"].asInt().unwrap()
                );
            }
        }
    }
    
    void addCompletionProgressNodes(int iconId, UnlockType unlockType, AchiObj* achiobj)
    {
        CCSize screenSize = CCDirector::sharedDirector()->getWinSize();
        
        CCMenu* menu = CCMenu::create();
        menu->setID("completionMenu");
        
        
        Slider* slider = Slider::create(this, nullptr);
        if (achiobj->color != (ccColor3B){0,0,0})
        {
            ccTexParams sliderParams = {GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT};
            slider->m_sliderBar->setTexture(CCSprite::create(fmt::format("{}/{}", Mod::get()->getID(), "_sliderBar.png").c_str())->getTexture());
            slider->m_sliderBar->getTexture()->setTexParameters(&sliderParams);
            slider->m_sliderBar->setColor(achiobj->color);
        }
        slider->removeChild(slider->m_touchLogic);
        slider->setID("completionSlider");
        slider->setScale(0.45f);
        slider->setAnchorPoint(CCPoint(0,0));
        slider->setPosition(CCPoint(81, -85));
        slider->setValue(as<float>(achiobj->currentValue) / as<float>(achiobj->maxValue));
        menu->addChild(slider);
        
        
        CCSprite* icon = CCSprite::createWithSpriteFrameName("GJ_lock_001.png");
        icon->setPosition(CCPoint(129, -84));
        if (trueIsItemUnlocked(iconId, unlockType))
        {
            icon = CCSprite::createWithSpriteFrameName("GJ_achImage_001.png");
            icon->setPosition(CCPoint(130, -85));
        }
        icon->setID("completionIcon");
        icon->setScale(0.45f);
        menu->addChild(icon);
        
        
        //create labels
        CCLabelBMFont* labelCount = CCLabelBMFont::create(std::to_string(achiobj->currentValue).c_str(), "bigFont.fnt");
        CCLabelBMFont* labelSlash = CCLabelBMFont::create("/", "bigFont.fnt");
        CCLabelBMFont* labelMax = CCLabelBMFont::create(std::to_string(achiobj->maxValue).c_str(), "bigFont.fnt");
        
        if (achiobj->currentValue >= 1000)
        {
            float newCurrent = std::round((achiobj->currentValue / 1000.f) * 10) / 10;
            labelCount = CCLabelBMFont::create(fmt::format("{}", newCurrent).append("K").c_str(), "bigFont.fnt");
            if (achiobj->currentValue >= 1000000)
            {
                newCurrent = std::round((achiobj->currentValue / 1000000.f) * 10) / 10;
                labelCount = CCLabelBMFont::create(fmt::format("{}", newCurrent).append("M").c_str(), "bigFont.fnt");
            }
        }
        if (achiobj->maxValue >= 1000)
        {
            float newMax = std::round((achiobj->maxValue / 1000.f) * 10) / 10;
            labelMax = CCLabelBMFont::create(fmt::format("{}", newMax).append("K").c_str(), "bigFont.fnt");
            if (achiobj->maxValue >= 1000000)
            {
                newMax = std::round((achiobj->maxValue / 1000000.f) * 10) / 10;
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
        
        menu->addChild(labelCount);
        menu->addChild(labelSlash);
        menu->addChild(labelMax);
        
        
        //add type icon
        if (achiobj->sprite != "")
        {
            if (achiobj->achiShort == "gauntlets")
                CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFramesWithFile("GauntletSheet.plist", "GauntletSheet.png");
            
            CCSprite* groupIcon = CCSprite::createWithSpriteFrameName(achiobj->sprite.c_str());
            if (achiobj->sprite[0] == '_') groupIcon = CCSprite::createWithSpriteFrameName(fmt::format("{}/{}", Mod::get()->getID(), achiobj->sprite).c_str());
            else if (achiobj->sprite == "GJ_downloadsIcon_001.png") groupIcon->setRotation(180);
            groupIcon->setID("completionGroupIcon");
            groupIcon->setPosition(CCPoint(labelMax->getPositionX() + labelMax->getContentSize().width * 0.35f + groupIcon->getContentSize().width/(2.f/achiobj->scale) + 1, -97.5f + achiobj->moveY));
            groupIcon->setScale(achiobj->scale);
            menu->addChild(groupIcon);
        }
        
        m_mainLayer->addChild(menu);
        
        CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile("GauntletSheet.plist");
        CCSpriteFrameCache::sharedSpriteFrameCache()->removeSpriteFramesFromFile("SecretSheet.plist");
    }
    
    void addCompletionIconOnly(int iconId, UnlockType unlockType)
    {
        if (textFromArea().find("2.21") != std::string::npos) return; //note, 2.21
        
        CCMenu* originalMenu = as<CCMenu*>(m_mainLayer->getChildByID("button-menu"));
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
    
    //equip button
    void addEquipButton(int iconId, UnlockType unlockType)
    {
        if (!trueIsItemUnlocked(iconId, unlockType)) return;
        
        CCMenu* originalMenu = as<CCMenu*>(m_mainLayer->getChildByID("button-menu"));
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
        
        //for unlocked spinoff games icons
        auto spinoff = m_mainLayer->getChildByID("spinoff-logo");
        if (spinoff != nullptr)
            equipButton->setPositionY(equipButton->getPositionY() - spinoff->getContentHeight() * 0.4f - 5);
        
        equipButton->setUserObject(new BetterUnlockInfo_Params(iconId, unlockType));
        originalMenu->addChild(equipButton);
    }
    
    void onEquipButtonClick(CCObject* sender) 
    {
        auto parameters = as<BetterUnlockInfo_Params*>(as<CCNode*>(sender)->getUserObject());     
        auto GM = GameManager::sharedState();      
        
        //note save it idk
        switch (as<int>(parameters->m_UnlockType))
        {
        case 1: GM->setPlayerFrame(parameters->m_IconId); break;
        case 2: GM->setPlayerColor(parameters->m_IconId); break;
        case 3:
        {   
            auto button = CCScene::get()->getChildByType<ItemInfoPopup>(0)->m_mainLayer->getChildByID("button-color-menu")->getChildByTag(1);
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
            FLAlertLayer::create("Note", "By equiping icon this way it won't update on your profile until you restart the game", "OK")->show();
        
        if (CCScene::get()->getChildByType<geode::Notification>(0) == nullptr)
            geode::Notification::create("Equipped", CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png"))->show();
    }
    
    //mics
    bool trueIsItemUnlocked(int iconId, UnlockType unlockType)
    {
        if (unlockType == UnlockType::Col1 || unlockType == UnlockType::Col2)
            return GameManager::get()->isColorUnlocked(iconId, unlockType);
      
        return GameManager::get()->isIconUnlocked(iconId, UnlockToIcon(unlockType));
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
        case UnlockType::ShipFire: return IconType::ShipFire;
        case UnlockType::GJItem: return IconType::Item;
        default:break;
        }
        
        return IconType::Cube;
    }
    
    void moveCredit()
    {
        auto buttonMenu = m_mainLayer->getChildByID("button-menu");
        
        auto cvoltonButton = buttonMenu->getChildByID("cvolton.betterinfo/chest-reveal-button");
        if (cvoltonButton != nullptr)
            cvoltonButton->removeFromParent();
        
        auto authorButton = as<CCMenuItemSpriteExtra*>(buttonMenu->getChildByID("author-button"));
        if (authorButton == nullptr) return;
        
        authorButton->setPosition(CCPoint(0, 173));
        authorButton->m_baseScale = 0.8f;
        authorButton->setScale(0.8f);
        
        auto icon = m_mainLayer->getChildByID("item-icon");
        icon->setScale(1.1f);
        icon->setPositionY(icon->getPositionY() - 6);
    }
};
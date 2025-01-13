#include "Includes.hpp"
#include <Geode/modify/PurchaseItemPopup.hpp>


// fix sound and "refresh" garage layer
class $modify(PurchaseItemPopup)
{
    void onPurchase(CCObject* sender)
    {
        PurchaseItemPopup::onPurchase(sender);
        
        auto scene = CCScene::get();
        if (scene->getChildByType<GJShopLayer>(0) == nullptr)
        {
            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");

            GJGarageLayer* garage = scene->getChildByType<GJGarageLayer>(0);
            if (garage != nullptr)
            {
                auto parameters = as<BetterUnlockInfo_Params*>(CCScene::get()->getChildByType<ItemInfoPopup>(0)->getUserObject());
                
                //udpate money label
                CCLabelBMFont* money = as<CCLabelBMFont*>(garage->getChildByID("orbs-label"));
                if (parameters->m_ShopType == 4)
                    money = as<CCLabelBMFont*>(garage->getChildByID("diamond-shards-label"));
                money->setString(std::to_string(std::atoi(money->getString()) - parameters->m_Price).c_str());
                
                //get button
                CCMenuItemSpriteExtra* iconButton;
                switch (parameters->m_UnlockType) 
                {
                    case UnlockType::ShipFire:
                        iconButton = as<CCMenuItemSpriteExtra*>(
                            garage->
                            getChildByType<ListButtonBar>(0)->
                            getChildByType<ListButtonBar>(0)->
                            getChildByType<BoomScrollLayer>(0)->
                            getChildByType<ExtendedLayer>(0)->
                            getChildByType<ListButtonPage>(0)->
                            getChildByType<CCMenu>(0)->
                            getChildByTag(parameters->m_IconId)
                        );
                    break;
                    
                    case UnlockType::GJItem:
                        iconButton = as<CCMenuItemSpriteExtra*>(
                            garage->
                            getChildByType<ListButtonBar>(0)->
                            getChildByType<CCMenu>(0)->
                            getChildByTag(parameters->m_IconId)
                        );
                    break;
                    
                    case UnlockType::Col1:
                    case UnlockType::Col2:
                        iconButton = as<CCMenuItemSpriteExtra*>(
                            garage->
                            getChildByType<CharacterColorPage>(0)->
                            getChildByType<CCLayer>(0)->
                            getChildByType<CCMenu>(0)->
                            getChildByID(std::to_string(parameters->m_IconId).c_str())
                        );
                    break;
                      
                    default:
                        iconButton = as<CCMenuItemSpriteExtra*>(
                            garage->
                            getChildByType<ListButtonBar>(0)->
                            getChildByType<BoomScrollLayer>(0)->
                            getChildByType<ExtendedLayer>(0)->
                            getChildByType<ListButtonPage>(0)->
                            getChildByType<CCMenu>(0)->
                            getChildByTag(parameters->m_IconId)
                        );
                    break;
                }
                if (iconButton == nullptr) goto end;
                
                //set scale
                float scale;
                switch (parameters->m_UnlockType) {
                    case UnlockType::Ship: scale = 0.6f; break;
                    case UnlockType::Ball: scale = 0.75f; break;
                    case UnlockType::Bird: scale = 0.68f; break;
                    case UnlockType::Robot: scale = 0.65f; break;
                    case UnlockType::Spider: scale = 0.65f; break;
                    case UnlockType::Swing: scale = 0.7f; break;
                    case UnlockType::Jetpack: scale = 0.6f; break;
                    default: scale = 0.8f; break;
                }
                
                //update icon
                if (parameters->m_UnlockType == UnlockType::Col1 || parameters->m_UnlockType == UnlockType::Col2)
                    iconButton->getChildByType<ColorChannelSprite>(0)->getChildByType<CCSprite>(0)->setVisible(false);
                else
                {
                    iconButton->removeAllChildren();
                    
                    GJItemIcon* newIcon = GJItemIcon::createBrowserItem(parameters->m_UnlockType, parameters->m_IconId);
                    newIcon->setTag(1);
                    newIcon->setPosition(CCPoint(15, 15));
                    newIcon->setScale(scale);
                    iconButton->addChild(newIcon);
                }
                
                end:
                CCTouchDispatcher::get()->unregisterForcePrio(scene->getChildByType<ItemInfoPopup>(0));
                scene->removeChild(scene->getChildByType<ItemInfoPopup>(0));
                
                /*  tf why error
                if(parameters->m_UnlockType == UnlockType::GJItem)
                    garage->onToggleItem(iconButton);*/
            }
            
            ProfilePage* profile = scene->getChildByType<ProfilePage>(0);
            if (profile != nullptr)
                scene->removeChild(as<CCNode*>(scene->getChildren()->objectAtIndex(scene->getChildrenCount()-1))); //remove item popup
            
        }
    }
};
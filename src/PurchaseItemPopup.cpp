#include "Includes.hpp"
#include <Geode/modify/PurchaseItemPopup.hpp>


// fix sound and "refresh" garage layer
class $modify(PurchaseItemPopup)
{
    void onPurchase(CCObject* sender)
    {
        PurchaseItemPopup::onPurchase(sender);
        
        auto scene = CCScene::get();
        if (getChildOfType<GJShopLayer>(scene, 0) == nullptr)
        {
            FMODAudioEngine::sharedEngine()->playEffect("buyItem01.ogg");
            
            GJGarageLayer* garage = getChildOfType<GJGarageLayer>(scene, 0);
            if (garage != nullptr)
            {
                auto parameters = static_cast<BetterUnlockInfo_Params*>(garage->getChildByID("BUInode")->getUserObject());
                
                //udpate money label
                CCLabelBMFont* money = static_cast<CCLabelBMFont*>(garage->getChildByID("orbs-label"));
                if (parameters->m_ShopType == 4)
                    money = static_cast<CCLabelBMFont*>(garage->getChildByID("diamond-shards-label"));
                money->setString(std::to_string(std::atoi(money->getString()) - parameters->m_Price).c_str());
                
                //update icon
                CCMenuItemSpriteExtra* iconButton;
                switch (parameters->m_UnlockType) 
                {
                    case UnlockType::ShipFire:
                        iconButton = static_cast<CCMenuItemSpriteExtra*>(
                            getChildOfType<CCMenu>(
                                getChildOfType<ListButtonPage>(
                                    getChildOfType<ExtendedLayer>(
                                        getChildOfType<BoomScrollLayer>(
                                            getChildOfType<ListButtonBar>(
                                                getChildOfType<ListButtonBar>(
                                                    garage, 0
                                                ), 0
                                            ), 0
                                        ), 0
                                    ), 0
                                ), 0
                            )->getChildByTag(parameters->m_IconId)
                        );
                        if (iconButton == nullptr) goto end;
                    break;
                    
                    case UnlockType::GJItem:
                        iconButton = static_cast<CCMenuItemSpriteExtra*>(
                            getChildOfType<CCMenu>(
                                getChildOfType<ListButtonBar>(garage, 0
                                ), 0
                            )->getChildByTag(parameters->m_IconId)
                        );
                    break;
                    
                    case UnlockType::Col1:
                    case UnlockType::Col2:
                        iconButton = static_cast<CCMenuItemSpriteExtra*>(
                            getChildOfType<CCMenu>(
                                getChildOfType<CCLayer>(
                                    getChildOfType<CharacterColorPage>(
                                        garage, 0
                                    ), 0
                                ), 0
                            )->getChildByID(std::to_string(parameters->m_IconId).c_str())
                        );
                    break;
                      
                    default:
                        iconButton = static_cast<CCMenuItemSpriteExtra*>(
                            getChildOfType<CCMenu>(
                                getChildOfType<ListButtonPage>(
                                    getChildOfType<ExtendedLayer>(
                                        getChildOfType<BoomScrollLayer>(
                                            getChildOfType<ListButtonBar>(
                                                garage, 0
                                            ), 0
                                        ), 0
                                    ), 0
                                ), 0
                            )->getChildByTag(parameters->m_IconId)
                        );
                    break;
                }
                
                //create unlocked icon
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
                
                if (parameters->m_UnlockType == UnlockType::Col1 || parameters->m_UnlockType == UnlockType::Col2)
                {
                    getChildOfType<CCSprite>(getChildOfType<ColorChannelSprite>(iconButton, 0), 0)->setVisible(false);
                }
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
                garage->removeChildByID("BUInode");
                CCTouchDispatcher::get()->unregisterForcePrio(getChildOfType<ItemInfoPopup>(scene, 0));
                scene->removeChild(getChildOfType<ItemInfoPopup>(scene, 0));
            }
            
            ProfilePage* profile = getChildOfType<ProfilePage>(scene, 0);
            if (profile != nullptr)
                scene->removeChild(static_cast<CCNode*>(scene->getChildren()->objectAtIndex(scene->getChildrenCount()-1))); //remove item popup
            
        }
    }
    
    void onClose(CCObject* sender)
    {
        PurchaseItemPopup::onClose(sender);
        
        GJGarageLayer* garage = getChildOfType<GJGarageLayer>(CCScene::get(), 0);
        if (garage != nullptr)
            garage->removeChildByID("BUInode");
    }
};
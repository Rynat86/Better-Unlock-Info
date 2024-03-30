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
                CCMenuItemSpriteExtra* iconButton = static_cast<CCMenuItemSpriteExtra*>(
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
                iconButton->removeAllChildren();
                
                //create unlocked icon
                GJItemIcon* newIcon = GJItemIcon::createBrowserItem(parameters->m_UnlockType, parameters->m_IconId);
                newIcon->setTag(1);
                newIcon->setPosition(CCPoint(15, 15));
                newIcon->setScale(0.8f);
                iconButton->addChild(newIcon);
                
                garage->removeChildByID("BUInode");

                scene->removeChild(getChildOfType<ItemInfoPopup>(scene, 0));
            }
            
            ProfilePage* profile = getChildOfType<ProfilePage>(scene, 0);
            if (profile != nullptr)
                scene->removeChild(static_cast<CCNode*>(scene->getChildren()->objectAtIndex(scene->getChildrenCount()-1)));
            
        }
    }
    
    void onClose(CCObject* sender)
    {
        PurchaseItemPopup::onClose(sender);
        
        GJGarageLayer* garage = getChildOfType<GJGarageLayer>(CCScene::get(), 0);
        if (garage != nullptr)
            garage->removeChildByID("dummyInfoNode");
    }
};
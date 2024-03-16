#pragma once

#include <Geode/Geode.hpp>
using namespace geode::prelude;


//for popup
struct BetterUnlockInfo_Params : public CCObject
{
    int m_IconId;
    UnlockType m_UnlockType;
    
    int m_Price;
    int m_ShopType;
    
    BetterUnlockInfo_Params(int iconId, UnlockType unlockType)
        : m_IconId(iconId), m_UnlockType(unlockType)
    {
        this->autorelease();
    }
    
    BetterUnlockInfo_Params(int iconId, UnlockType unlockType, int price, int shopType) 
        : m_IconId(iconId), m_UnlockType(unlockType), m_Price(price), m_ShopType(shopType)
    {
        this->autorelease();
    }
};

//for profile
struct IconObject
{
    UnlockType unlockType;
    int iconId;
};
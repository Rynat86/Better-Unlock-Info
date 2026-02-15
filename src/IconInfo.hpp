#include "Includes.hpp"

struct IconInfo : public CCObject
{
    int m_IconId;
    UnlockType m_UnlockType;
    
    int m_Price;
    int m_ShopType;
    
    IconInfo(int iconId, UnlockType unlockType)
        : m_IconId(iconId), m_UnlockType(unlockType)
    {
        this->autorelease();
    }
    
    IconInfo(int iconId, UnlockType unlockType, int price, int shopType) 
        : m_IconId(iconId), m_UnlockType(unlockType), m_Price(price), m_ShopType(shopType)
    {
        this->autorelease();
    }
};
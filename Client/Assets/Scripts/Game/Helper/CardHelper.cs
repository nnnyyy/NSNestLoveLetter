using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    //카드 정보
    public static class CardHelper
    {
        static Dictionary<CardType, string> m_DicCardImage = new Dictionary<CardType, string>()
        {
            { CardType.Unknown, "atlas_back" },
            { CardType.Guard, "atlas_guard" },
            { CardType.RoyalSubject, "atlas_royal_subject" },
            { CardType.Gossip, "atlas_gossip" },
            { CardType.Companion, "atlas_companion" },
            { CardType.Hero, "atlas_hero" },
            { CardType.Wizard, "atlas_wizard" },
            { CardType.Lady, "atlas_lady" },
            { CardType.Princess, "atlas_princess" }
        };

        public static string GetCardImageName(this ICardInfo cardInfo, CardType cardType)
        {
            return m_DicCardImage[cardType];
        }
    
    }
}

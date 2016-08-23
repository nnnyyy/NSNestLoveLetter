using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    public enum CardType
    {
        Back = 0,
        Guard = 1,
        RoyalSubject = 2,
        Gossip = 3,
        Companion = 4,
        Hero = 5,
        Wizard = 6,
        Lady = 7,
        Princess = 8
    }
    
    public enum CardStatus
    {
        Open = 1,
        Close = 2
    }

    public enum CardPosition
    {
        Hand = 1,
        Board = 2
    }

    //카드 정보
    public static class CardInfo
    {
        static Dictionary<CardType, string> m_DicCardImage = new Dictionary<CardType, string>()
        {
            { CardType.Back, "" },
            { CardType.Guard, "" },
            { CardType.RoyalSubject, "" },
            { CardType.Gossip, "" },
            { CardType.Companion, "" },
            { CardType.Hero, "" },
            { CardType.Wizard, "" },
            { CardType.Lady, "" },
            { CardType.Princess, "" }
        };

        public static string GetCardImageName(CardType cardType)
        {
            return m_DicCardImage[cardType];
        }
    
    }
}

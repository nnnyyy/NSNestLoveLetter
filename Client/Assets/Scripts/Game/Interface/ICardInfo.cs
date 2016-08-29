using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    public enum CardType
    {
        Unknown = 0,
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

    /// <summary>
    /// 카드의 정보
    /// </summary>
    public interface ICardInfo : ICardEvent
    {
        /// <summary>
        /// 카드의 Wordl Position을 조정합니다.
        /// </summary>
        void SetPosition(Vector3 position);

        /// <summary>
        /// 카드 소유자 (0은 무소유)
        /// </summary>
        int CardOwner { get; }

        /// <summary>
        /// 카드 종류
        /// </summary>
        CardType CardType { get; }

        /// <summary>
        /// 카드 상태(Open, Close)
        /// </summary>
        CardStatus CardStatus { get; }

        /// <summary>
        /// 카드 위치
        /// </summary>
        CardPosition CardPosition { get; }
    }

    public interface ICardInfoModify
    {
        /// <summary>
        /// 카드 소유자 (0은 무소유)
        /// </summary>
        int CardOwner { get; set; }
        
        /// <summary>
        /// 카드 상태(Open, Close)
        /// </summary>
        CardStatus CardStatus { get; set; }

        /// <summary>
        /// 카드 위치
        /// </summary>
        CardPosition CardPosition { get; set; }
    }
}

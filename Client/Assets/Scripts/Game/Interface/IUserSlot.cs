using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    /// <summary>
    /// 유저의 정보를 가지고 있습니다.
    /// </summary>
    public interface IUserSlot
    {
        /// <summary>
        /// 유저 고유 넘버
        /// </summary>
        int UserNumber { get; set; }

        /// <summary>
        /// 유저 이름
        /// </summary>
        string UserName { get; set; }

        /// <summary>
        /// 유저 이름
        /// </summary>
        int UserState { get; set; }

        /// <summary>
        /// 동료 방어
        /// </summary>
        bool IsShield { get; set; }

        /// <summary>
        /// 죽음.
        /// </summary>
        bool IsDead { get; set; }

        /// <summary>
        /// 유저 손에 가지고 있는 카드 리스트
        /// </summary>
        List<ICardInfo> ListHandCard { get; }

        /// <summary>
        /// 유저가 내려놓은 카드 리스트
        /// </summary>
        List<ICardInfo> ListLeftCard { get; }

        /// <summary>
        /// 유저 턴 상태값.
        /// </summary>
        bool IsTurn { get; set; }

        /// <summary>
        /// 유저에게 카드를 줍니다. (해당 카드는 유저소유가 됩니다.)
        /// </summary>
        bool AddUserHandCard(ICardInfo cardInfo);

        /// <summary>
        /// 유저에게서 카드를 가져갑니다.(해당 카드는 유저소유가 더이상 아닙니다.)
        /// </summary>
        bool RemoveUserHandCard(ICardInfo cardInfo);

        /// <summary>
        /// 유저의 카드를 내려둡니다.(해당 카드는 유저소유가 유지됩니다.)
        /// </summary>
        bool LeftUserHandCard(ICardInfo cardInfo);
    }
}

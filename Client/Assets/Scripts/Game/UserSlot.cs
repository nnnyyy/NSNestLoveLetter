using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    //유저와 1:1로 매칭되는 카드 처리스크립트
    
    /// <summary>
    /// 유저슬롯은 자신이 가지고 있는 카드의 상태를 바꾸기만 합니다.
    /// 나머지는 BoardAgent 에게 맡깁니다.
    /// </summary>
    public class UserSlot : ComSlot, IUserSlot
    {
        public override void AddUserHandCard(ICardInfo cardInfo)
        {
            base.AddUserHandCard(cardInfo);
            ICardInfoModify cardInfoModify = cardInfo as ICardInfoModify;
            if (cardInfoModify != null)
            {
                cardInfoModify.CardStatus = CardStatus.Open;
            }
            cardInfo.OnPressCard += CardInfo_OnPressCard;
        }
        
        public override void RemoveUserHandCard(ICardInfo cardInfo)
        {
            base.RemoveUserHandCard(cardInfo);
            cardInfo.OnPressCard -= CardInfo_OnPressCard;
        }

        public override void LeftUserHandCard(ICardInfo cardInfo)
        {
            base.LeftUserHandCard(cardInfo);
        }

        private void CardInfo_OnPressCard(ICardInfo cardInfo)
        {
            if (!IsTurn)
            {
                //TODO : 카드 타입에 따라 기능을 선택하고 처리합니다.
                switch (cardInfo.CardType)
                {
                    case CardType.Guard:
                        break;
                }
            }
            else
            {
                //TODO : 카드 정보를 보여줍니다.
            }
        }
    }
}

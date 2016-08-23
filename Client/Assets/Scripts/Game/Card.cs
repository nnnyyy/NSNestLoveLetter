using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    //카드 선택시 이벤트
    public delegate void CardPress(Card card);

    //카드 정보
    public class Card : MonoBehaviour
    {
        [SerializeField]
        UISprite m_SprCard = null;

        CardPress m_OnCardPress = null;
        public event CardPress OnCardPress
        {
            add { m_OnCardPress += value; }
            remove { m_OnCardPress -= value; }
        }

        public CardType CardType;

        //카드의 보여짐 상태.
        CardStatus m_CardStatus = CardStatus.Close;
        public CardStatus CardStatus
        {
            get { return m_CardStatus; }
            set
            {
                m_CardStatus = value;
                SetStatus(m_CardStatus);
            }
        }

        //카드의 위치
        CardPosition m_CardPosition = CardPosition.Hand;
        public CardPosition CardPosition
        {
            get { return m_CardPosition; }
            set
            {
                m_CardPosition = value;
                SetPosition(m_CardPosition);
            }
        }

        public void SetCard(CardType cardType)
        {
            CardType = cardType;
            CardStatus = CardStatus.Close;
        }

        void SetStatus(CardStatus CardStatus)
        {
            if (m_SprCard != null)
                return;

            if(CardStatus == CardStatus.Close)
                m_SprCard.spriteName = CardInfo.GetCardImageName(CardType.Back);
            else
                m_SprCard.spriteName = CardInfo.GetCardImageName(CardType);
        }

        void SetPosition(CardPosition cardPosition)
        {
            //TODO 카드 포지션에 따라 카드 이동처리
        }

        public void OnPressCard()
        {
            if (m_OnCardPress != null)
                m_OnCardPress(this);
        }
    }
}

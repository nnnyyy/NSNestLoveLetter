using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    /// <summary>
    /// 1. 카드 정보
    /// 누가 소유인지, 
    /// 어디(손, 보드)에 있는지
    /// 어떤 상태인지(앞판,뒷판)에 대한 정보 보유
    /// 
    /// 실제 카드 자신은 움직이지 않습니다. 이벤트만 발생합니다.
    /// 카드 위치 조정은 BoardAgent에게 맡깁시다.
    /// </summary>
    public class Card : MonoBehaviour, ICardInfo, ICardInfoModify
    {
        [SerializeField]
        UISprite m_SprCard = null;

        #region ICardEvent

        PressCard m_OnPressCard = null;
        public event PressCard OnPressCard
        {
            add { m_OnPressCard += value; }
            remove { m_OnPressCard -= value; }
        }
        
        MoveCard m_OnMoveCard = null;
        public event MoveCard OnMoveCard
        {
            add { m_OnMoveCard += value; }
            remove { m_OnMoveCard -= value; }
        }

        ChangeCardStatus m_OnChangeCardStatus = null;
        public event ChangeCardStatus OnChangeCardStatus
        {
            add { m_OnChangeCardStatus += value; }
            remove { m_OnChangeCardStatus -= value; }
        }

        #endregion

        #region ICardInfo

        public void SetPosition(Vector3 position)
        {
            transform.position = position;
        }

        int m_CardOwner = 0;
        public int CardOwner
        {
            get { return m_CardOwner; }
            set
            {
                SetCardOwner(m_CardOwner);
            }
        }

        void SetCardOwner(int newUserNumber)
        {
            if (m_CardOwner == newUserNumber)
                return;

            //UserSlot oldUserSlot = BoardAgent.Instance.GetUserSlot(m_CardOwner);
            //if(oldUserSlot != null)
            //    oldUserSlot.RemoveCard(this);

            //m_CardOwner = newUserNumber;
            //UserSlot newUserSlot = BoardAgent.Instance.GetUserSlot(m_CardOwner);
            //if (newUserSlot != null)
            //    newUserSlot.AddCard(this);

            //TODO: 카드 주인의 userSLot으로 위치 이동처리
            if (m_OnMoveCard != null)
                m_OnMoveCard(this, m_CardOwner, CardPosition.Hand);
        }

        CardType m_CardType = CardType.Unknown;
        public CardType CardType
        {
            get { return m_CardType; }
            private set { m_CardType = value; }
        }

        /// <summary>
        /// 카드 상태(Open, Close)
        /// </summary>
        CardStatus m_CardStatus = Game.CardStatus.Close;
        public CardStatus CardStatus
        {
            get { return m_CardStatus; }
            set
            {
                if(m_CardStatus != value)
                {
                    m_CardStatus = value;
                    SetStatus(m_CardStatus);
                }
            }
        }

        void SetStatus(CardStatus cardStatus)
        {
            if (m_SprCard != null)
                return;

            if (cardStatus == CardStatus.Close)
                m_SprCard.spriteName = this.GetCardImageName(CardType.Unknown);
            else
                m_SprCard.spriteName = this.GetCardImageName(CardType);

            if (m_OnChangeCardStatus != null)
                m_OnChangeCardStatus(this, cardStatus);
        }

        CardPosition m_CardPosition = CardPosition.Hand;
        public CardPosition CardPosition
        {
            get { return m_CardPosition; }
            set
            {
                if(m_CardPosition!=value)
                    SetPosition(m_CardOwner, value);
            }
        }

        void SetPosition(int userNumber, CardPosition cardPosition)
        {
            m_CardOwner = userNumber;
            m_CardPosition = cardPosition;

            if (m_OnMoveCard != null)
                m_OnMoveCard(this, m_CardOwner, m_CardPosition);
        }
        
        #endregion

        public void OnPress()
        {
            if (m_OnPressCard != null)
                m_OnPressCard(this);
        }
    }
}

﻿using UnityEngine;
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
    public class ComSlot : MonoBehaviour, IUserSlot
    {
        [Range(1, 10)]
        [SerializeField]
        int m_UserNumber = 0;
        public int UserNumber { get { return m_UserNumber; } }

        [SerializeField]
        GameObject m_ObjUserHand = null;

        [SerializeField]
        GameObject m_ObjUserLeft = null;

        List<ICardInfo> m_ListHandCard = new List<ICardInfo>();
        public List<ICardInfo> ListHandCard { get { return m_ListHandCard; } }

        List<ICardInfo> m_ListLeftCard = new List<ICardInfo>();
        public List<ICardInfo> ListLeftCard { get { return m_ListLeftCard; } }

        bool m_IsTurn = false;
        public bool IsTurn
        {
            get { return m_IsTurn; }
            set
            {
                if (m_IsTurn == value)
                    return;

                m_IsTurn = value;
                SetUserSlotStatus();
            }
        }

        public virtual void AddUserHandCard(ICardInfo cardInfo)
        {
            ICardInfoModify cardInfoModify = cardInfo as ICardInfoModify;
            if (cardInfoModify!=null)
            {
                cardInfoModify.CardOwner = m_UserNumber;
                cardInfoModify.CardStatus = CardStatus.Close;
                cardInfoModify.CardPosition = CardPosition.Hand;
            }
            
            m_ListHandCard.Add(cardInfo);
            SetPositionCard();
        }
        
        void SetPositionCard()
        {
            if (m_ObjUserHand == null || m_ObjUserLeft == null)
                return;

            Vector3 userPosition = m_ObjUserHand.transform.position;
            foreach (ICardInfo cardInfo in m_ListHandCard)
            {
                cardInfo.SetPosition(userPosition);
                userPosition = new Vector3(userPosition.x + 10.0f, userPosition.y, userPosition.z);
            }

            userPosition = m_ObjUserLeft.transform.position;
            foreach (ICardInfo cardInfo in m_ListLeftCard)
            {
                cardInfo.SetPosition(userPosition);
                userPosition = new Vector3(userPosition.x + 10.0f, userPosition.y, userPosition.z);
            }
        }

        public virtual void RemoveUserHandCard(ICardInfo cardInfo)
        {
            //TODO : 카드 리소스 로딩하여 순서대로 추가
            if (m_ListHandCard.Contains(cardInfo))
            {
                m_ListHandCard.Remove(cardInfo);
                SetPositionCard();
            }
        }

        public virtual void LeftUserHandCard(ICardInfo cardInfo)
        {
            m_ListHandCard.Remove(cardInfo);
            m_ListLeftCard.Add(cardInfo);

            ICardInfoModify cardInfoModify = cardInfo as ICardInfoModify;
            if (cardInfoModify != null)
            {
                cardInfoModify.CardStatus = CardStatus.Open;
                cardInfoModify.CardPosition = CardPosition.Board;
            }
                
            SetPositionCard();
        }

        void Start()
        {
            BoardAgent.Instance.AddUserSlot(this);
        }

        void SetUserSlotStatus()
        {
            if (IsTurn)
            {
                //TODO : 내 차례 표시!!
                //활성화 처리
                Debug.Log("UserNumber " + UserNumber + " is Turn!!");
            }
            else
            {
                //TODO : 내 차례 아님 표시..
            }
        }
    }
}

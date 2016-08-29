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
    public class ComSlot : MonoBehaviour, IUserSlot
    {
        [SerializeField]
        GameObject m_ObjUserHand = null;

        [SerializeField]
        GameObject m_ObjUserLeft = null;

        [SerializeField]
        UILabel m_LblNick = null;

        [SerializeField]
        UILabel m_LblStatus = null;
        
        int m_UserNumber = 0;
        public int UserNumber
        {
            get { return m_UserNumber; }
            set { m_UserNumber = value; }
        }

        string m_UserName;
        public string UserName
        {
            get { return m_UserName; }
            set { m_UserName = value; m_LblNick.text = value; }
        }

        int m_UserState = 0;
        public int UserState
        {
            get { return m_UserState; }
            set { m_UserState = value; m_LblStatus.text = value.ToString(); }
        }

        public bool m_IsShield = false;
        public bool IsShield
        {
            get { return m_IsShield; }
            set { m_IsShield = value; }
        }

        bool m_IsDead = false;
        public bool IsDead
        {
            get { return m_IsDead; }
            set { m_IsDead = value; }
        }
        
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

        public virtual bool AddUserHandCard(ICardInfo cardInfo)
        {
            if(!m_ListHandCard.Contains(cardInfo))
            {
                m_ListHandCard.Add(cardInfo);

                ICardInfoModify cardInfoModify = cardInfo as ICardInfoModify;
                if (cardInfoModify != null)
                {
                    cardInfoModify.CardOwner = m_UserNumber;
                    cardInfoModify.CardStatus = CardStatus.Close;
                    cardInfoModify.CardPosition = CardPosition.Hand;
                }
                
                SetPositionCard();

                return true;
            }
            return false;
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

        public virtual bool RemoveUserHandCard(ICardInfo cardInfo)
        {
            //TODO : 카드 리소스 로딩하여 순서대로 추가
            if (m_ListHandCard.Contains(cardInfo))
            {
                m_ListHandCard.Remove(cardInfo);
                SetPositionCard();
                return true;
            }
            return false;
        }

        public virtual bool LeftUserHandCard(ICardInfo cardInfo)
        {
            if(m_ListHandCard.Contains(cardInfo))
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
                return true;
            }
            
            return false;
        }

        IEnumerator Start()
        {
            YieldInstruction nextFrame = new WaitForEndOfFrame();
            while (!BoardAgent.Instance.AddUserSlot(this))
                yield return nextFrame;
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

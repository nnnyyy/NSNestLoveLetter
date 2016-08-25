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
    public class UserSlot : MonoBehaviour
    {
        //유저 고유 번호
        //넘버링은 1부터..
        [Range(1,10)]
        public int UserNumber = 1;

        List<Card> m_ListCard = new List<Card>();

        bool m_IsMyTurn = false;
        public bool IsMyTurn
        {
            get
            {
                return m_IsMyTurn;
            }
            set
            {
                if (m_IsMyTurn == value)
                    return;

                m_IsMyTurn = value;
                SetUserSlotStatus();
            }
        }
        
        /// <summary>
        /// 유저 손에 카드가 들어옵니다.
        /// </summary>
        public void AddCard(ICardInfo cardInfo)
        {
            //TODO : 카드 리소스 로딩하여 순서대로 추가
        }

        /// <summary>
        /// 유저손에서 카드가 나갑니다.
        /// </summary>
        public void RemoveCard(ICardInfo cardInfo)
        {
            //TODO : 카드 리소스 로딩하여 순서대로 추가
        }

        void Start()
        {
            BoardAgent.Instance.AddUserSlot(this);
        }

        void SetUserSlotStatus()
        {
            if(IsMyTurn)
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

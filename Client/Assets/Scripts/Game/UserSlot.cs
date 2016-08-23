using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    //유저와 1:1로 매칭되는 카드 처리스크립트
    public class UserSlot : MonoBehaviour
    {
        //유저 고유 번호
        [Range(1,4)]
        public int UserNumber = 1;

        List<Card> m_ListCard = new List<Card>();

        public bool IsMyTurn = false;
        
        public void AddCard(CardType cardType)
        {
            //TODO : 카드 리소스 로딩하여 순서대로 추가
        }

        void Start()
        {
            BoardAgent.Instance.AddUserSlot(this);
        }

        void Update()
        {
            if(IsMyTurn)
            {
                //TODO : 내 차례 표시!!
                Debug.Log("UserNumber " + UserNumber + " is Turn!!");
            }
            else
            {
                //TODO : 내 차례 아님 표시..
            }
        }
    }
}

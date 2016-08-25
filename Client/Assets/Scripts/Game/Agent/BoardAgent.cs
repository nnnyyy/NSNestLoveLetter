using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    /// <summary>
    /// 게임을 진행하면서 카드 처리 결과를 담당 유저에게 전달합니다.
    /// 
    /// UserSlot의 이벤트와, Card의 이벤트를 받아
    /// 상황에 맞는 행동을 합니다.
    /// 
    /// Sender와 Receiver를 통해 정보를 보내고 처리합니다.
    /// </summary>
    public class BoardAgent : MonoBehaviour
    {
        static BoardAgent mInstance = null;
        static public BoardAgent Instance
        {
            get
            {
                if (mInstance == null)
                {
                    mInstance = DontDestroyObject.Create<BoardAgent>((delObj) =>
                    {
                        mInstance = null;
                    });
                    mInstance.Init();
                }

                return mInstance;
            }
        }

        /// <summary>
        /// 게임에 참여하는 유저의 수를 세팅합니다.
        /// </summary>
        int m_UserCount = 0;
        public void SetGameUserCount(int count)
        {
            m_UserCount = count;
        }
        
        public void Init()
        {
            m_ListUserSlot.Clear();
        }

        List<UserSlot> m_ListUserSlot = new List<UserSlot>();
        public void AddUserSlot(UserSlot userSlot)
        {
            if(!m_ListUserSlot.Contains(userSlot))
                m_ListUserSlot.Add(userSlot);
        }

        public UserSlot GetUserSlot(int userNum)
        {
            UserSlot result = null;
            foreach (UserSlot useSlot in m_ListUserSlot)
            {
                if(useSlot.UserNumber == userNum)
                {
                    result = useSlot;
                    break;
                }
            }

            return result;
        }

        int TurnTokenUserNumber = 0;
        void StartGame()
        {
            if(m_UserCount == m_ListUserSlot.Count)
            {
                TurnTokenUserNumber = 0;
                NextTurnProcess();
            }
            else
            {
                Debug.Log("게임에 맞는 유저 수가 세팅되지 않음.");
            }
        }

        void NextTurnProcess()
        {
            TurnTokenUserNumber++;
            if (TurnTokenUserNumber > m_UserCount)
                TurnTokenUserNumber = 1;

            foreach (UserSlot useSlot in m_ListUserSlot)
                useSlot.IsMyTurn = (useSlot.UserNumber == TurnTokenUserNumber);
        }
    }
}

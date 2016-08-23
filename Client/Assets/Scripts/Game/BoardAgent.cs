using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.Game
{
    /// <summary>
    /// 게임을 진행하면서 카드 처리 결과를 담당 유저에게 전달합니다.
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

        UserSlot GetUserSlot(int userNum)
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
            TurnTokenUserNumber = 0;
            NextTurnProcess();
        }

        void NextTurnProcess()
        {
            TurnTokenUserNumber++;
            if (TurnTokenUserNumber > 4)
                TurnTokenUserNumber = 1;

            foreach (UserSlot useSlot in m_ListUserSlot)
                useSlot.IsMyTurn = (useSlot.UserNumber == TurnTokenUserNumber);
        }
    }
}

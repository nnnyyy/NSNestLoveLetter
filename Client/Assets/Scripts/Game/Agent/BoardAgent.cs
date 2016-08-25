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
    public partial class BoardAgent : MonoBehaviour
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
            m_UserSlot = null;
            m_ListComSlot.Clear();
            AddReceiver();
        }
        
        /// <summary>
        /// 게임 시작 준비 완료 여부
        /// </summary>
        bool m_IsReadyRoom = false;
        public bool IsReady { get { return m_IsReadyRoom; } }

        /// <summary>
        /// 게임에 참여하는 유저의 수를 세팅합니다.
        /// </summary>
        int m_UserCount = 0;
        void SetGameUser(Dictionary<int, GCPRoomState.UserInfo> dicUserInfo)
        {
            m_UserCount = dicUserInfo.Count;

            int index = 0;
            foreach(GCPRoomState.UserInfo userInfo in dicUserInfo.Values)
            {
                if(m_ListComSlot.Count > index)
                {
                    IUserSlot userSlot = m_ListComSlot[index];
                    if(userSlot!=null)
                    {
                        userSlot.UserNumber = userInfo.sn;
                        userSlot.UserName = userInfo.nickName;
                        userSlot.UserState = (int)userInfo.readyState;
                    }
                }
            }
        }

        IUserSlot m_UserSlot = null;
        List<IUserSlot> m_ListComSlot = new List<IUserSlot>();
        public bool AddUserSlot(IUserSlot userSlot)
        {
            if (userSlot is UserSlot)
            {
                m_UserSlot = userSlot;
                m_UserSlot.UserName = GlobalData.Instance.userNickname;
                m_UserSlot.UserNumber = GlobalData.Instance.userSN;

                return true;
            }
            else if (!m_ListComSlot.Contains(userSlot))
            {
                m_ListComSlot.Add(userSlot);
                return true;
            }

            return false;
        }

        public IUserSlot GetUserSlot(int userNum)
        {
            if (m_UserSlot.UserNumber == userNum)
                return m_UserSlot;

            IUserSlot result = null;
            foreach (IUserSlot useSlot in m_ListComSlot)
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
            //STart
        }
        
        void SetTurn(int userNumber)
        {
            m_UserSlot.IsTurn = (m_UserSlot.UserNumber == userNumber);
            foreach (IUserSlot useSlot in m_ListComSlot)
                useSlot.IsTurn = (useSlot.UserNumber == userNumber);
        }

        void CreateCard(int userNumber)
        {
            //TODO : 카드를 만든후, 이벤트 연결 처리후, 특정 유저에게 줍니다.
        }
    }
}

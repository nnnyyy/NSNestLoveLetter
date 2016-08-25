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

        public void Init()
        {
            m_UserSlot = null;
            m_ListComSlot.Clear();
            AddReceiver();
        }
        
        void OnDestroy()
        {
            RemoveReceiver();
        }

        #region Recevier Event

        void AddReceiver()
        {
            Receiver.OnCreateRoomRetCallback += Receiver_OnCreateRoomRetCallback;
            Receiver.OnEnterRoomRetCallback += Receiver_OnEnterRoomRetCallback;
            Receiver.OnGameStartRetCallback += Receiver_OnGameStartRetCallback;
            Receiver.OnLeaveRoomRetCallback += Receiver_OnLeaveRoomRetCallback;
            Receiver.OnLLActionRetCallback += Receiver_OnLLActionRetCallback;
            Receiver.OnLLFinalResultCallback += Receiver_OnLLFinalResultCallback;
            Receiver.OnLLRoundResultCallback += Receiver_OnLLRoundResultCallback;
            Receiver.OnLLStatusCallback += Receiver_OnLLStatusCallback;
            Receiver.OnLoginRetCallback += Receiver_OnLoginRetCallback;
            Receiver.OnRoomListRetCallback += Receiver_OnRoomListRetCallback;
            Receiver.OnRoomStateCallback += Receiver_OnRoomStateCallback;
        }

        void RemoveReceiver()
        {
            Receiver.OnCreateRoomRetCallback -= Receiver_OnCreateRoomRetCallback;
            Receiver.OnEnterRoomRetCallback -= Receiver_OnEnterRoomRetCallback;
            Receiver.OnGameStartRetCallback -= Receiver_OnGameStartRetCallback;
            Receiver.OnLeaveRoomRetCallback -= Receiver_OnLeaveRoomRetCallback;
            Receiver.OnLLActionRetCallback -= Receiver_OnLLActionRetCallback;
            Receiver.OnLLFinalResultCallback -= Receiver_OnLLFinalResultCallback;
            Receiver.OnLLRoundResultCallback -= Receiver_OnLLRoundResultCallback;
            Receiver.OnLLStatusCallback -= Receiver_OnLLStatusCallback;
            Receiver.OnLoginRetCallback -= Receiver_OnLoginRetCallback;
            Receiver.OnRoomListRetCallback -= Receiver_OnRoomListRetCallback;
            Receiver.OnRoomStateCallback -= Receiver_OnRoomStateCallback;
        }
        
        private void Receiver_OnRoomStateCallback(GCPRoomState roomState)
        {
            int readyCount = 0;
            int index = 0;
            foreach (GCPRoomState.UserInfo userInfo in roomState.listUsers)
            {
                if(userInfo.sn == GlobalData.Instance.userSN)
                {
                    m_UserSlot.UserName = userInfo.nickName;
                    m_UserSlot.UserNumber = userInfo.sn;
                    m_UserSlot.UserState = userInfo.readyState;
                }
                else
                {
                    if(m_ListComSlot.Count > index)
                    {
                        IUserSlot userSlot = m_ListComSlot[index];
                        if(userSlot!=null)
                        {
                            userSlot.UserName = userInfo.nickName;
                            userSlot.UserNumber = userInfo.sn;
                            userSlot.UserState = userInfo.readyState;
                        }
                    }
                    index++;
                }
                if (userInfo.readyState == 1)
                    readyCount++;
            }

            m_IsReadyRoom = (readyCount == roomState.listUsers.Count);
        }

        private void Receiver_OnRoomListRetCallback(GCPRoomListRet roomListRet)
        {
            throw new System.NotImplementedException();
        }

        private void Receiver_OnLoginRetCallback(GCPLoginRet loginRet)
        {
            throw new System.NotImplementedException();
        }

        private void Receiver_OnLLStatusCallback(GCPLLStatus llStatus)
        {
            int index = 0;
            foreach(GCPLLStatus.PlayerInfo info in llStatus.listPlayer)
            {
                if(info.userSN == GlobalData.Instance.userSN)
                {
                    m_UserSlot.IsShield = info.shieldState==1 ? true : false;
                    m_UserSlot.IsDead = info.deadState==1 ? true : false;
                }
                else
                {
                    if (m_ListComSlot.Count > index)
                    {
                        IUserSlot userSlot = m_ListComSlot[index];
                        if(userSlot!=null)
                        {
                            userSlot.IsShield = info.shieldState == 1 ? true : false;
                            userSlot.IsDead = info.deadState == 1 ? true : false;
                        }
                    }
                    index++;
                }
            }
            
            SetTurn(llStatus.currentTurnUserIndex);
        }

        private void Receiver_OnLLRoundResultCallback(GCPLLRoundResult llRoundResult)
        {
            throw new System.NotImplementedException();
        }

        private void Receiver_OnLLFinalResultCallback(GCPLLFinalResult llFinalResult)
        {
            throw new System.NotImplementedException();
        }

        private void Receiver_OnLLActionRetCallback(GCPLLActionRet llActionRet)
        {
            throw new System.NotImplementedException();
        }

        private void Receiver_OnLeaveRoomRetCallback(GCPLeaveRoomRet leaveRoomRet)
        {
            throw new System.NotImplementedException();
        }

        private void Receiver_OnGameStartRetCallback(GCPGameStartRet gameStartRet)
        {
            throw new System.NotImplementedException();
        }

        private void Receiver_OnEnterRoomRetCallback(GCPEnterRoomRet enterRoomRet)
        {
            m_IsReadyRoom = false;
            m_UserSlot = null;
            m_ListComSlot.Clear();
        }

        private void Receiver_OnCreateRoomRetCallback(GCPCreateRoomRet createRoomRet)
        {
            throw new System.NotImplementedException();
        }

        #endregion
        
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
                m_UserSlot.UserName = GlobalData.Instance.UserNickname;
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

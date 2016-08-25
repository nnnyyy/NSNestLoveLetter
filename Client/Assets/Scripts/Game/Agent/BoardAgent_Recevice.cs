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
            
        }

        private void Receiver_OnLoginRetCallback(GCPLoginRet loginRet)
        {
            
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
            
        }

        private void Receiver_OnLLFinalResultCallback(GCPLLFinalResult llFinalResult)
        {
            
        }

        private void Receiver_OnLLActionRetCallback(GCPLLActionRet llActionRet)
        {
            
        }

        private void Receiver_OnLeaveRoomRetCallback(GCPLeaveRoomRet leaveRoomRet)
        {
            
        }

        private void Receiver_OnGameStartRetCallback(GCPGameStartRet gameStartRet)
        {
            
        }

        private void Receiver_OnEnterRoomRetCallback(GCPEnterRoomRet enterRoomRet)
        {
            m_IsReadyRoom = false;
            m_UserSlot = null;
            m_ListComSlot.Clear();
        }

        private void Receiver_OnCreateRoomRetCallback(GCPCreateRoomRet createRoomRet)
        {
            
        }

        #endregion
    }
}

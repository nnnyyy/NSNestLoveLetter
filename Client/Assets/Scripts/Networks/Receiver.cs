using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;

namespace NSNetwork
{
    public class Receiver
    {
        public static void OnReceive(Packet packet)
        {
            byte[] data = packet.GetData();
            eGCP type = (eGCP)BitConverter.ToInt16(data, 0);

            switch (type)
            {
                case eGCP.GCP_LoginRet:         Receiver.OnLogin(new GCPLoginRet(data));                    break;
                case eGCP.GCP_RoomListRet:      Receiver.OnRoomListRet(new GCPRoomListRet(data));           break;
                case eGCP.GCP_CreateRoomRet:    Receiver.OnCreateRoomRet(new GCPCreateRoomRet(data));       break;
                case eGCP.GCP_EnterRoomRet:     Receiver.OnEnterRoomRet(new GCPEnterRoomRet(data));         break;
                case eGCP.GCP_LeaveRoomRet:     Receiver.OnLeaveRoomRet(new GCPLeaveRoomRet(data));         break;
                case eGCP.GCP_RoomState:        Receiver.OnRoomState(new GCPRoomState(data));               break;
                case eGCP.GCP_GameStartRet:     Receiver.OnGameStartRet(new GCPGameStartRet(data));         break;
                case eGCP.GCP_GameLoveLetter:   Receiver.OnReceiveGameLoveLetter(packet);                   break;
            }
        }

        static void OnReceiveGameLoveLetter(Packet packet)
        {
            byte[] data = packet.GetData();
            eGCP type = (eGCP)BitConverter.ToInt16(data, 0);
            if (type != eGCP.GCP_GameLoveLetter)
                return;

            eGCP_LoveLetter typeLL = (eGCP_LoveLetter)BitConverter.ToInt16(data, sizeof(short));

            switch (typeLL)
            {
                case eGCP_LoveLetter.GCP_LL_Status:         Receiver.OnLLStatus(new GCPLLStatus(data));             break;
                case eGCP_LoveLetter.GCP_LL_ActionRet:      Receiver.OnLLActionRet(new GCPLLActionRet(data));       break;
                case eGCP_LoveLetter.GCP_LL_RoundResult:    Receiver.OnLLRoundResult(new GCPLLRoundResult(data));   break;
                case eGCP_LoveLetter.GCP_LL_FinalResult:    Receiver.OnLLFinalResult(new GCPLLFinalResult(data));   break;
            }
        }

        public delegate void LoginRetDelegate(GCPLoginRet loginRet);
        public static LoginRetDelegate onLoginRetCallback;

        static void OnLogin(GCPLoginRet packet)
        {
            Debug.Log("### OnLogin ###");
            if (packet.result == GCPLoginRet.eResult.Success)
            {
                GlobalData.Instance.UserNickname = packet.nickName;
                GlobalData.Instance.userSN = packet.sn;
            }

            if (onLoginRetCallback != null)
                onLoginRetCallback(packet);
        }

        public delegate void RoomListRetDelegate(GCPRoomListRet roomListRet);
        public static RoomListRetDelegate onRoomListRetCallback;

        static void OnRoomListRet(GCPRoomListRet packet)
        {
            Debug.Log("### OnRoomListRet ###");

            if (onRoomListRetCallback != null)
                onRoomListRetCallback(packet);
        }

        public delegate void CreateRoomRetDelegate(GCPCreateRoomRet createRoomRet);
        public static CreateRoomRetDelegate onCreateRoomRetCallback;

        static void OnCreateRoomRet(GCPCreateRoomRet packet)
        {
            Debug.Log("### OnCreateRoomRet ###");
            if (packet.result == GCPCreateRoomRet.eReuslt.Success)
            {
                GlobalData.Instance.roomSN = packet.sn;
            }

            if (onCreateRoomRetCallback != null)
                onCreateRoomRetCallback(packet);
        }

        public delegate void EnterRoomRetDelegate(GCPEnterRoomRet enterRoomRet);
        public static EnterRoomRetDelegate onEnterRoomRetCallback;

        static void OnEnterRoomRet(GCPEnterRoomRet packet)
        {
            Debug.Log("### OnEnterRoomRet ###");
            if (packet.result == GCPEnterRoomRet.eResult.Success)
            {
                GlobalData.Instance.roomSN = packet.sn;
            }

            if (onEnterRoomRetCallback != null)
                onEnterRoomRetCallback(packet);
        }

        public delegate void LeaveRoomRetDelegate(GCPLeaveRoomRet leaveRoomRet);
        public static LeaveRoomRetDelegate onLeaveRoomRetCallback;

        static void OnLeaveRoomRet(GCPLeaveRoomRet packet)
        {
            Debug.Log("### LeaveRoomRet ###");
            GlobalData.Instance.roomSN = 0;

            if (onLeaveRoomRetCallback != null)
                onLeaveRoomRetCallback(packet);
        }

        public delegate void RoomStateDelegate(GCPRoomState roomState);
        public static RoomStateDelegate onRoomStateCallback;

        static void OnRoomState(GCPRoomState packet)
        {
            Debug.Log("### OnRoomState ###");
            GlobalData.Instance.roomUsers = packet.listUsers;

            if (onRoomStateCallback != null)
                onRoomStateCallback(packet);
        }

        public delegate void GameStartRestDelegate(GCPGameStartRet gameStartRet);
        public static GameStartRestDelegate onGameStartRetCallback;

        static void OnGameStartRet(GCPGameStartRet packet)
        {
            Debug.Log("### OnGameStartRet ###");
            if (onGameStartRetCallback != null)
                onGameStartRetCallback(packet);
        }


        // Game Love Letter
        public delegate void LLStatusDelegate(GCPLLStatus llStatus);
        public static LLStatusDelegate onLLStatusCallback;

        static void OnLLStatus(GCPLLStatus packet)
        {
            Debug.Log("### OnLLStatus ###");
            if (onLLStatusCallback != null)
                onLLStatusCallback(packet);
        }

        public delegate void LLActionRetDelegate(GCPLLActionRet llActionRet);
        public static LLActionRetDelegate onLLActionRetCallback;

        static void OnLLActionRet(GCPLLActionRet packet)
        {
            Debug.Log("### OnLLActionRet ###");
            if (onLLActionRetCallback != null)
                onLLActionRetCallback(packet);
        }

        public delegate void LLRoundResultDelegate(GCPLLRoundResult llRoundResult);
        public static LLRoundResultDelegate onLLRoundResultCallback;

        static void OnLLRoundResult(GCPLLRoundResult packet)
        {
            Debug.Log("### OnLLRoundResult ###");
            if (onLLRoundResultCallback != null)
                onLLRoundResultCallback(packet);
        }

        public delegate void LLFinalResultDelagate(GCPLLFinalResult llFinalResult);
        public static LLFinalResultDelagate onLLFinalResultCallback;

        static void OnLLFinalResult(GCPLLFinalResult packet)
        {
            Debug.Log("### OnLLFinalResult ###");
            if (onLLFinalResultCallback != null)
                onLLFinalResultCallback(packet);
        }
    }
}
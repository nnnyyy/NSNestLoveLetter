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
            }
        }

        public delegate void LoginRetDelegate(GCPLoginRet.eResult result);
        public static LoginRetDelegate onLoginRetCallback;

        static void OnLogin(GCPLoginRet packet)
        {
            switch (packet.result)
            {
                case GCPLoginRet.eResult.Success:              Debug.Log("Login Success..");                   break;
                case GCPLoginRet.eResult.ErrorIdOrPassword:    Debug.Log("Error Id Or Password..");            break;
                case GCPLoginRet.eResult.NotJoined:            Debug.Log("Not Joinedg..");                     break;
                case GCPLoginRet.eResult.NowConneting:         Debug.Log("Now Conneting..");                   break;
            }

            if (onLoginRetCallback != null)
                onLoginRetCallback(packet.result);
        }

        public delegate void RoomListRetDelegate(GCPRoomListRet roomListRet);
        public static RoomListRetDelegate onRoomListRetCallback;

        static void OnRoomListRet(GCPRoomListRet packet)
        {
            Debug.Log("### OnRoomListRet ###");
            for( int i = 0; i < packet.listRooms.Count; ++i )
            {
                Debug.Log("Room SN >> " + packet.listRooms[i].sn + ", Count >> " + (int)(packet.listRooms[i].userCount));
            }

            if (onRoomListRetCallback != null)
                onRoomListRetCallback(packet);
        }

        public delegate void CreateRoomRetDelegate(GCPCreateRoomRet.eReuslt result);
        public static CreateRoomRetDelegate onCreateRoomRetCallback;

        static void OnCreateRoomRet(GCPCreateRoomRet packet)
        {
            Debug.Log("### OnCreateRoomRet ###");
            if (packet.result == GCPCreateRoomRet.eReuslt.Success)
                Debug.Log("Create Success >> " + packet.sn);
            else
                Debug.Log("Create Fail..");

            if (onCreateRoomRetCallback != null)
                onCreateRoomRetCallback(packet.result);
        }

        public delegate void EnterRoomRetDelegate(GCPEnterRoomRet enterRoomRet);
        public static EnterRoomRetDelegate onEnterRoomRetCallback;

        static void OnEnterRoomRet(GCPEnterRoomRet packet)
        {
            Debug.Log("### OnEnterRoomRet ###");
            Debug.Log(packet.result);
            Debug.Log(packet.sn);

            if (onEnterRoomRetCallback != null)
                onEnterRoomRetCallback(packet);
        }

        public delegate void LeaveRoomRetDelegate(GCPLeaveRoomRet leaveRoomRet);
        public static LeaveRoomRetDelegate onLeaveRoomRetCallback;

        static void OnLeaveRoomRet(GCPLeaveRoomRet packet)
        {
            Debug.Log("### LeaveRoomRet ###");
            Debug.Log(packet.sn);

            if (onLeaveRoomRetCallback != null)
                onLeaveRoomRetCallback(packet);
        }

        public delegate void RoomStateDelegate(GCPRoomState roomState);
        public static RoomStateDelegate onRoomStateCallback;

        static void OnRoomState(GCPRoomState packet)
        {
            Debug.Log("### OnRoomState ###");
            Debug.Log(packet.flag);
            for( int i = 0; i < packet.listUsers.Count; ++ i)
            {
                Debug.Log( "sn >> " + packet.listUsers[i].sn + ", ready >> " + (int)(packet.listUsers[i].readyState) );
            }

            if (onRoomStateCallback != null)
                onRoomStateCallback(packet);
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
    }
}
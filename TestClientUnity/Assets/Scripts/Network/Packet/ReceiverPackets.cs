﻿using UnityEngine;
using System;
using System.Collections;
using System.Collections.Generic;


namespace NSNetwork
{
    public class GCPLoginRet : ReceivePacket
    {
        public enum eResult
        {
            Success = 0,
            ErrorIdOrPassword = -1,
            NotJoined = -2,
            NowConneting = -3,
        }

        public eResult result;
        public int sn = 0;
        public string nickName = "";

        public GCPLoginRet(byte[] data)
        {
            type = (eGCP)GetShort(data);
            result = (eResult)GetShort(data);
            if(result == eResult.Success)
            {
                sn = GetInt(data);
                nickName = GetString(data);
            }
        }
    }

    public class GCPRegisterUserRet : ReceivePacket
    {
        public enum eReuslt
        {
            Success = 0,
            AlreadyRegisted = -1,
            Error = -2
        }

        public eReuslt result;
        public GCPRegisterUserRet(byte[] data)
        {
            type = (eGCP)GetShort(data);
            result = (eReuslt)GetShort(data);
        }
    }

    public class GCPRoomListRet : ReceivePacket
    {
        public class RoomInfo
        {
            public int sn = 0;
            public char userCount = (char)0;
        }

        public List<RoomInfo> listRooms = new List<RoomInfo>();

        public GCPRoomListRet(byte[] data)
        {
            type = (eGCP)GetShort(data);
            int count = GetInt(data);
            for( int i = 0; i < count; ++ i )
            {
                RoomInfo room = new RoomInfo();
                room.sn = GetInt(data);
                room.userCount = GetChar(data);
                listRooms.Add(room);
            }
        }
    }

    public class GCPCreateRoomRet : ReceivePacket
    {
        public enum eReuslt
        {
            Success = 0,
            Fail = -1,
        }

        public eReuslt result;
        public int sn;

        public GCPCreateRoomRet(byte[] data)
        {
            type = (eGCP)GetShort(data);
            result = (eReuslt)GetShort(data);

            if(result == eReuslt.Success)
                sn = GetInt(data);
        }
    }

    public class GCPEnterRoomRet : ReceivePacket
    {
        public enum eResult
        {
            Success = 0,
            Fail = -1,
        }

        public eResult result;
        public int sn = 0;

        public GCPEnterRoomRet(byte[] data)
        {
            type = (eGCP)GetShort(data);
            result = (eResult)GetShort(data);

            if(result == eResult.Success)
                sn = GetInt(data);
        }
    }

    public class GCPLeaveRoomRet : ReceivePacket
    {
        public int sn;
        public GCPLeaveRoomRet(byte[] data)
        {
            type = (eGCP)GetShort(data);
            sn = GetInt(data);
        }
    }

    public class GCPRoomState : ReceivePacket
    {
        public static readonly UInt32 FLAG_READY = 0x01;
        public static readonly UInt32 FLAG_WITHOUT_ROOM_MASTER = 0x7fffffff;
        public static readonly UInt32 FLAG_ROOM_MASTER = 0x80000000;
        public static readonly UInt32 FLAG_ALL = 0xffffffff;

        public enum eFlagType
        {
            Ready = 0,
            RoomMaster,
            UserInfos,
            All
        }

        public class UserInfo
        {
            public int sn = 0;
            public string nickName = "";
            public char readyState = (char)0;
        }

        public int flag;        
        public int masterSN;
        public List<UserInfo> listUsers = new List<UserInfo>();

        public GCPRoomState(byte[] data)
        {
            type = (eGCP)GetShort(data);
            flag = GetInt(data);            

            if ( (flag & FLAG_ROOM_MASTER) != 0 )
            {                
                masterSN = GetInt(data);
            }

            if( (flag & FLAG_WITHOUT_ROOM_MASTER) != 0 )
            {
                int userCount = (int)GetChar(data);
                for (int i = 0; i < userCount; ++i)
                {
                    UserInfo user = new UserInfo();
                    user.sn = GetInt(data);
                    user.nickName = GetString(data);
                    user.readyState = GetChar(data);
                    listUsers.Add(user);
                }
            }
        }
    }

    public class GCPGameStartRet : ReceivePacket
    {
        public enum eResult
        {
            Success = 0,
            NotEnoughUser = -1,
            NotRoomMaster = -2,
            NotAllReady = -3
        }

        public eResult result;
        public Dictionary<int, int> dSN_to_GameIdx;
        public Dictionary<int, int> dGameIdx_to_SN;

        public GCPGameStartRet(byte[] data)
        {
            type = (eGCP)GetShort(data);
            result = (eResult)GetInt(data);
            dSN_to_GameIdx = new Dictionary<int, int>();
            dGameIdx_to_SN = new Dictionary<int, int>();
            if(result == 0)
            {
                int nCnt = GetInt(data);
                for(int i = 0; i < nCnt; ++i)
                {
                    int nUserSN = GetInt(data);
                    int nGameIdx = GetInt(data);
                    dSN_to_GameIdx.Add(nUserSN, nGameIdx);
                    dGameIdx_to_SN.Add(nGameIdx, nUserSN);
                }                
            }
        }
    }

    // Game Love Letter
    public class GCPLLInitStatus : ReceivePacket
    {
        public class PlayerInfo
        {
            public int userSN;
            public int deadState;
            public int shieldState;
            public List<int> listGroundCards = new List<int>();
            public List<int> listHandCards = new List<int>();
            public bool bMyTurn;
        }

        public int currentTurnUserIndex;
        public int currentTurnUserGetCardIndex;
        public List<PlayerInfo> listPlayer = new List<PlayerInfo>();

        public GCPLLInitStatus(byte[] data)
        {
            listPlayer.Clear();
            type = (eGCP)GetShort(data);
            eGCP_LoveLetter llType = (eGCP_LoveLetter)GetShort(data);
            currentTurnUserIndex = GetInt(data);
            currentTurnUserGetCardIndex = GetInt(data);
            int userCount = GetInt(data);

            PlayerInfo local = null;
            for( int i = 0; i < userCount; ++i )
            {
                PlayerInfo player = new PlayerInfo();
                player.userSN = GetInt(data);
                player.deadState = GetInt(data);
                player.shieldState = GetInt(data);
                int cardCount = GetInt(data);

                player.listGroundCards.Clear();
                for( int j = 0; j < cardCount; ++j )
                {
                    player.listGroundCards.Add(GetInt(data));
                }
                listPlayer.Add(player);              
                
                if(GlobalData.Instance.userSN == player.userSN)
                {
                    local = player;
                }  
            }
                        
            local.listHandCards.Add(GetInt(data));
        }
    }

    public class GCPLLStatus : ReceivePacket
    {
        public int currentTurnUserIndex;
        public int currentTurnUserGetCardIndex;        

        public GCPLLStatus(byte[] data)
        {            
            type = (eGCP)GetShort(data);
            eGCP_LoveLetter llType = (eGCP_LoveLetter)GetShort(data);
            currentTurnUserIndex = GetInt(data);
            currentTurnUserGetCardIndex = GetInt(data);         
        }
    }

    public class GCPLLActionRet : ReceivePacket
    {
        public int nCardType;
        public int nSrcIdx;
        public int nTargetIdx;
        public int nCardIdx;
        public bool bSucceed;
        public bool bMyTurn;
        public int nRet;
        public int nDeadCardIdx;
        public int nDropCardIdx;
        public bool bTargetPlayer;
        public int nNewCard;
        public int nSrcToTargetCardIdx;
        public int nTargetToSrcCardIdx;
        public bool bSrcOrTarget;
        public GCPLLActionRet(byte[] data)
        {
            Debug.Log( "### GCPLLActionRet ###" );
            type = (eGCP)GetShort(data);
            eGCP_LoveLetter llType = (eGCP_LoveLetter)GetShort(data);
            nCardType = GetInt(data);
            switch (nCardType)
            {
                case 1:
                    nSrcIdx = GetInt(data);
                    nTargetIdx = GetInt(data);
                    nCardIdx = GetInt(data);
                    bSucceed = GetInt(data) == 1 ? true : false;
                    break;

                case 2:
                    nSrcIdx = GetInt(data);
                    nTargetIdx = GetInt(data);
                    bMyTurn = GetInt(data) == 1 ? true : false;
                    if (bMyTurn)
                    {
                        nCardIdx = GetInt(data);
                    }
                    break;

                case 3:
                    nSrcIdx = GetInt(data);
                    nTargetIdx = GetInt(data);
                    nRet = GetInt(data);
                    if(nRet != 0)
                    {
                        nDeadCardIdx = GetInt(data);
                    }
                    break;

                case 4:
                    nSrcIdx = GetInt(data);
                    break;

                case 5:
                    nSrcIdx = GetInt(data);
                    nTargetIdx = GetInt(data);
                    nDropCardIdx = GetInt(data);
                    bTargetPlayer = GetInt(data) == 1 ? true : false;
                    if (bTargetPlayer)
                    {
                        nNewCard = GetInt(data);
                    }
                    break;

                case 6:
                    nSrcIdx = GetInt(data);
                    nTargetIdx = GetInt(data);
                    bSrcOrTarget = GetInt(data) == 1 ? true : false;
                    if (bSrcOrTarget)
                    {
                        nSrcToTargetCardIdx = GetInt(data);
                        nTargetToSrcCardIdx = GetInt(data);
                    }
                    break;

                case 7:
                    nSrcIdx = GetInt(data);
                    break;                    
            }
        }
    }

    public class GCPLLRoundResult : ReceivePacket
    {
        public int winUserIndex;

        public GCPLLRoundResult(byte[] data)
        {
            type = (eGCP)GetShort(data);
            eGCP_LoveLetter llType = (eGCP_LoveLetter)GetShort(data);
            winUserIndex = GetInt(data);
        }
    }

    public class GCPLLFinalResult : ReceivePacket
    {
        public int winUserIndex;
        public GCPLLFinalResult(byte[] data)
        {
            type = (eGCP)GetShort(data);
            eGCP_LoveLetter lltype = (eGCP_LoveLetter)GetShort(data);
            winUserIndex = GetInt(data);
        }
    }

}
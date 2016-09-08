﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;

public class GlobalData : Singleton<GlobalData>
{
    public readonly string P_ID = "UserID";
    public readonly string P_PW = "UserPW";
    public readonly string P_TUTORIAL1 = "Tuto1";

    public int userSN = 0;
    public string userNickname = "";
    public int cntWin = 0;
    public int cntLose = 0;
    public int roomSN = 0;
    public int roomMasterSN = 0;

    public List<GCPRoomState.UserInfo> roomUsers;    


    public bool IsInRoom() { return (roomSN > 0); }
    public bool IsRoomMaster(int _SN) { return (roomMasterSN == _SN); }

    public void ClearData()
    {
        userSN = -1;
        userNickname = "";
        cntWin = 0;
        cntLose = 0;
        roomSN = 0;
        roomMasterSN = 0;
        if (roomUsers != null) {
            roomUsers.Clear();
            roomUsers = null;
        }
        
    }    
}

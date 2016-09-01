using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;

public class GlobalData : Singleton<GlobalData>
{    

    public int userSN = 0;
    public string userNickname = "";
    public int cntWin = 0;
    public int cntLose = 0;
    public int roomSN = 0;
    public int roomMasterSN = 0;

    public List<GCPRoomState.UserInfo> roomUsers;    


    public bool IsInRoom() { return (roomSN > 0); }
    public bool IsRoomMaster() { return (roomMasterSN == userSN); }

    public void ClearData()
    {
        userSN = 0;
        userNickname = "";
        cntWin = 0;
        cntLose = 0;
        roomSN = 0;
        roomMasterSN = 0;

        roomUsers = null;
    }    
}

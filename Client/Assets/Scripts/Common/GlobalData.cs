using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNetwork;

public class GlobalData : MonoBehaviour
{
    static GlobalData mInstance = null;
    static public GlobalData Instance
    {
        get
        {
            if (mInstance == null)
            {
                mInstance = NSNest.Common.DontDestroyObject.Create<GlobalData>((delObj) =>
                {
                    mInstance = null;
                });
            }

            return mInstance;
        }
    }

    public int userSN = 0;
    public string UserNickname = "";
    public int cntWin = 0;
    public int cntLose = 0;
    public int roomSN = 0;

    public List<GCPRoomState.UserInfo> roomUsers;


    public bool IsInRoom() { return (roomSN > 0); }
}

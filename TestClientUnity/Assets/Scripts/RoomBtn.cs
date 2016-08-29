using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class RoomBtn : MonoBehaviour {

    public Text m_lbRoomInfo;
    private int m_nRoomIndex = -1;
    private int m_nUserCnt = 0;
    public int roomIdx { get { return m_nRoomIndex; } }
    public int userCnt { get { return m_nUserCnt; } }

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public void SetRoomInfo(int _roomIdx, int _userCnt)
    {
        m_nRoomIndex = _roomIdx;
        m_nUserCnt = _userCnt;
        m_lbRoomInfo.text = "Room Index : " + m_nRoomIndex + ", UserCnt : " + m_nUserCnt;
    }

    public void OnBtnEnterRoom()
    {
        Debug.Log("OnBtnEnterRoom - " + m_nRoomIndex);
    }
}

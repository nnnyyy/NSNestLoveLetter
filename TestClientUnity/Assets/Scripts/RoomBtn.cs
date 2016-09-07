using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using NSNetwork;

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
        m_lbRoomInfo.text = "Room <color=yellow>" + m_nRoomIndex + "</color> | User Count <color=yellow>" + m_nUserCnt + "</color>";
    }

    public void OnBtnEnterRoom()
    {
        SoundManager.Instance.PlaySfx("btnNormal");
        Debug.Log("OnBtnEnterRoom - " + m_nRoomIndex);
        Sender.EnterRoom(m_nRoomIndex);               
    }
}

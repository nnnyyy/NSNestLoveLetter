using UnityEngine;
using System.Collections;
using DG.Tweening;
using System.Collections.Generic;
using NSNetwork;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class Game : MonoBehaviour {

    delegate void touchListener(int nType, int nID, float x, float y, float dx, float dy);
    event touchListener begin0;
    event touchListener move0;
    event touchListener end0;
    Vector2[] touchDelta = new Vector2[1];

    static public Transform s_tfBaseForConv;
    static public Transform s_tfRoot;
    static public Vector3 ConvPosToRootLocal(Vector3 vSrcPos)
    {
        s_tfBaseForConv.position = vSrcPos;
        return s_tfBaseForConv.localPosition;
    }

    static public Vector3 ConvPosToRootGlobal(Vector3 vSrcPos)
    {
        s_tfBaseForConv.position = vSrcPos;
        return s_tfBaseForConv.position;
    }

    public UserLocalInfo m_LocalUser;
    public UserRemoteInfo[] m_aRemoteUsers;
    public Transform tfTestBase;
    public Transform tfBase;
    public Transform tfGrave;

    //  게임 관련
    bool isGameRunning = false;
    public List<GameUser> m_aUser;
    public Dictionary<int, GameUser> m_mUser;

    public Text lbTitle;

    // Use this for initialization
    void Start () {
        Receiver.OnRoomStateCallback += OnRoomState;
        begin0 += OnTouch;        
        move0 += OnTouch;
        end0 += OnTouch;
        s_tfBaseForConv = tfTestBase;
        s_tfRoot = tfBase;
        CardManager.Init();
        m_aUser = new List<GameUser>();
        m_mUser = new Dictionary<int, GameUser>();
        lbTitle.text = "Room : " + GlobalData.Instance.roomSN;
        Refresh();        
    }

    // Update is called once per frame
    void Update () {
        //  Define 걸어서 나누자
        bool bMouseClicked = Input.GetMouseButtonDown(0);
        if (bMouseClicked)
        {
            Vector2 pos = Input.mousePosition;            
            if (begin0 != null) begin0(0, 0, pos.x, pos.y, 0, 0);            
        }

        int cnt = Input.touchCount;
        for(int i = 0; i < cnt; ++i)
        {
            Touch touch = Input.GetTouch(i);
            int id = touch.fingerId;
            Vector2 pos = touch.position;
            if (touch.phase == TouchPhase.Began) touchDelta[id] = touch.position;

            float x, y, dx = 0, dy = 0;
            x = pos.x;
            y = pos.y;
            if(touch.phase != TouchPhase.Began)
            {
                dx = x - touchDelta[id].x;
                dy = y - touchDelta[id].y;
            }

            if (touch.phase == TouchPhase.Began)
            {
                switch (id)
                {
                    case 0:
                        if (begin0 != null) begin0(0, id, x, y, dx, dy);
                        break;
                }                
            }
        }        	
	}

    public void OnTouch(int nType, int nID, float x, float y, float dx, float dy)
    {
        Collider2D coll;
        Vector3 ray = Camera.main.ScreenToWorldPoint(new Vector3(x, y, 1));
        if (coll = Physics2D.OverlapPoint(new Vector2(ray.x, ray.y), 0x300))
        {
            if(coll.gameObject.CompareTag("Card"))
            {
                Debug.Log("Card Processing");
            }
            coll.transform.SendMessage("Selected");
        }
    }

    public void OnBtnReadyOrStart()
    {

    }

    public void OnBtnLeave()
    {
        Receiver.OnLeaveRoomRetCallback += OnLeaveRoomRet;
        Sender.LeaveRoom();        
    }

    public void OnLeaveRoomRet(GCPLeaveRoomRet leaveRoomRet)
    {
        SceneManager.LoadScene("Lobby");
    }

    void Refresh()
    {
        ClearUI();

        if (GlobalData.Instance.roomUsers == null) return;
        int idx = 0;
        foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
        {
            UserInfoBase uib;
            if (u.sn == GlobalData.Instance.userSN)
            {
                uib = m_LocalUser;
            }
            else
            {
                uib = m_aRemoteUsers[idx];
                idx++;
            }
            
            uib.m_lbName.text = u.nickName;
            if(GlobalData.Instance.roomMasterSN == u.sn)
            {
                uib.m_lbReadyState.text = "Master";
            }
            else
            {
                uib.m_lbReadyState.text = u.readyState == 1 ? "Ready" : "Not Ready";
            }                
            
            
        }
    }

    void ClearUI()
    {
        m_LocalUser.ClearInfo();
        foreach (UserInfoBase uib in m_aRemoteUsers)
        {
            uib.ClearInfo();
        }
    }

    public void OnRoomState(GCPRoomState roomState)
    {        
    }
}
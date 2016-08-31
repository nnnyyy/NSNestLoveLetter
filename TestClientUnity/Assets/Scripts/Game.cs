using UnityEngine;
using System.Collections;
using DG.Tweening;
using System.Collections.Generic;
using NSNetwork;

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
        if (nType == 0)
        {
            Card c = CardManager.CreateCard(Card.SizeType.LOCAL_HAND, 1);
            m_LocalUser.PutHand(c);
        }
    }

    static int userSN = 0;
    public void AddLocalUser()
    {
        
        if( m_aUser.Count >= 4 )
        {
            Debug.Log("Error Add Local User - Already Max");
            return;
        }
        GameUser newUser = new GameUser();
        newUser.m_nUserSN = userSN++;
        newUser.m_sName = "김삼돌";
        m_aUser.Add(newUser);
        m_mUser.Add(newUser.m_nUserSN, newUser);
        Refresh();
    }

    void Refresh()
    {
        ClearUI();

        int idx = 0;
        foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
        {
            if (u.sn == GlobalData.Instance.userSN)
            {
                m_LocalUser.m_lbName.text = u.nickName;
                if(GlobalData.Instance.roomMasterSN == u.sn)
                {
                    m_LocalUser.m_lbReadyState.text = "Master";
                }
                else
                {
                    m_LocalUser.m_lbReadyState.text = u.readyState == 1 ? "Ready" : "Not Ready";
                }                
            }
            else
            {
                m_aRemoteUsers[idx].m_lbName.text = u.nickName;
                m_aRemoteUsers[idx].m_lbReadyState.text = u.readyState == 1 ? "Ready" : "Not Ready";
                idx++;
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
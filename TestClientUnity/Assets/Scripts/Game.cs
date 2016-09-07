using UnityEngine;
using System.Collections;
using DG.Tweening;
using System.Collections.Generic;
using NSNetwork;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class Game : MonoBehaviour {
    public UIMsgBox msgBox;
    static public Transform s_tfBaseForConv;
    static public Transform s_tfRoot;
    private GameLoveLetterMan gameMan;
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

    //  게임 관련
    public Text lbTitle;
    public Button btnReadyOrStart;

    // Use this for initialization
    void Start () {
        ScreenFade.Fade(1, 0, 1.0f, 0, true, () =>
        {
        });
        s_tfBaseForConv = tfTestBase;
        s_tfRoot = tfBase;
        NetworkUnityEvent.Instance.curMsgBox = msgBox;
        SoundManager.Instance.PlayBGM("bgm2");
        gameMan = GetComponent<GameLoveLetterMan>();
        gameMan.msgBox = msgBox;
        SetCallback();
        CardManager.Init();
        lbTitle.text = "Room : " + GlobalData.Instance.roomSN;
        Refresh();         
    }

    // Update is called once per frame
    void Update () {
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            msgBox.ShowYesNo("나가시겠습니까? 게임 중에는 패널티가 주어집니다.", () =>
            {
                OnBtnLeave();
            });            
        }            
    }

    public void OnTouch(int nType, int nID, float x, float y, float dx, float dy)
    {
        if (!gameMan.bInteractable || gameMan.isTouchProcessing)
        {            
            return;
        }        
                   
        Collider2D coll;
        Vector3 ray = Camera.main.ScreenToWorldPoint(new Vector3(x, y, 1));
        if (coll = Physics2D.OverlapPoint(new Vector2(ray.x, ray.y), 0x300))
        {
            if(coll.gameObject.CompareTag("Card"))
            {
                gameMan.ProcessCard(coll.GetComponent<Card>().m_nNum);
            }
            coll.transform.SendMessage("Selected");
        }
    }

    public void OnBtnReadyOrStart()
    {
        Debug.Log("OnBtnReadyOrStart");
        if( GlobalData.Instance.IsRoomMaster(GlobalData.Instance.userSN) )
        {
            if (GlobalData.Instance.roomUsers.Count < 3) return;
            foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
            {
                if(u.readyState == 0)
                {
                    return;
                }
            }
            Sender.GameStart();
        }
        else
        {
            Sender.GameReady();
        }
    }

    public void OnBtnLeave()
    {
        msgBox.ShowYesNo("방에서 나가시겠습니까?", ()=> {
            Sender.LeaveRoom();
        });        
    }

    public void OnLeaveRoomRet(GCPLeaveRoomRet leaveRoomRet)
    {        
        if (GlobalData.Instance.userSN == leaveRoomRet.sn)
        {
            ScreenFade.Fade(0, 1, 1.0f, 0, true, () =>
            {
                RemoveCallback();
                SceneManager.LoadScene("Lobby");
            });            
        }
        else {
        }
    }

    void Refresh()
    {        
        ClearUI();
        btnReadyOrStart.interactable = true;

        if (GlobalData.Instance.roomUsers == null) return;
        int idx = 0;
        int readyCnt = 0;
        foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
        {
            UserInfoBase uib;
            if (u.sn == GlobalData.Instance.userSN)
            {
                uib = m_LocalUser;
                readyCnt++;
            }
            else
            {
                uib = m_aRemoteUsers[idx];
                if( u.readyState != 0)
                {
                    readyCnt++;
                }
                idx++;
            }
            
            uib.SetNickName(u.nickName);
            if(GlobalData.Instance.IsRoomMaster(u.sn))
            {
                uib.m_lbReadyState.text = "Master";
            }
            else
            {
                uib.m_lbReadyState.text = u.readyState == 1 ? "Ready" : "Not Ready";
            }  
        }

        if(GlobalData.Instance.IsRoomMaster(GlobalData.Instance.userSN))
        {
            btnReadyOrStart.GetComponentInChildren<Text>().text = "Start!";            
            btnReadyOrStart.interactable = (readyCnt > 1 && readyCnt == GlobalData.Instance.roomUsers.Count) ? true : false;
        }
        else
        {
            GCPRoomState.UserInfo me = null;
            foreach (GCPRoomState.UserInfo u in GlobalData.Instance.roomUsers)
            {
                if(u.sn == GlobalData.Instance.userSN) { me = u;  break; }
            }
            btnReadyOrStart.GetComponentInChildren<Text>().text = me.readyState == 1 ? "Cancel" : "Ready";
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
        SoundManager.Instance.PlaySfx("enterroom");
        Refresh();
    }

    public void OnGameStartRet(GCPGameStartRet gameStartRet)
    {
        if(gameStartRet.result != 0)
        {
            return;
        }

        gameMan.ResetUIForGame(gameStartRet, m_LocalUser, m_aRemoteUsers);
        GameStart();        
    }   

    void SetCallback()
    {        
        Receiver.OnRoomStateCallback += OnRoomState;
        Receiver.OnLeaveRoomRetCallback += OnLeaveRoomRet;
        Receiver.OnGameStartRetCallback += OnGameStartRet;
        Receiver.OnLLInitStatusCallback += OnLLInitStatus;
        Receiver.OnLLActionRetCallback += OnLLActionRet;
        Receiver.OnLLStatusCallback += OnLLStatus;
        Receiver.OnLLRoundResultCallback += OnRoundResult;
        Receiver.OnLLFinalResultCallback += OnFinalRoundResult;
        Receiver.OnLLAbortedCallback += OnAborted;
        TouchMan.Instance.ResetEvent();
        TouchMan.Instance.begin0 += OnTouch;
        TouchMan.Instance.move0 += OnTouch;
        TouchMan.Instance.end0 += OnTouch;
        /*begin0 += OnTouch;
        move0 += OnTouch;
        end0 += OnTouch;*/
    }

    void OnLLInitStatus(GCPLLInitStatus status) {        gameMan.OnLLInitStatus(status);    }
    void OnLLActionRet(GCPLLActionRet action) { gameMan.OnLLActionRet(action); }
    void OnLLStatus(GCPLLStatus status) { gameMan.OnLLStatus(status); }
    void OnRoundResult(GCPLLRoundResult ret) { gameMan.OnRoundResult(ret); }
    void OnFinalRoundResult(GCPLLFinalResult ret) {
        btnReadyOrStart.gameObject.SetActive(true);
        gameMan.OnFinalRoundResult(ret);
    }
    void OnAborted(GCPLLAborted ret) {
        btnReadyOrStart.gameObject.SetActive(true);
        gameMan.OnAborted(ret);
    }


    void RemoveCallback()
    {
        Receiver.OnRoomStateCallback -= OnRoomState;
        Receiver.OnLeaveRoomRetCallback -= OnLeaveRoomRet;
        Receiver.OnGameStartRetCallback -= OnGameStartRet;
        Receiver.OnLLInitStatusCallback -= OnLLInitStatus;
        Receiver.OnLLActionRetCallback -= OnLLActionRet;
        Receiver.OnLLStatusCallback -= OnLLStatus;
        Receiver.OnLLRoundResultCallback -= OnRoundResult;
        Receiver.OnLLFinalResultCallback -= OnFinalRoundResult;
        Receiver.OnLLAbortedCallback -= OnAborted;
        TouchMan.Instance.ResetEvent();
        TouchMan.Instance.begin0 -= OnTouch;
        TouchMan.Instance.move0 -= OnTouch;
        TouchMan.Instance.end0 -= OnTouch;
        /*begin0 -= OnTouch;
        move0 -= OnTouch;
        end0 -= OnTouch;*/
    }

    void GameStart()
    {
        btnReadyOrStart.gameObject.SetActive(false);
    }
}
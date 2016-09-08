using UnityEngine;
using System.Collections;
using DG.Tweening;
using System.Collections.Generic;
using NSNetwork;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class Game : MonoBehaviour {
    public UIMsgBox msgBox;    
    private GameLoveLetterMan gameMan;

    public UserLocalInfo m_LocalUser;
    public UserRemoteInfo[] m_aRemoteUsers;
    public Transform tfTestBase;
    public Transform tfBase;    

    //  게임 관련
    public Text lbTitle;
    public Button btnReadyOrStart;
    [SerializeField]
    private UICardInfo uiCardInfo; 
    private Card currentProcessingCard = null;
    private long tCurrentProcessingCard;
    private Vector3 vCurrentProcessingCardPos;

    // Use this for initialization
    void Start () {
        ScreenFade.Fade(1, 0, 1.0f, 0, true, () =>
        {
        });        
        NetworkUnityEvent.Instance.curMsgBox = msgBox;
        SoundManager.Instance.PlayBGM("bgm2");
        gameMan = GetComponent<GameLoveLetterMan>();
        gameMan.msgBox = msgBox;
        RemoveCallback();
        SetCallback();
        CardManager.Init();
        uiCardInfo.gameObject.SetActive(false);
        lbTitle.text = "Room : " + GlobalData.Instance.roomSN;
        Refresh();
        int nTutoRet = PlayerPrefs.GetInt(GlobalData.Instance.P_TUTORIAL1, -1);
        if(nTutoRet == -1)
        {
            msgBox.Show("자신의 턴일 때, 카드를 길게 누르면 상세 설명이 나옵니다.", "확인", ()=> {
                PlayerPrefs.SetInt(GlobalData.Instance.P_TUTORIAL1, 1);
            });
        }        
    }

    // Update is called once per frame
    void Update () {
        if (Input.GetKeyDown(KeyCode.Escape))
        {   
            OnBtnLeave();
        }            
    }

    public void OnTouch(int nType, int nID, float x, float y, float dx, float dy)
    {
        Collider2D coll;
        Vector3 ray = Camera.main.ScreenToWorldPoint(new Vector3(x, y, 1));
        coll = Physics2D.OverlapPoint(new Vector2(ray.x, ray.y), 0x300);

        if (!gameMan.bInteractable || gameMan.isTouchProcessing)
        {            
            return;
        }

        switch (nType)
        {
            case 0:/*Begin*/
                {                    
                    if (coll != null && coll.gameObject.CompareTag("Card"))
                    {
                        Card c = coll.GetComponent<Card>();
                        if (!c.bActive) return;
                        currentProcessingCard = c;
                        tCurrentProcessingCard = System.DateTime.Now.Ticks;
                        vCurrentProcessingCardPos = currentProcessingCard.transform.position;
                    }                    
                }
                break;

            case 1:/*Move*/
                {
                    if(currentProcessingCard && (System.DateTime.Now.Ticks - tCurrentProcessingCard > 1000))
                    {
                        //  카드 설명                        
                        if (!uiCardInfo.gameObject.activeInHierarchy)
                        {
                            uiCardInfo.gameObject.SetActive(true);
                            uiCardInfo.Show(currentProcessingCard.m_nNum);
                        }     
                        else
                        {
                            currentProcessingCard.transform.position = new Vector3(ray.x, ray.y, 1);
                        }                   
                    }
                }
                break;

            case 2:/*End*/
                {
                    if(coll != null && currentProcessingCard != null && coll.GetComponent<Card>() == currentProcessingCard)
                    {
                        if (!uiCardInfo.gameObject.activeInHierarchy)
                        {
                            gameMan.ProcessCard(currentProcessingCard.m_nNum);
                        }
                        currentProcessingCard.transform.position = vCurrentProcessingCardPos;
                        currentProcessingCard = null;                        
                    }

                    uiCardInfo.gameObject.SetActive(false);
                    uiCardInfo.Hide();
                }
                break;
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
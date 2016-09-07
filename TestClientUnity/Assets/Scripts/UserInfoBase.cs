using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using NSNetwork;
using System.Collections.Generic;

public class UserInfoBase : MonoBehaviour {

    public GameObject m_panelHands;
    public GameObject m_panelGround;
    public Text m_lbName;
    public Text m_lbReadyState;
    public Text m_lbWinLose;
    public bool m_bLocal;
    public GameObject m_myTurn;
    public GameObject m_shield;
    public bool bShield = false;
    public int m_nGameIndex;
    public int m_nUserSN;
    public bool bDead = false;
    public int m_nTokken;
    public GameObject m_dead;
    public List<Card> liCardHand = new List<Card>();
    public List<Card> liCardGround = new List<Card>();
    public EmotionMan emotion;
    public Image[] jewelries;

    // Use this for initialization
    void Start () {        
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    public void ClearInfo()
    {
        m_lbName.text = "-NoName-";
        m_lbReadyState.text = "------";
        m_lbWinLose.text = "------";
        m_nUserSN = 0;
        m_nGameIndex = -1;
        SetShield(false);
        SetDead(false);
        bDead = false;
        m_nTokken = 0;
        ResetTokenImg();        
    }

    public void SetNickName(string s)
    {
        m_lbName.text = s;
    }

    public void SetReadyStateMsg(string s)
    {
        m_lbReadyState.text = s;
    }

    public void SetWinLoseMsg(string s)
    {
        m_lbWinLose.text = s;
    }

    virtual public void Refresh(GCPLLInitStatus.PlayerInfo pinfo) {
        ClearHandAndGround();
        SetDead(false);
        SetShield(pinfo.shieldState == 1 ? true : false);
    }

    public void SetShield(bool _bShield)
    {
        if (bShield == _bShield) {
            return;
        }
        m_shield.SetActive(_bShield);
        bShield = _bShield;
    }

    public void SetDead(bool _bDead)
    {
        m_dead.SetActive(_bDead);
        bDead = _bDead;
    }

    public void SetMyTurn(bool _bMyTurn)
    {
        m_myTurn.SetActive(_bMyTurn);
    }

    virtual public void PutHand(Card c) {
        SoundManager.Instance.PlaySfx("card_move");
    }
    virtual public void DropCard(Card c) {
        SoundManager.Instance.PlaySfx("card_move");
    }
    virtual public void DropCard(int nCard) {
        SoundManager.Instance.PlaySfx("card_move");
    }
    virtual public void SendCard(UserInfoBase targetUI, int nCard) {        
    }

    protected void ClearHandAndGround() {
        liCardHand.Clear();
        liCardGround.Clear();
        List<Transform> liDelete = new List<Transform>();
        foreach (Transform child in m_panelHands.transform) { liDelete.Add(child); }
        foreach (Transform t in liDelete) { DestroyObject(t.gameObject); }
        liDelete = new List<Transform>();
        foreach (Transform child in m_panelGround.transform) { liDelete.Add(child); }
        foreach (Transform t in liDelete) { DestroyObject(t.gameObject); }
    }

    public void Dead() {
        bDead = true;
        m_dead.SetActive(true);
    }

    public void AddTokken()
    {
        m_nTokken++;
        ResetTokenImg();
    }

    void ResetTokenImg()
    {
        foreach(Image img in jewelries)
        {
            img.gameObject.SetActive(false);
        }
        for(int i = 0; i < m_nTokken;++i)
        {
            jewelries[i].gameObject.SetActive(true);
        }            
    }
}

public class GameUser
{
    public GameUser()
    {
        m_nUserSN = -1;
        m_nGameIndex = -1;
        infoUI = null;
    }

    public int m_nUserSN;    
    public int m_nGameIndex;
    public UserInfoBase infoUI;
    public bool m_bLocal;

    public string GetNickName()
    {
        return infoUI.m_lbName.text;
    }
}

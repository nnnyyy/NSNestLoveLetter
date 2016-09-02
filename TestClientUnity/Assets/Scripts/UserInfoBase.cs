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
    public List<Card> liCardHand = new List<Card>();
    public List<Card> liCardGround = new List<Card>();

    // Use this for initialization
    void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public void ClearInfo()
    {
        m_lbName.text = "";
        m_lbReadyState.text = "";
        m_lbWinLose.text = "";
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

    virtual public void PutHand(Card c) { }
    virtual public void DropCard(Card c) { }
    virtual public void DropCard(int nCard) { }
    virtual public void SendCard(UserInfoBase targetUI, int nCard) {        
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
}

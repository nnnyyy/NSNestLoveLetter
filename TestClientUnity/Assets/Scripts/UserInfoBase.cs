using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class UserInfoBase : MonoBehaviour {

    public GameObject m_panelHands;
    public GameObject m_panelGround;
    public Text m_lbName;
    public Text m_lbReadyState;

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
    }

    virtual public void PutHand(Card c) { }
}

public class GameUser
{
    public GameUser()
    {
        m_sName = "";
        m_bMaster = false;
        m_bReady = false;
        m_nGameIndex = -1;
    }

    public int m_nUserSN;
    public string m_sName;
    public bool m_bMaster;
    public bool m_bReady;
    public int m_nGameIndex;
}

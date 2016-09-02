using UnityEngine;
using System.Collections;
using NSNetwork;
using DG.Tweening;

public class UserRemoteInfo : UserInfoBase {

	// Use this for initialization
	void Start () {
        m_bLocal = false;
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    void Selected()
    {
        Debug.Log("Selected!");
    }

    public override void Refresh(GCPLLInitStatus.PlayerInfo pinfo)
    {
        base.Refresh(pinfo);
        Card c = CardManager.CreateCard(0);
        PutHand(c);        
    }

    public override void PutHand(Card c)
    {
        base.PutHand(c);
        c.Set(0); //  항상 뒷면
        liCardHand.Add(c);
        c.transform.parent = m_panelHands.transform;
        c.transform.position = m_panelHands.transform.position;
        c.transform.localScale = new Vector3(0.4f, 0.4f, 1);
    }

    public override void DropCard(Card c)
    {        
    }

    public override void DropCard(int nCard)
    {
        base.DropCard(nCard);        
        Card cDrop = null;
        foreach (Card c in liCardHand)
        {            
            liCardHand.Remove(c);
            cDrop = c;
            break;            
        }

        liCardGround.Add(cDrop);
        cDrop.Set(nCard);
        cDrop.transform.parent = m_panelGround.transform;
        cDrop.transform.position = m_panelGround.transform.position;
        cDrop.transform.localScale = new Vector3(0.3f, 0.3f, 1);
    }

    public override void SendCard(UserInfoBase targetUI, int nCard)
    {
        Card c = liCardHand[0];
        liCardHand.Remove(c);
        if (targetUI.m_bLocal)
        {
            c.Set(nCard);
            targetUI.PutHand(c);
        }
        else {
            c.Set(0);
            targetUI.PutHand(c);
        }
    }
}

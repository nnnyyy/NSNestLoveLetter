﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;
using NSNetwork;

public class UserLocalInfo : UserInfoBase {    

    // Use this for initialization
    void Start () {
        m_bLocal = true;
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    public override void Refresh(GCPLLInitStatus.PlayerInfo pinfo)
    {
        base.Refresh(pinfo);
        foreach(int nCard in pinfo.listHandCards)
        {
            Card c = CardManager.CreateCard(nCard);
            PutHand(c);
        }
    }

    public override void PutHand(Card c)
    {
        base.PutHand(c);
        liCardHand.Add(c);
        c.transform.parent = m_panelHands.transform;
        c.transform.position = m_panelHands.transform.position;
        c.transform.localScale = new Vector3(0.8f, 0.8f, 1);
    }

    public override void DropCard(Card c)
    {
                        
    }

    public override void DropCard(int nCard)
    {
        base.DropCard(nCard);
        Card cDrop = null;
        foreach(Card c in liCardHand)
        {
            if(c.m_nNum == nCard)
            {
                liCardHand.Remove(c);
                cDrop = c;
                break;
            }
        }

        liCardGround.Add(cDrop);
        cDrop.transform.parent = m_panelGround.transform;
        cDrop.transform.position = m_panelGround.transform.position;
        cDrop.transform.localScale = new Vector3(0.4f, 0.4f, 1);
    }

    public override void SendCard(UserInfoBase targetUI, int nCard)
    {
        Card c = liCardHand[0];
        liCardHand.Remove(c);        
        c.Set(0);
        targetUI.PutHand(c);
        
    }
}

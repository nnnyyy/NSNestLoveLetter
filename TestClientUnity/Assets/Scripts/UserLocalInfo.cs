using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;
using NSNetwork;

public class UserLocalInfo : UserInfoBase {    

    // Use this for initialization
    void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public override void Refresh(GCPLLStatus.PlayerInfo pinfo)
    {
        base.Refresh(pinfo);

        if (pinfo.bMyTurn)
        {
            m_myTurn.SetActive(true);
        }
        else
        {
            m_myTurn.SetActive(false);
        }

        if (pinfo.shieldState == 1)
        {
            bShield = true;
            m_shield.SetActive(true);
        }
        else
        {
            bShield = false;
            m_shield.SetActive(false);
        }

        int diff = pinfo.listGroundCards.Count - liCardGround.Count;
        if (diff > 0)
        {
            while (diff > 0)
            {
                int cardNum = pinfo.listGroundCards[pinfo.listGroundCards.Count - diff];
                Card c = CardManager.CreateCard(Card.SizeType.REMOTE, cardNum);
                PutGround(c);
                --diff;
            }
        }

        diff = pinfo.listHandCards.Count - liCardHand.Count;
        if (diff > 0)
        {
            while (diff > 0)
            {
                int cardNum = pinfo.listHandCards[pinfo.listHandCards.Count - diff];
                Card c = CardManager.CreateCard(Card.SizeType.REMOTE, cardNum);
                PutHand(c);
                --diff;
            }
        }
    }

    public override void PutHand(Card c)
    {
        base.PutHand(c);
        c.transform.SetParent(m_panelHands.transform);
        c.transform.DOScale(new Vector3(0.5f, 0.5f, 1), 0.3f);
        c.transform.DOMove(m_panelHands.transform.position, 0.3f).OnComplete(()=> {
            liCardHand.Add(c);
            SortCards();
        });
        c.touchEvent += OnTouchCard;
    }

    public void HandToGround(Card c)
    {
        
    }

    void SortCards()
    {
        int idx = 0;
        int nStart = 0 - ((int)m_panelHands.gameObject.GetComponent<RectTransform>().rect.width / 2 );
        foreach(Card c in liCardHand)
        {
            c.transform.DOLocalMove(new Vector3(nStart + idx * 40, 0, 1), 0.5f);            
            idx++;
        }
    }

    void OnTouchCard(Card cTouched)
    {

    }
}

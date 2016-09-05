using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using DG.Tweening;
using NSNetwork;

public class UserLocalInfo : UserInfoBase {    

    // Use this for initialization
    void Start () {
        m_bLocal = true;
        m_dead.SetActive(false);
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
        c.transform.localScale = new Vector3(0.8f, 0.8f, 1);
        c.transform.DOMove(m_panelHands.transform.position, 0.3f);
        c.transform.DOScale(new Vector3(0.8f, 0.8f, 1), 0.3f).OnComplete(() =>
        {
            Sort();
        });
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
        Vector3 localScaleBackup = cDrop.transform.localScale;
        cDrop.transform.parent = m_panelGround.transform;
        cDrop.transform.localScale = localScaleBackup;
        cDrop.transform.DOMove(m_panelGround.transform.position, 0.3f);
        cDrop.transform.DOScale(new Vector3(0.4f, 0.4f, 1), 0.3f).OnComplete(()=> {
            Sort();
        });        
    }

    public override void SendCard(UserInfoBase targetUI, int nCard)
    {
        Card c = liCardHand[0];
        liCardHand.Remove(c);        
        c.Set(0);
        targetUI.PutHand(c);        
    }

    void Sort()
    {
        int gap = 40;
        if(liCardHand.Count == 1)
        {
            liCardHand[0].transform.DOMove(m_panelHands.transform.position, 0.2f);            
        }
        else if (liCardHand.Count == 2)
        {
            Vector3 v = Vector3.zero;
            v.x -= gap;
            liCardHand[0].transform.DOLocalMove(v, 0.2f);
            v = Vector3.zero; v.z = -1;
            v.x += gap;
            liCardHand[1].transform.DOLocalMove(v, 0.2f);
        }

        int idx = 0;
        int nStart = 0 - ((int)m_panelHands.gameObject.GetComponent<RectTransform>().rect.width / 2);
        foreach (Card c in liCardGround)
        {
            c.transform.DOLocalMove(new Vector3(nStart + idx * gap, 0, 1), 0.5f);
            idx++;
        }
    }
}

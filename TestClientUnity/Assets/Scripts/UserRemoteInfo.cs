using UnityEngine;
using System.Collections;
using NSNetwork;
using DG.Tweening;

public class UserRemoteInfo : UserInfoBase {

	// Use this for initialization
	void Start () {        
        m_bLocal = false;
        m_dead.SetActive(false);        
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
        c.bActive = false;
        liCardHand.Add(c);        
        c.transform.parent = m_panelHands.transform;
        c.transform.localScale = new Vector3(0.8f, 0.8f, 1);
        c.transform.DOMove(m_panelHands.transform.position, 0.3f);
        c.transform.DOScale(new Vector3(0.4f, 0.4f, 1), 0.3f).OnComplete(() =>
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
        foreach (Card c in liCardHand)
        {            
            liCardHand.Remove(c);
            cDrop = c;
            break;            
        }

        liCardGround.Add(cDrop);
        cDrop.Set(nCard);
        cDrop.bActive = false;
        Vector3 localScaleBackup = cDrop.transform.localScale;
        cDrop.transform.parent = m_panelGround.transform;
        cDrop.transform.localScale = localScaleBackup;
        cDrop.transform.DOMove(m_panelGround.transform.position, 0.3f);
        cDrop.transform.DOScale(new Vector3(0.4f, 0.4f, 1), 0.4f).OnComplete(() =>
        {
            Sort();
        });
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

    void Sort()
    {
        int gap = 15;
        int ground_gap = 35;
        if (liCardHand.Count == 1)
        {
            liCardHand[0].transform.DOMove(m_panelHands.transform.position, 0.2f);
        }
        else if (liCardHand.Count == 2)
        {
            Vector3 v = Vector3.zero;
            v.x -= gap;
            liCardHand[0].transform.DOLocalMove(v, 0.2f);
            v = Vector3.zero;
            v.x += gap;
            liCardHand[1].transform.DOLocalMove(v, 0.2f);
        }

        int idx = 0;
        int nStart = 0 - ((int)m_panelHands.gameObject.GetComponent<RectTransform>().rect.width / 2);
        foreach (Card c in liCardGround)
        {
            c.transform.DOLocalMove(new Vector3(nStart + idx * ground_gap, 0, 1), 0.5f);
            idx++;
        }
    }
}

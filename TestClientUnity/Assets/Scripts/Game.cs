using UnityEngine;
using System.Collections;
using DG.Tweening;

public class Game : MonoBehaviour {
    static public Transform s_tfBaseForConv;
    static public Transform s_tfRoot;
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
    public UserInfoBase[] m_InfoList;
    public Transform tfTestBase;
    public Transform tfBase;
    public Transform tfTestRemoteHand;
    public Transform tfTestLocalHand;

    // Use this for initialization
    void Start () {
        s_tfBaseForConv = tfTestBase;
        s_tfRoot = tfBase;
        CardManager.Init();
        Card c = CardManager.CreateCard(Card.SizeType.LOCAL_HAND, 1);
        m_InfoList[0].PutHand(c);
        Card c2 = CardManager.CreateCard(Card.SizeType.LOCAL_HAND, 1);
        m_InfoList[0].PutHand(c2);
        Card c3 = CardManager.CreateCard(Card.SizeType.LOCAL_HAND, 1);
        m_InfoList[0].PutHand(c3);
        /*
        c.transform.SetParent(s_tfRoot);
        Vector3 vRoot = ConvPosToRootGlobal(tfTestRemoteHand.position);
        Sequence doSequence = DOTween.Sequence();
        doSequence.Append(c.transform.DOMove(vRoot, 0.5f));
        doSequence.Join(c.transform.DOScale(0.25f, 0.25f));
        vRoot = ConvPosToRootGlobal(tfTestLocalHand.position);
        doSequence.Append(c.transform.DOMove(vRoot, 0.5f));
        doSequence.Join(c.transform.DOScale(0.5f, 0.5f)).OnComplete(()=> {
            
        });
        */
    }

    // Update is called once per frame
    void Update () {
        	
	}
}
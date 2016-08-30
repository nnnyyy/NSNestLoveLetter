using UnityEngine;
using System.Collections;
using DG.Tweening;

public class Game : MonoBehaviour {

    public UserInfoBase[] m_InfoList;
    public Transform tfTestBase;
    public Transform tfBase;
    public Transform tfTestRemoteHand;
    public Transform tfTestLocalHand;

    // Use this for initialization
    void Start () {
        CardManager.Init();
        Card c = CardManager.CreateCard(Card.SizeType.LOCAL_HAND, 1);
        m_InfoList[0].PutHand(c);
        c.transform.parent = tfBase.transform;
        //c.transform.SetSiblingIndex(0);
        tfTestBase.position = tfTestRemoteHand.position;
        Sequence doSequence = DOTween.Sequence();
        doSequence.Append(c.transform.DOLocalMove(tfTestBase.localPosition, 0.5f));
        doSequence.Join(c.transform.DOScale(0.25f, 0.25f));
        tfTestBase.position = tfTestLocalHand.position;
        doSequence.Append(c.transform.DOLocalMove(tfTestBase.localPosition, 0.5f));
        doSequence.Join(c.transform.DOScale(0.5f, 0.5f)).OnComplete(()=> {
            c.transform.parent = tfTestLocalHand;
        });
        

    }

    // Update is called once per frame
    void Update () {
	
	}
}
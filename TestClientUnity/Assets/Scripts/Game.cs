using UnityEngine;
using System.Collections;
using DG.Tweening;

public class Game : MonoBehaviour {

    delegate void touchListener(int nType, int nID, float x, float y, float dx, float dy);

    event touchListener begin0, begin1;
    event touchListener move0, move1;
    event touchListener end0, end1;
    Vector2[] touchDelta = new Vector2[1];

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

    public void OnTouch(int nType, int nID, float x, float y, float dx, float dy)
    {
        if(nType == 0)
        {
            Card c = CardManager.CreateCard(Card.SizeType.LOCAL_HAND, 1);
            m_InfoList[0].PutHand(c);
        }        
    }

    // Use this for initialization
    void Start () {
        begin0 += OnTouch;
        move0 += OnTouch;
        end0 += OnTouch;

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
        //  Define 걸어서 나누자
        bool bMouseClicked = Input.GetMouseButton(0);
        if (bMouseClicked)
        {
            Vector2 pos = Input.mousePosition;            
            if (begin0 != null) begin0(0, 0, pos.x, pos.y, 0, 0);            
        }

        int cnt = Input.touchCount;
        for(int i = 0; i < cnt; ++i)
        {
            Touch touch = Input.GetTouch(i);
            int id = touch.fingerId;
            Vector2 pos = touch.position;
            if (touch.phase == TouchPhase.Began) touchDelta[id] = touch.position;

            float x, y, dx = 0, dy = 0;
            x = pos.x;
            y = pos.y;
            if(touch.phase != TouchPhase.Began)
            {
                dx = x - touchDelta[id].x;
                dy = y - touchDelta[id].y;
            }

            if (touch.phase == TouchPhase.Began)
            {
                switch (id)
                {
                    case 0:
                        if (begin0 != null) begin0(0, id, x, y, dx, dy);
                        break;
                }                
            }
        }        	
	}
}
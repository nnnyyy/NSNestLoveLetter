using UnityEngine;
using System.Collections;

public class TouchMan : Singleton<TouchMan> {

    public delegate void touchListener(int nType, int nID, float x, float y, float dx, float dy);
    public event touchListener begin0;
    public event touchListener move0;
    public event touchListener end0;
    Vector2[] touchDelta = new Vector2[1];

    // Use this for initialization
    void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
        //  Define 걸어서 나누자        
        bool bMouseClicked = Input.GetMouseButtonDown(0);
        if (bMouseClicked)
        {
            Vector2 pos = Input.mousePosition;
            if (begin0 != null) begin0(0, 0, pos.x, pos.y, 0, 0);
        }

        int cnt = Input.touchCount;
        for (int i = 0; i < cnt; ++i)
        {
            Touch touch = Input.GetTouch(i);
            int id = touch.fingerId;
            Vector2 pos = touch.position;
            if (touch.phase == TouchPhase.Began) touchDelta[id] = touch.position;

            float x, y, dx = 0, dy = 0;
            x = pos.x;
            y = pos.y;
            if (touch.phase != TouchPhase.Began)
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

    public void ResetEvent()
    {
        begin0 = null;
        move0 = null;
        end0 = null;
    }
}

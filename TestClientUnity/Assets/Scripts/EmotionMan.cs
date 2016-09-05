using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using System;

public class EmotionMan : MonoBehaviour {

    public Image[] laugh;
    public Image[] crying;

    private int tStart;
    private bool bShow;
    private int nCurLaugh = -1; 

    // Use this for initialization
    void Start () {	
	}
	
	// Update is called once per frame
	void Update () {	
        if(bShow && Environment.TickCount - tStart >= 1100)
        {
            bShow = false;
            if(nCurLaugh != -1)
            {
                laugh[nCurLaugh].gameObject.SetActive(false);
            }
        }
	}

    public void Laugh() {
        if (laugh.Length <= 0) return;
        if(nCurLaugh != -1)
        {
            laugh[nCurLaugh].gameObject.SetActive(false);
        }
        nCurLaugh = UnityEngine.Random.Range(0, laugh.Length);
        laugh[nCurLaugh].gameObject.SetActive(true);
        SoundManager.instance.PlaySingle(6);
        bShow = true;
        tStart = Environment.TickCount;
    }
}

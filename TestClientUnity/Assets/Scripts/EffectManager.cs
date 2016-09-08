using UnityEngine;
using System.Collections;

public class EffectManager : MonoBehaviour {

    [SerializeField]
    private Animator[] animators;

    [SerializeField]
    private Transform uiPanelRoot;

    private static EffectManager instance;
    public static EffectManager GetInstance()
    {
        if (!instance)
        {
            instance = GameObject.FindObjectOfType<EffectManager>();
            if (!instance)
                Debug.LogError("There needs to be one active MyClass script on a GameObject in your scene.");
        }

        return instance;
    }

    public void ShowEffect(int _idx, Vector3 _vPos)
    {
        Animator anim = GameObject.Instantiate<Animator>(animators[_idx]);
        anim.transform.SetParent(uiPanelRoot);
        anim.transform.localScale = new Vector3(1, 1, 1);
        anim.transform.position = _vPos;
        anim.Play("Normal");
    }

    // Use this for initialization
    void Start () {        
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}

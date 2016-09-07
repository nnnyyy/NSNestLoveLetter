using UnityEngine;
using System.Collections;

public class AutoDestroyedAnimation : MonoBehaviour {

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
        Animator anim = GetComponent<Animator>();
        if(anim.GetCurrentAnimatorStateInfo(0).normalizedTime >= 1.0f)
        {
            Destroy(this.gameObject);
        }
	}
}

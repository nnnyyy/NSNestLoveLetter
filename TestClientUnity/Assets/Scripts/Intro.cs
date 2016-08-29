using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using UnityEngine.SceneManagement;

public class Intro : MonoBehaviour {

    public UIMsgBox msgBox;
    public Text m_lbID;
    public Text m_lbPw;

    public enum Type {
        OK = 0,
        OKCANCEL = 1,        
    }

	// Use this for initialization
	void Start () {
        msgBox.gameObject.SetActive(false);
    }
	
	// Update is called once per frame
	void Update () {
	
	}

    public void OnBtnLogin() {
        if(m_lbID.text == "" || m_lbPw.text == "")
        {
            msgBox.Show("아이디 또는 비밀번호를 입력 해 주세요");
            return;
        }

        SceneManager.LoadScene("Lobby");
    }

    public void OnBtnRegister(){
        msgBox.Show("지원하지 않는 기능입니다.");
    }
}

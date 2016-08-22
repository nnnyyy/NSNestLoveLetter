using UnityEngine;
using System.Collections;

public class DefaultPopup : MonoBehaviour
{
    public enum eType
    {
        OK,
        OKCancel,
    }

    public UILabel labelTitle;
    public UILabel labelMessage;
    public GameObject btnOK;
    public GameObject btnCancel;


    public static DefaultPopup objPopup;
    public delegate void CallbackDelegate();
    private CallbackDelegate onCallbackOk;
    private CallbackDelegate onCallbackCancel;

    private DefaultPopup()
    {
    }

    public void Show(string title, string message, eType type, CallbackDelegate _onCallbackOk, CallbackDelegate _onCallbackCancel)
    {
        labelTitle.text = title;
        labelMessage.text = message;
        onCallbackOk = _onCallbackOk;
        onCallbackCancel = _onCallbackCancel;

        if (type == eType.OK)
        {
            btnOK.SetActive(true);
            btnCancel.SetActive(false);
            btnOK.transform.localPosition = new Vector3(0f, -110f, 0f);
        }
        else
        {
            btnOK.SetActive(true);
            btnCancel.SetActive(true);
            btnOK.transform.localPosition = new Vector3(-60f, -110f, 0f);
            btnCancel.transform.localPosition = new Vector3(60f, -110f, 0f);
        }
    }

    public void OnClickOK()
    {
        if (onCallbackOk != null)
            onCallbackOk();

        GameObject.Destroy(objPopup.gameObject);
        objPopup = null;
    }

    public void OnClickCancel()
    {
        if (onCallbackCancel != null)
            onCallbackCancel();

        GameObject.Destroy(objPopup.gameObject);
        objPopup = null;
    }


    public static void OpenPopup(string title, string message, eType type, CallbackDelegate _onCallbackOk = null, CallbackDelegate _onCallbackCancel = null)
    {
        if(objPopup == null)
        {
            GameObject prafab = Resources.Load<GameObject>("UI/GUI_DefaultPopup");
            GameObject obj = GameObject.Instantiate(prafab) as GameObject;
            objPopup = obj.GetComponent<DefaultPopup>();
        }

        objPopup.Show(title, message, type, _onCallbackOk, _onCallbackCancel);
    }

}

using UnityEngine;
using System.Collections.Generic;
using NSNest.Common;
using NSNetwork;

namespace NSNest.UI
{
    public class IntroController : MonoBehaviour
    {
        public UIInput inputID;
        public UIInput inputPW;

        void Start()
        {
            NetworkUnityEvent.Instance.Connect(Const.SERVER_IP, Const.SERVER_PORT);

            inputID.value = PlayerPrefs.GetString("UserID", "");
            inputPW.value = PlayerPrefs.GetString("UserPW", "");
        }

        public void OnClickCreateAccount()
        {
        }

        public void OnClickLogin()
        {
            if (string.IsNullOrEmpty(inputID.value) == true)
            {
                DefaultPopup.OpenPopup("Login", "id is null", DefaultPopup.eType.OK);
                return;
            }

            if (string.IsNullOrEmpty(inputPW.value) == true)
            {
                DefaultPopup.OpenPopup("Login", "pw is null", DefaultPopup.eType.OK);
                return;
            }

            Receiver.onLoginRetCallback = OnLoginCallBack;
            Debug.Log("id >> " + inputID.value + ", pw >> " + inputPW.value);
            Sender.Login(inputID.value, inputPW.value);
        }

        public void OnClickClose()
        {
            Application.Quit();
        }

        void OnLoginCallBack(GCPLoginRet loginRet)
        {
            switch(loginRet.result)
            {
                case GCPLoginRet.eResult.Success:
                    PlayerPrefs.SetString("UserID", inputID.value);
                    PlayerPrefs.SetString("UserPW", inputPW.value);
                    SceneLoader.Instance.Load("Room");
                    break;

                case GCPLoginRet.eResult.ErrorIdOrPassword:
                    DefaultPopup.OpenPopup("Login", "ErrorIdOrPassword", DefaultPopup.eType.OK);
                    break;

                case GCPLoginRet.eResult.NotJoined:
                    DefaultPopup.OpenPopup("Login", "NotJoined", DefaultPopup.eType.OK);
                    break;

                case GCPLoginRet.eResult.NowConneting:
                    DefaultPopup.OpenPopup("Login", "NowConneting", DefaultPopup.eType.OK);
                    break;

                default:
                    DefaultPopup.OpenPopup("Login", "default", DefaultPopup.eType.OK);
                    break;
            }
        }
    }
}
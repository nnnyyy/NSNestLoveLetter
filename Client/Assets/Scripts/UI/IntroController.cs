using UnityEngine;
using System.Collections.Generic;

namespace NSNest.UI
{
    public class IntroController : MonoBehaviour
    {
        ButtonController m_ButtonController = null;
        ButtonController ButtonController
        {
            get
            {
                if (m_ButtonController == null)
                {
                    m_ButtonController = GetComponent<ButtonController>();
                    if (m_ButtonController == null)
                        m_ButtonController = gameObject.AddComponent<ButtonController>();
                }
                return m_ButtonController;
            }
        }

        void Start()
        {
            ButtonController.OnPressButton += ButtonController_OnPressButton;
        }

        void OnDestroy()
        {
            ButtonController.OnPressButton -= ButtonController_OnPressButton;
        }

        private void ButtonController_OnPressButton(string buttonName)
        {
            Debug.Log("ButtonController_OnPressButton :" + buttonName);
        }
    }
}
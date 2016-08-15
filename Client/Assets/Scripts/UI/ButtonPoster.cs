using UnityEngine;
using System.Collections;

namespace NSNest.UI
{
    public delegate void PressButton(string buttonName);
    
    public class ButtonPoster : MonoBehaviour
    {
        [SerializeField]
        private string m_ButtonName = null;

        PressButton m_OnPressButton = null;
        public event PressButton OnPressButton
        {
            add { m_OnPressButton += value; }
            remove { m_OnPressButton -= value; }
        }

        // Use this for initialization
        void OnClick()
        {
            if(m_OnPressButton!=null)
                m_OnPressButton(m_ButtonName);
        }
    }

}
using UnityEngine;
using System.Collections.Generic;

namespace NSNest.UI
{
    public class ButtonController : MonoBehaviour
    {
        PressButton m_OnPressButton = null;
        public event PressButton OnPressButton
        {
            add { m_OnPressButton += value; }
            remove { m_OnPressButton -= value; }
        }

        List<ButtonPoster> m_ListButtonPoster = null;
        List<ButtonPoster> ListButtonPoster
        {
            get
            {
                if(m_ListButtonPoster==null)
                    m_ListButtonPoster = new List<ButtonPoster>(GetComponentsInChildren<ButtonPoster>(true));
                return m_ListButtonPoster;
            }
        }
        
        void Awake()
        {
            foreach(ButtonPoster poster in ListButtonPoster)
                poster.OnPressButton += Poster_OnPressButton;
        }

        void OnDestroy()
        {
            foreach (ButtonPoster poster in ListButtonPoster)
                poster.OnPressButton -= Poster_OnPressButton;
        }

        void Poster_OnPressButton(string buttonName)
        {
            if (m_OnPressButton != null)
                m_OnPressButton(buttonName);
        }
    }

}
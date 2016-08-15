using UnityEngine;
using System.Collections.Generic;

namespace NSNest.UI
{
    public static class ButtonControllerHelper
    {
        public static void InitButton(this IButtonEventReceiver controller)
        {
            ButtonController buttonController = GetButtonController(controller);
            if (buttonController != null)
                buttonController.OnPressButton += controller.OnPressButton;
        }

        public static void ReleaseButton(this IButtonEventReceiver controller)
        {
            ButtonController buttonController = GetButtonController(controller);
            if (buttonController != null)
                buttonController.OnPressButton -= controller.OnPressButton;
        }

        static ButtonController GetButtonController(IButtonEventReceiver controller)
        {
            ButtonController buttonController = null;
            MonoBehaviour monoBehaviour = (controller as MonoBehaviour);
            if (monoBehaviour != null)
            {
                buttonController = monoBehaviour.GetComponent<ButtonController>();
                if (buttonController == null)
                    buttonController = monoBehaviour.gameObject.AddComponent<ButtonController>();
            }

            return buttonController;
        }
    }
}
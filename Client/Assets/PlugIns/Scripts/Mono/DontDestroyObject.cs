using UnityEngine;
using System.Collections;

namespace NSNest.Common
{
    public class DontDestroyObject : MonoBehaviour
    {
        System.Action<GameObject> OnDestroyEvent = null;
        
        static public T Create<T>(System.Action<GameObject> onDestory = null) where T : MonoBehaviour
        {
            var root = Create(typeof(T).Name, onDestory);
            if (root != null)
            {
                return root.AddComponent<T>();
            }
            else
            {
                return null;
            }
        }

        static public GameObject Create(string title, System.Action<GameObject> onDestory)
        {
            GameObject root = new GameObject("(DontDestroy) " + title);

            if (Application.isPlaying)
                Object.DontDestroyOnLoad(root);
            
            var neverDie = root.AddComponent<DontDestroyObject>();
            neverDie.OnDestroyEvent = onDestory;

            return root;
        }
        
        void OnDestroy()
        {
            if (OnDestroyEvent != null)
                OnDestroyEvent(gameObject);
        }

        void OnApplicationQuit()
        {
            GameObject.DestroyImmediate(gameObject);
        }
    }
}
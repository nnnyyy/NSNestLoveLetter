using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using NSNest.Common;

namespace NSNest.UI
{
    public partial class ViewName
    {
        public string Name { get; private set; }
        public ViewName(string name)
        {
            Name = name;
        }

        public static implicit operator string(ViewName viewName)
        {
            return viewName.Name;
        }
    }

    public enum ViewLoadErrorCode
    {
        None,
        LoadFail,
        Loading,
        CastFail
    }

    public delegate void ViewLoadResult<T>(T loadObject, ViewLoadErrorCode errorCode);
    public delegate void ViewLoadResult(GameObject loadTransform, ViewLoadErrorCode errorCode);

    public delegate void RequestResult(Transform loadedTransform, ViewLoadErrorCode errorCode);

    public class ViewLoader : MonoBehaviour
    {
        static ViewLoader mInstance = null;
        static public ViewLoader Instance
        {
            get
            {
                if (mInstance == null)
                {
                    mInstance = DontDestroyObject.Create<ViewLoader>((delObj) =>
                    {
                        mInstance = null;
                    });
                    AttachEvent();
                }

                return mInstance;
            }
        }

        static System.Action m_EventAction = AttachEvent;

        static void AttachEvent()
        {
            m_EventAction = () => { };

            SceneLoader.Instance.OnSceneLoaded += Instance_OnSceneLoaded;
        }

        private static void Instance_OnSceneLoaded(string sceneName)
        {
            //Do Nothing..
        }

        public static void Load<T>(ViewName viewName, ViewLoadResult<T> result = null) where T : class
        {
            Instance._Load<T>(viewName, result);
        }

        public static void Load(ViewName viewName, ViewLoadResult result = null)
        {
            Instance._Load(viewName, result);
        }

        public void _Load<T>(ViewName viewName, ViewLoadResult<T> result = null) where T : class
        {
            CommonLoad(viewName, (loadedTransform, errorCode) =>
            {
                if (result != null)
                {
                    T view = loadedTransform != null && errorCode == ViewLoadErrorCode.None
                        ? loadedTransform.GetComponent<T>()
                        : null;
                    errorCode = view == null && errorCode == ViewLoadErrorCode.None ? ViewLoadErrorCode.CastFail : errorCode;

                    result((view), errorCode);
                }
            });
        }

        public void _Load(ViewName viewName, ViewLoadResult result = null)
        {
            CommonLoad(viewName, (loadedTransform, errorCode) =>
            {
                if (result != null)
                {
                    result(
                    (loadedTransform != null ? loadedTransform.gameObject : null)
                    , errorCode
                    );
                }
            });
        }

        void CommonLoad(ViewName viewName, RequestResult requestResult)
        {
            m_EventAction();

            if (viewName != null && !string.IsNullOrEmpty(viewName.Name))
            {
                Transform trans = ObjectAccessor.GetDock(viewName.Name);
                if(trans!=null)
                    requestResult(trans, ViewLoadErrorCode.None);
                else
                    LoadProcess(viewName, requestResult);
            }
            else
                requestResult(null, ViewLoadErrorCode.LoadFail);
        }

        Dictionary<ViewName, Queue<RequestResult>> m_DicResourceQueue = new Dictionary<ViewName, Queue<RequestResult>>();
        void LoadProcess(ViewName viewName, RequestResult requestResult)
        {
            Queue<RequestResult> queue = null;
            if (m_DicResourceQueue.TryGetValue(viewName, out queue))
            {
                queue.Enqueue(requestResult);
            }
            else
            {
                queue = new Queue<RequestResult>();
                queue.Enqueue(requestResult);
                m_DicResourceQueue.Add(viewName, queue);
                StartCoroutine(Loading_Routine(Resources.LoadAsync(viewName.Name), (loadTransform, errorCode) => {

                    if (errorCode != ViewLoadErrorCode.None)
                    {
                        if (requestResult != null)
                            requestResult(loadTransform, errorCode);
                        return;
                    }

                    Transform trans = ObjectAccessor.GetDock(viewName.Name);
                    if (trans == null)
                        ObjectAccessor.RegisterDock(viewName.Name, loadTransform);

                    Transform transRoot = ObjectAccessor.GetDock("UI_ROOT");
                    if(transRoot!=null)
                    {
                        loadTransform.parent = transRoot;
                        loadTransform.localPosition = Vector3.zero;
                        loadTransform.localRotation = Quaternion.identity;
                        loadTransform.localScale = Vector3.one;
                    }

                    Queue<RequestResult> waitQueue = null;
                    if (m_DicResourceQueue.TryGetValue(viewName, out waitQueue))
                    {
                        while (waitQueue.Count == 0)
                        {
                            RequestResult waitRequestResult = waitQueue.Dequeue();
                            if (waitRequestResult != null)
                                waitRequestResult(loadTransform, ViewLoadErrorCode.None);
                        }       
                    }
                }));
            }
        }

        IEnumerator Loading_Routine(ResourceRequest resourceRequest, RequestResult requestResult)
        {
            YieldInstruction nextFrame = new WaitForEndOfFrame();

            while (!resourceRequest.isDone)
                yield return nextFrame;

            Object asset = resourceRequest.asset;

            GameObject inst = ((asset != null && asset is GameObject) ?
                GameObject.Instantiate(asset) : asset) as GameObject;

            if (requestResult != null)
            {
                if(inst!=null)
                    requestResult(inst.transform, ViewLoadErrorCode.None);
                else
                    requestResult(null, ViewLoadErrorCode.LoadFail);
            }
                
        }
    }
}

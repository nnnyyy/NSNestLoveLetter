using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using UnityEngine.SceneManagement;

namespace NSNest.Common
{
    /// <summary>
    /// 씬 로딩 시작
    /// </summary>
    public delegate void SceneLoadBegin(string sceneName);
    
    /// <summary>
    /// 씬 로딩 완료
    /// </summary>
    public delegate void SceneLoaded(string sceneName);

    /// <summary>
    /// 코-루틴을 사용하기 위한 컴포넌트
    /// </summary>
    public class SceneLoader : MonoBehaviour, ISceneLoader
    {
        static ISceneLoader mInstance = null;
        static public ISceneLoader Instance
        {
            get
            {
                if (mInstance == null)
                {
                    mInstance = DontDestroyObject.Create<SceneLoader>((delObj) => 
                    {
                        mInstance = null;
                    });
                }

                return mInstance;
            }
        }
        
        private SceneLoadBegin m_OnSceneLoadBegin = null;
        public event SceneLoadBegin OnSceneLoadBegin
        { add { m_OnSceneLoadBegin += value; } remove { m_OnSceneLoadBegin -= value; } }

        private SceneLoaded m_OnSceneLoaded = null;
        public event SceneLoaded OnSceneLoaded
        { add { m_OnSceneLoaded += value; } remove { m_OnSceneLoaded -= value; } }

        public void Load(string sceneName, bool isAdditive = false)
        {
            if (string.IsNullOrEmpty(sceneName))
                return;

            StartCoroutine(Routine_Loading(sceneName, isAdditive));
        }
        
        IEnumerator Routine_Loading(string sceneName, bool isAdditive)
        {
            if (m_OnSceneLoadBegin != null)
                m_OnSceneLoadBegin(sceneName);

            YieldInstruction nextFrame = new WaitForEndOfFrame();

            AsyncOperation operation = SceneManager.LoadSceneAsync(sceneName, 
                isAdditive ? LoadSceneMode.Additive : LoadSceneMode.Single);

            while (!operation.isDone)
                yield return nextFrame;
            
            if (m_OnSceneLoaded != null)
                m_OnSceneLoaded(sceneName);
        }
    }
}
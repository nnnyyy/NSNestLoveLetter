using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class SoundManager : Singleton<SoundManager> {        
    public float lowPitchRange = .95f;              //The lowest a sound effect will be randomly pitched.
    public float highPitchRange = 1.05f;            //The highest a sound effect will be randomly pitched.
    public readonly int MAX_SOURCE_CNT = 10;
    List<AudioSource> liSourcesSfx;
    AudioSource sourceBGM;
    List<AudioClip> clips;
    Dictionary<string, AudioClip> mClips;

    public void Awake()
    {
        liSourcesSfx = new List<AudioSource>();
        for(int i = 0; i < MAX_SOURCE_CNT; ++i)
        {
            liSourcesSfx.Add(gameObject.AddComponent<AudioSource>());
        }

        foreach(AudioSource asource in liSourcesSfx)
        {
            asource.playOnAwake = false;
            asource.loop = false;
        }

        sourceBGM = gameObject.AddComponent<AudioSource>();
        sourceBGM.loop = true;
        sourceBGM.playOnAwake = false;
        LoadClipsFromResources();
    }

    void LoadClipsFromResources() {
        clips = new List<AudioClip>();
        mClips = new Dictionary<string, AudioClip>();
        AudioClip[] acs = Resources.LoadAll< AudioClip>("sounds");
        foreach(AudioClip ac in acs)
        {
            clips.Add(ac);
            mClips.Add(ac.name, ac);
            Debug.Log(ac.name);
        }
    }

    //Used to play single sound clips.
    public void PlaySfx(int _num)
    {
        if (clips == null || clips.Count <= 0) return;
        
        foreach(AudioSource audiosource in liSourcesSfx)
        {
            if (audiosource.isPlaying) continue;
            audiosource.clip = clips[_num];
            audiosource.Play();
            break;
        }
    }

    public void PlaySfx(string sName)
    {
        if (mClips == null || mClips.Count <= 0) return;
        AudioClip ac = null;
        if(!mClips.TryGetValue(sName, out ac))
        {
            return;
        }

        foreach (AudioSource audiosource in liSourcesSfx)
        {
            if (audiosource.isPlaying) continue;
            audiosource.clip = ac;
            audiosource.Play();
            break;
        }
    }

    public void PlayBGM(int _num)
    {
        if (clips == null || clips.Count <= 0) return;
        sourceBGM.clip = clips[_num];
        sourceBGM.Play();
    }

    public void PlayBGM(string sName)
    {        
        if (mClips == null || mClips.Count <= 0) return;
        AudioClip ac = null;
        if (!mClips.TryGetValue(sName, out ac))
        {
            return;
        }
        if (sourceBGM.clip && sourceBGM.clip.name == sName) return;
        sourceBGM.clip = ac;
        sourceBGM.Play();
    }
}

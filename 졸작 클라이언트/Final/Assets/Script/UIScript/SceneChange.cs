using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SceneChange : MonoBehaviour
{

    static protected SceneChange sceneChange;
    static public SceneChange getInstance { get { return sceneChange; } }
    // Use this for initialization

    private void Awake()
    {
        sceneChange = this;

    }
    void Start()
    {
        DontDestroyOnLoad(this);
    }

    // Update is called once per frame
    void Update()
    {

    }

    public void ChangeScene(string SceneName)
    {
        
        SceneManager.LoadScene(SceneName);
    }

    public int GetSceneNumber()
    {
        Scene scene = SceneManager.GetActiveScene();
        return scene.buildIndex;
    }
}


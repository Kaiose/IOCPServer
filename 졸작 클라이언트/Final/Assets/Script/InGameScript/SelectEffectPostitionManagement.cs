using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SelectEffectPostitionManagement : MonoBehaviour {


    public SpriteRenderer spriteRenderer;
    public Transform CurrenntPositition;
  
	// Use this for initialization
	void Start () {
        

	}
	
	// Update is called once per frame
	void Update () {

        Management();

	}

    void Management()
    {
        if (spriteRenderer.flipX)
        {
            CurrenntPositition.localPosition = new Vector3(0.03f, CurrenntPositition.localPosition.y);
        }
        else
        {

            CurrenntPositition.localPosition = new Vector3(-0.03f, CurrenntPositition.localPosition.y);
        }
    }


}

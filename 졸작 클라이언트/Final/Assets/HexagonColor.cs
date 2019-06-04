using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HexagonColor : MonoBehaviour {

    public Color hexagonColor { get { return gameObject.GetComponent<SpriteRenderer>().material.color; } }
    
    
    public float Rvariation = 0.6f;
    public float Gvariation = -0.4f;
    public float Bvariation = 0.2f;

    // Use this for initialization
    void Start () {
    }

    // Update is called once per frame
    void Update () {
        ChangeColor();
	}

    public void ChangeColor()
    {
        gameObject.GetComponent<SpriteRenderer>().material.color = new Color(hexagonColor.r + Rvariation * Time.deltaTime, hexagonColor.g + Gvariation*Time.deltaTime, hexagonColor.b + Bvariation*Time.deltaTime);

        if (hexagonColor.r > 1.0f || hexagonColor.r < 0.0f)
        {
            Rvariation *= -1.0f;
        }

        if (hexagonColor.g > 1.0f || hexagonColor.g < 0.0f)
        {
            Gvariation *= -1.0f;
        }

        if (hexagonColor.b > 1.0f || hexagonColor.b < 0.0f)
        {
            Bvariation *= -1.0f;
        }
    }
}

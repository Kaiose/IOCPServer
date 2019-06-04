using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DummyClient;
public class SetBoundary : MonoBehaviour {

    // Use this for initialization

    public BoxCollider2D boundary;

    private void Awake()
    {



    }
    void Start () {
        GameManager.getInstance.BoundaryTop = boundary.bounds.center.y + boundary.bounds.extents.y;
        GameManager.getInstance.BoundaryBottom = boundary.bounds.center.y - boundary.bounds.extents.y;
        GameManager.getInstance.BoundaryRight = boundary.bounds.extents.x + boundary.bounds.center.x;
        GameManager.getInstance.BoundaryLeft = boundary.bounds.center.x - boundary.bounds.extents.x;


    }

    // Update is called once per frame
    void Update () {
		
	}
}

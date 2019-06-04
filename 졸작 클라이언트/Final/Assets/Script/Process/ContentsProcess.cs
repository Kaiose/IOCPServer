using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
namespace DummyClient
{
    public abstract class ContentsProcess : MonoBehaviour
    {
        
      
        public bool defaultRun(Packetinterface packet)
        {
            
            return false;
        }

        public abstract void parse(Packetinterface packet);


    }
}

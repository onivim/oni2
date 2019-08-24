open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;

let createElement =
    (~children, ()) => 
            <BoxShadow
              boxShadow={Style.BoxShadow.make(
                ~xOffset=-11.,
                ~yOffset=-11.,
                ~blurRadius=25.,
                ~spreadRadius=0.,
                ~color=Color.rgba(0., 0., 0., 0.2),
                (),
              )}>
              ...children
              </BoxShadow>;
  

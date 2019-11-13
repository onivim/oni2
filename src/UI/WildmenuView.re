open Revery.UI;
open Oni_Core;
open Oni_Model;

let make =
    (
      ~state: Quickmenu.t,
      ~configuration: Configuration.t,
      ~theme: Theme.t,
      (),
    ) =>
  <QuickmenuView
    font={GlobalContext.current().state.uiFont}
    theme
    autofocus=false
    configuration
    state
  />;
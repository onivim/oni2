open Revery.UI;
open Oni_Core;
open Oni_Model;

let component = React.component("wildmenu");

let createElement =
    (
      ~children as _,
      ~state: Quickmenu.t,
      ~configuration: Configuration.t,
      ~theme: Theme.t,
      (),
    ) =>
  component(hooks =>
    (
      hooks,
      <QuickmenuView
        font={GlobalContext.current().state.uiFont}
        theme
        autofocus=false
        configuration
        state
      />,
    )
  );

open Revery.UI;
open Oni_Core;
open Oni_Model;

let component = React.component("wildmenu");

let createElement =
    (
      ~children as _,
      ~state: Menu.t,
      ~configuration: Configuration.t,
      ~theme: Theme.t,
      (),
    ) =>
  component(hooks =>
    (
      hooks,
      <MenuView
        font={GlobalContext.current().state.uiFont}
        theme
        autofocus=false
        configuration
        state
      />,
    )
  );

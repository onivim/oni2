open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;
open Oni_Core.Types.UiFont;

let component = React.component("commandline");

let createElement =
    (
      ~children as _,
      ~state: Quickmenu.t,
      ~configuration: Configuration.t,
      ~theme: Theme.t,
      (),
    ) =>
  component(hooks => (
    hooks,
    <QuickmenuView
      font = GlobalContext.current().state.uiFont
      theme = theme
      autofocus = false // We want input to go straight to Vim
      configuration = configuration
      state
      />
  ));


open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;
open Oni_Core.Types.UiFont;

let component = React.component("wildmenu");

let createElement =
    (
      ~children as _,
      ~state: Menu.t,
      ~configuration: Configuration.t,
      ~theme: Theme.t,
      (),
    ) =>
  component(hooks => (
    hooks,
    <MenuView
      font = GlobalContext.current().state.uiFont
      theme = theme
      autofocus = false // We want input to go straight to Vim
      configuration = configuration
      state
      />
  ));
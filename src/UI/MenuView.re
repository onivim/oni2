open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Model;

type state = {
  text: string,
  cursorPosition: int
};

let component = React.component("Menu");

let onSelect = (_) => {
  GlobalContext.current().dispatch(MenuSelect);
};

let onSelectedChange =
  pos => GlobalContext.current().dispatch(MenuPosition(pos));

let createElement =
    (
      ~children as _,
      ~font: Types.UiFont.t,
      ~menu: Menu.t,
      ~theme: Theme.t,
      ~configuration: Configuration.t,
      (),
    ) =>
  component(hooks => {
    let ({ text, cursorPosition }, setState, hooks) =
      Hooks.state({ text: "", cursorPosition: 0 }, hooks);

    let onInput = (str, pos) => {
      setState({ text: str, cursorPosition: pos });
      GlobalContext.current().dispatch(MenuSearch(str));
    };

    (
      hooks,
      menu.isOpen ? 
        <QuickMenuView
          font = font
          theme = theme
          configuration = configuration
          loadingAnimation = menu.loadingAnimation
          text
          cursorPosition
          items = Job(menu.filterJob)
          selected = Some(menu.selectedItem)
          onInput
          onSelectedChange
          onSelect
          />
        : React.empty
    );
  });

open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;
open Oni_Core.Types.UiFont;

let menuWidth = 400;

let component = React.component("commandline");

let getPrefix = (t: Vim.Types.cmdlineType) => {
  switch (t) {
  | SearchForward => "/"
  | SearchReverse => "?"
  | _ => ":"
  };
};

let createElement =
    (
      ~children as _,
      ~command: Commandline.t,
      ~wildmenu: Wildmenu.t,
      ~configuration: Configuration.t,
      ~theme: Theme.t,
      (),
    ) =>
  component(hooks => {
    let uiFont = State.(GlobalContext.current().state.uiFont);

    let items =
      wildmenu.items
        |> List.map(name => Actions.{ name, category: None, icon: None, command: () => Noop })
        |> Array.of_list;

    (
      hooks,
      command.show ?
        <QuickMenuView
          font = uiFont
          theme = theme
          configuration = configuration
          // autofocus=false
          prefix = getPrefix(command.cmdType)
          text = command.text
          cursorPosition = command.position
          items = Array(items)
          selected = Some(wildmenu.selected)
          onInput = ((text, position) => GlobalContext.current().dispatch(KeyboardInput(text.[String.length(text) - 1] |> Char.escaped)))
          onSelectedChange = (index => GlobalContext.current().dispatch(WildmenuSelect(index)))
          onSelect = ((_) => ())
          />
        : React.empty
    );
  });

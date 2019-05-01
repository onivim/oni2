open Oni_Model;
open Revery_UI;
open Revery.UI.Components;

type t =
  | Messages;

type panel = {
  title: string,
  panelType: t,
  command: State.t => unit,
};

let headerTextSize = 15;

let underlineStyles =
  Style.[height(1), backgroundColor(Revery.Colors.white)];

let header = bgColor =>
  Style.[padding(8), backgroundColor(bgColor), flexDirection(`Row)];

let headerText = font =>
  Style.[
    fontFamily(font),
    fontSize(headerTextSize),
    textOverflow(`Ellipsis),
  ];

let heading =
  Style.[width(80), justifyContent(`Center), marginHorizontal(5)];

let component = React.component("Panels");

let panels = [
  {
    title: "Messages",
    panelType: Messages,
    command: _state => GlobalContext.current().dispatch(GetMessages),
  },
];

let createElement = (~children, ~state: State.t, ()) =>
  component(hooks => {
    let font = state.uiFont.fontFile;
    let bg = state.theme.colors.editorWhitespaceForeground;

    let (activePanel, setActivePanel, hooks) =
      React.Hooks.state(Messages, hooks);

    let (hidePanels, setHidePanels, hooks) = React.Hooks.state(false, hooks);

    let onClick = (panel, _) => {
      panel.command(state);
      setActivePanel(panel.panelType);
    };

    (
      hooks,
      !hidePanels
        ? <View>
            <View style={header(bg)}>
              ...{List.map(
                panel =>
                  <Clickable style=heading onClick={onClick(panel)}>
                    <Text text={panel.title} style={headerText(font)} />
                  </Clickable>,
                panels,
              )}
            </View>
            <View>
              {switch (activePanel) {
               | Messages => <MessagesView state />
               }}
            </View>
          </View>
        : React.empty,
    );
  });

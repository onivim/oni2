open Oni_Model;
open Revery_UI;
open Revery.UI.Components;

type t =
  | Messages;

type panel = {
  title: string,
  panelType: t,
};

let headerTextSize = 15;

let underlineStyles =
  Style.[height(1), backgroundColor(Revery.Colors.white)];

let header = Style.[padding(8)];

let headerText = font =>
  Style.[
    fontFamily(font),
    fontSize(headerTextSize),
    textOverflow(`Ellipsis),
  ];

let heading = Style.[width(80), justifyContent(`Center)];

let component = React.component("Panels");

let panels = [{title: "Messages", panelType: Messages}];

let createElement = (~children, ~state: State.t, ()) =>
  component(hooks => {
    let font = state.uiFont.fontFile;

    let (activePanel, setActivePanel, hooks) =
      React.Hooks.state(Messages, hooks);
    let (hidePanels, setHidePanels, hooks) = React.Hooks.state(false, hooks);

    (
      hooks,
      !hidePanels
        ? <View>
            <View style=header>
              ...{List.map(
                panel =>
                  <Clickable
                    style=heading
                    onClick={() => setActivePanel(panel.panelType)}>
                    <Text text={panel.title} style={headerText(font)} />
                    <View style=underlineStyles />
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

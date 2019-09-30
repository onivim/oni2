open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Model;

let component = React.component("Menu");

let menuWidth = 400;
let menuHeight = 320;

let containerStyles = (theme: Theme.t) =>
  Style.[
    backgroundColor(theme.editorMenuBackground),
    color(theme.editorMenuForeground),
  ];

let menuItemStyle =
  Style.[
    fontSize(14),
    width(menuWidth - 50),
    cursor(Revery.MouseCursors.pointer),
  ];

let inputStyles = font =>
  Style.[
    border(~width=2, ~color=Color.rgba(0., 0., 0., 0.1)),
    backgroundColor(Color.rgba(0., 0., 0., 0.3)),
    width(menuWidth - 10),
    color(Colors.white),
    fontFamily(font),
  ];

let handleChange = str => GlobalContext.current().dispatch(MenuSearch(str));

let handleKeyDown = (event: NodeEvents.keyEventParams) =>
  switch (event.keycode) {
  | v when v == 1073741905 /*Key.Keycode.down*/ =>
          GlobalContext.current().dispatch(MenuNextItem)
  | v when v == 1073741906 /*Key.Keycode.up*/ =>
          GlobalContext.current().dispatch(MenuPreviousItem)
  | _ => ()
  };

let loseFocusOnClose = isOpen =>
  /**
   TODO: revery-ui/revery#412 if the menu is hidden abruptly the element is not automatically unfocused
   as revery is unaware the element is no longer in focus
 */
  (
    switch (Focus.focused, isOpen) {
    | ({contents: Some(_)}, false) => Focus.loseFocus()
    | (_, _) => ()
    }
  );

let onClick = () => {
  GlobalContext.current().dispatch(MenuSelect);
};

let onMouseOver = pos => GlobalContext.current().dispatch(MenuPosition(pos));

type fontT = Types.UiFont.t;

let getLabel = (command: Actions.menuCommand) => {
  switch (command.category) {
  | Some(v) => v ++ ": " ++ command.name
  | None => command.name
  };
};

let createElement =
    (
      ~children as _,
      ~font: fontT,
      ~menu: Menu.t,
      ~theme: Theme.t,
      ~configuration: Configuration.t,
      (),
    ) =>
  component(hooks => {
    let hooks =
      React.Hooks.effect(
        Always,
        () => {
          loseFocusOnClose(menu.isOpen);
          None;
        },
        hooks,
      );

    let commands = Job.getCompletedWork(menu.filterJob).uiFiltered;
    let time = Time.getTime() |> Time.to_float_seconds;

    let jobProgress = Job.getProgress(menu.filterJob);

    let loadingOpacityAnimation = Animation.getValue(menu.loadingAnimation);
    let loadingSpinner =
      menu.isLoading
        ? <View style=Style.[height(40), width(menuWidth)]>
            <Center>
              <View
                style=Style.[
                  transform(
                    Transform.[RotateY(Math.Angle.Radians(time *. 2.))],
                  ),
                ]>
                <Opacity opacity=loadingOpacityAnimation>
                  <Container
                    width=10
                    height=10
                    color={theme.oniNormalModeBackground}
                  />
                </Opacity>
              </View>
            </Center>
          </View>
        : <Opacity opacity=0.3>
            <View style=Style.[height(2), width(menuWidth)]>
              <View
                style=Style.[
                  height(2),
                  width(
                    1
                    + (
                      int_of_float(float_of_int(menuWidth) *. jobProgress)
                      - 1
                    ),
                  ),
                  backgroundColor(theme.oniNormalModeBackground),
                ]
              />
            </View>
          </Opacity>;

    React.(
      hooks,
      menu.isOpen
        ? <AllowPointer>
            <OniBoxShadow configuration theme>
              <View style={containerStyles(theme)}>
                <View style=Style.[width(menuWidth), padding(5)]>
                  <OniInput
                    autofocus=true
                    placeholder="type here to search the menu"
                    cursorColor=Colors.white
                    style={inputStyles(font.fontFile)}
                    onChange=handleChange
                    onKeyDown=handleKeyDown
                  />
                </View>
                <View>
                  <FlatList
                    rowHeight=40
                    height=menuHeight
                    width=menuWidth
                    count={Array.length(commands)}
                    render={index => {
                      let cmd = commands[index];
                      <MenuItem
                        onClick
                        theme
                        style=menuItemStyle
                        label={getLabel(cmd)}
                        icon={cmd.icon}
                        onMouseOver={_ => onMouseOver(index)}
                        selected={index == menu.selectedItem}
                      />;
                    }}
                  />
                  loadingSpinner
                </View>
              </View>
            </OniBoxShadow>
          </AllowPointer>
        : React.listToElement([]),
    );
  });

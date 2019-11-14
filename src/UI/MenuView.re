open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Model;

type state = {
  text: string,
  cursorPosition: int,
};

let component = React.component("Menu");

module Constants = {
  let menuWidth = 400;
  let menuHeight = 320;
};

module Styles = {
  let container = (theme: Theme.t) =>
    Style.[
      backgroundColor(theme.menuBackground),
      color(theme.menuForeground),
    ];

  let input = font =>
    Style.[
      border(~width=2, ~color=Color.rgba(0., 0., 0., 0.1)),
      backgroundColor(Color.rgba(0., 0., 0., 0.3)),
      width(Constants.menuWidth - 10),
      color(Colors.white),
      fontFamily(font),
    ];

  let menuItem =
    Style.[
      fontSize(14),
      width(Constants.menuWidth - 50),
      cursor(Revery.MouseCursors.pointer),
    ];

  let label =
      (~font: Types.UiFont.t, ~theme: Theme.t, ~highlighted, ~isSelected) =>
    Style.[
      fontFamily(font.fontFile),
      textOverflow(`Ellipsis),
      fontSize(12),
      backgroundColor(
        isSelected ? theme.menuSelectionBackground : theme.menuBackground,
      ),
      color(
        highlighted ? theme.oniNormalModeBackground : theme.menuForeground,
      ),
      textWrap(TextWrapping.NoWrap),
    ];
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

let onSelect = _ => GlobalContext.current().dispatch(MenuSelect);

let onSelectedChange = index =>
  GlobalContext.current().dispatch(MenuPosition(index));

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

    let ({text, cursorPosition}, setState, hooks) =
      Hooks.state({text: "", cursorPosition: 0}, hooks);

    let handleChange = (str, pos) => {
      setState({text: str, cursorPosition: pos});
      GlobalContext.current().dispatch(MenuSearch(str));
    };

    let handleKeyDown = (event: NodeEvents.keyEventParams) =>
      switch (event.keycode) {
      | v when v == 1073741905 /*Key.Keycode.down*/ =>
        GlobalContext.current().dispatch(MenuNextItem)
      | v when v == 1073741906 /*Key.Keycode.up*/ =>
        GlobalContext.current().dispatch(MenuPreviousItem)
      | _ => ()
      };

    let items = Job.getCompletedWork(menu.filterJob).uiFiltered;
    let time = Time.getTime() |> Time.to_float_seconds;

    let jobProgress = Job.getProgress(menu.filterJob);

    let loadingOpacityAnimation = Animation.getValue(menu.loadingAnimation);
    let loadingSpinner =
      menu.isLoading
        ? <View style=Style.[height(40), width(Constants.menuWidth)]>
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
            <View style=Style.[height(2), width(Constants.menuWidth)]>
              <View
                style=Style.[
                  height(2),
                  width(
                    1
                    + (
                      int_of_float(
                        float_of_int(Constants.menuWidth) *. jobProgress,
                      )
                      - 1
                    ),
                  ),
                  backgroundColor(theme.oniNormalModeBackground),
                ]
              />
            </View>
          </Opacity>;

    let renderItem = index => {
      let item = items[index];
      let isSelected = index == menu.selectedItem;

      let labelView = {
        let style = Styles.label(~font, ~theme, ~isSelected);

        let highlighted = {
          let text = MenuJob.getLabel(item);
          let textLength = String.length(text);

          // Assumes ranges are sorted low to high
          let rec highlighter = last =>
            fun
            | [] => [
                <Text
                  style={style(~highlighted=false)}
                  text={String.sub(text, last, textLength - last)}
                />,
              ]

            | [(low, high), ...rest] => [
                <Text
                  style={style(~highlighted=false)}
                  text={String.sub(text, last, low - last)}
                />,
                <Text
                  style={style(~highlighted=true)}
                  text={String.sub(text, low, high + 1 - low)}
                />,
                ...highlighter(high + 1, rest),
              ];

          highlighter(0, item.highlight);
        };

        <View style=Style.[flexDirection(`Row)]> ...highlighted </View>;
      };

      <MenuItem
        onClick={() => onSelect(index)}
        theme
        style=Styles.menuItem
        label={`Custom(labelView)}
        icon={item.icon}
        onMouseOver={() => onSelectedChange(index)}
        isSelected
      />;
    };

    (
      hooks,
      menu.isOpen
        ? <AllowPointer>
            <OniBoxShadow configuration theme>
              <View style={Styles.container(theme)}>
                <View style=Style.[width(Constants.menuWidth), padding(5)]>
                  <OniInput
                    autofocus=true
                    placeholder="type here to search the menu"
                    cursorColor=Colors.white
                    style={Styles.input(font.fontFile)}
                    onChange=handleChange
                    onKeyDown=handleKeyDown
                    text
                    cursorPosition
                  />
                </View>
                <View>
                  <FlatList
                    rowHeight=40
                    height=Constants.menuHeight
                    width=Constants.menuWidth
                    count={Array.length(items)}
                    selected={Some(menu.selectedItem)}
                    render=renderItem
                  />
                  loadingSpinner
                </View>
              </View>
            </OniBoxShadow>
          </AllowPointer>
        : React.listToElement([]),
    );
  });

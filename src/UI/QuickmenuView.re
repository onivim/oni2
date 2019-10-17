
open Revery;
open Revery.UI;
open Revery.UI.Components;
open Oni_Core;
open Oni_Model;

module Constants = {
  let menuWidth = 400;
  let menuHeight = 320;
};

let component = React.component("Menu");

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


module Styles = {
  let container = (theme: Theme.t) =>
    Style.[
      backgroundColor(theme.editorMenuBackground),
      color(theme.editorMenuForeground),
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

  let label = (~font, ~fg) =>
    Style.[
      fontFamily(font),
      textOverflow(`Ellipsis),
      fontSize(12),
      color(fg),
      textWrap(TextWrapping.NoWrap)
    ];
};


let onSelectedChange = index =>
  GlobalContext.current().dispatch(MenuFocus(index));

let onInput = (text, cursorPosition) =>
  GlobalContext.current().dispatch(MenuInput({ text, cursorPosition }));

let onSelect = (_) =>
  GlobalContext.current().dispatch(MenuSelect);


let createElement =
    (
      ~children as _,
      ~font: Types.UiFont.t,
      ~theme: Theme.t,
      ~configuration: Configuration.t,
      ~autofocus: bool=true,
      ~state: Quickmenu.t,
      ~placeholder: string="type here to search the menu",
      ~onInput: (string, int) => unit = onInput,
      ~onSelectedChange: int => unit = onSelectedChange,
      ~onSelect: int => unit = onSelect,
      (),
    ) =>
  component(hooks => {
    let Quickmenu.{source, selected, text, cursorPosition, prefix} = state;

    // TODO: Cam this be removed in favor of NotifyKeyPressed in MenuStoreConnector?
    let handleKeyDown = (event: NodeEvents.keyEventParams) =>
      switch (event) {
      | {key: Revery.Key.KEY_DOWN, _} =>
        GlobalContext.current().dispatch(MenuFocusNext)
      | {key: Revery.Key.KEY_UP, _} =>
        GlobalContext.current().dispatch(MenuFocusPrevious)
      | _ => ()
      };

    let (items, jobProgress) =
      switch (source) {
        | Loading =>
          ([||], 0.)

        | Progress({ items, progress }) =>
          (items, progress)

        | Complete(items) =>
          (items, 1.)
      };

    let loadingSpinner =
      if (jobProgress == 0.) {
        <AnimatedView duration=2.>
          ...(opacity =>
            <View style=Style.[height(40), width(Constants.menuWidth)]>
              <Center>
                <AnimatedView duration=(2. *. Float.pi) repeat=true>
                  ...(t =>
                    <View
                      style=Style.[
                        transform(
                          Transform.[RotateY(Math.Angle.Radians(t *. 2.))],
                        ),
                      ]>
                      <Opacity opacity>
                        <Container
                          width=10
                          height=10
                          color={theme.oniNormalModeBackground}
                        />
                      </Opacity>
                    </View>
                  )
                </AnimatedView>
              </Center>
            </View>
          )
        </AnimatedView>
      } else {
        <Opacity opacity=0.3>
          <View style=Style.[height(2), width(Constants.menuWidth)]>
            <View
              style=Style.[
                height(2),
                width(
                  1
                  + (
                    int_of_float(float_of_int(Constants.menuWidth) *. jobProgress)
                    - 1
                  ),
                ),
                backgroundColor(theme.oniNormalModeBackground),
              ]
            />
          </View>
        </Opacity>
      };

    let renderItem = index => {
      let item = items[index];

      let labelView = {
        let font = GlobalContext.current().state.uiFont.fontFile;
        let normalStyle = Styles.label(~font, ~fg=theme.editorMenuForeground);
        let highlightedStyle = Styles.label(~font, ~fg=theme.oniNormalModeBackground);

        let highlighted = {
          let text =
            Quickmenu.getLabel(item);
          let textLength =
            String.length(text);

          // Assumes ranges are sorted low to high
          let rec highlighter = last => fun
            | [] =>
              [ <Text
                  style=normalStyle
                  text=String.sub(text, last, textLength - last) />
              ]

            | [(low, high), ...rest] =>
              [ <Text
                  style=normalStyle
                  text=String.sub(text, last, low - last) />,
                <Text
                  style=highlightedStyle
                  text=String.sub(text, low, high + 1 - low) />,
                ...highlighter(high + 1, rest)
              ];

          highlighter(0, item.highlight);
        };

        <View style= Style.[flexDirection(`Row)]>
          ...highlighted
        </View>
      };

      <MenuItem
        onClick={() => onSelect(index)}
        theme
        style=Styles.menuItem
        label=`Custom(labelView)
        icon=item.icon
        onMouseOver={() => onSelectedChange(index)}
        selected={index == selected}
      />;
    };

    (
      hooks,
      <AllowPointer>
        <OniBoxShadow configuration theme>
          <View style={Styles.container(theme)}>
            <View style=Style.[width(Constants.menuWidth), padding(5)]>
              <OniInput
                autofocus
                placeholder
                ?prefix
                cursorColor=Colors.white
                style={Styles.input(font.fontFile)}
                onChange=onInput
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
                selected
                render=renderItem
              />
              loadingSpinner
            </View>
          </View>
        </OniBoxShadow>
      </AllowPointer>
    );
  });

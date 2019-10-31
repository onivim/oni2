open Revery;
open Revery.UI;
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

  let progressBarTrack =
    Style.[height(2), width(Constants.menuWidth), overflow(`Hidden)];

  let progressBarIndicator = (~width as barWidth, ~offset, ~theme: Theme.t) =>
    Style.[
      height(2),
      width(barWidth),
      transform(Transform.[TranslateX(offset)]),
      backgroundColor(theme.oniNormalModeBackground),
    ];
};

let onSelectedChange = index =>
  GlobalContext.current().dispatch(MenuFocus(index));

let onInput = (text, cursorPosition) =>
  GlobalContext.current().dispatch(MenuInput({text, cursorPosition}));

let onSelect = _ => GlobalContext.current().dispatch(MenuSelect);

let progressBar = (~children as _, ~progress=?, ~theme, ()) => {
  let indicatorWidth = 100.;
  let menuWidth = float_of_int(Constants.menuWidth);
  let trackWidth = menuWidth +. indicatorWidth;
  <AnimatedView duration=1.5 repeat=true>
    ...{t => {
      let (width, offset) =
        switch (progress) {
        | Some(progress) => (int_of_float(menuWidth *. progress), 0.)
        | None => (
            int_of_float(indicatorWidth),
            trackWidth *. t -. indicatorWidth,
          )
        };

      <View style=Styles.progressBarTrack>
        <View style={Styles.progressBarIndicator(~width, ~offset, ~theme)} />
      </View>;
    }}
  </AnimatedView>;
};

let createElement =
    (
      ~children as _,
      ~font: Types.UiFont.t,
      ~theme: Theme.t,
      ~configuration: Configuration.t,
      ~autofocus: bool=true,
      ~state: Menu.t,
      ~placeholder: string="type here to search the menu",
      ~onInput: (string, int) => unit=onInput,
      ~onSelectedChange: int => unit=onSelectedChange,
      ~onSelect: int => unit=onSelect,
      (),
    ) =>
  component(hooks => {
    let Menu.{
          items,
          filterProgress,
          ripgrepProgress,
          selected,
          text,
          cursorPosition,
          prefix,
          _,
        } = state;

    let progress =
      Actions.(
        switch (filterProgress, ripgrepProgress) {
        | (Loading, _)
        | (_, Loading) => Loading

        | (InProgress(a), InProgress(b)) => InProgress((a +. b) /. 2.)

        | (InProgress(value), _)
        | (_, InProgress(value)) => InProgress(value)

        | (Complete, Complete) => Complete
        }
      );

    let renderItem = index => {
      let item = items[index];
      let isSelected = Some(index) == selected;

      let labelView = {
        let style = Styles.label(~font, ~theme, ~isSelected);

        let highlighted = {
          let text = Menu.getLabel(item);
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
              {switch (progress) {
               | Complete => React.empty
               | InProgress(progress) => <progressBar progress theme />
               | Loading => <progressBar theme />
               }}
            </View>
          </View>
        </OniBoxShadow>
      </AllowPointer>,
    );
  });

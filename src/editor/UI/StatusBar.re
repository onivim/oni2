/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Model;

open Oni_Model.StatusBarModel;

let component = React.component("StatusBar");

let getTextStyle = uiFont => {
  Oni_Core.Types.UiFont.(
    Style.[
      fontFamily(uiFont.fontFile),
      fontSize(11),
      textWrap(TextWrapping.NoWrap),
    ]
  );
};

let viewStyle =
  Style.[
    flexDirection(`Row),
    alignItems(`Center),
    flexGrow(1),
    justifyContent(`FlexEnd),
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
  ];

let convertPositionToString = (position: option(Types.Position.t)) =>
  switch (position) {
  | Some(v) =>
    string_of_int(Types.Index.toOneBasedInt(v.line))
    ++ ","
    ++ string_of_int(Types.Index.toOneBasedInt(v.character))
  | None => ""
  };

module StatusBarSection = {
  let component = React.component("StatusBarSection");

  let createElement = (~children, ~direction, ()) =>
    component(hooks =>
      (
        hooks,
        <View
          style=Style.[
            flexDirection(`Row),
            justifyContent(direction),
            alignItems(direction),
            flexGrow(1),
          ]>
          ...children
        </View>,
      )
    );
};

module StatusBarItem = {
  let component = React.component("StatusBarItem");

  let getStyle = (h, bg: Color.t) =>
    Style.[
      flexDirection(`Column),
      justifyContent(`Center),
      alignItems(`Center),
      height(h),
      backgroundColor(bg),
      paddingHorizontal(10),
      minWidth(50),
    ];

  let createElement = (~children, ~height, ~backgroundColor, ()) =>
    component(hooks =>
      (
        hooks,
        <View style={getStyle(height, backgroundColor)}> ...children </View>,
      )
    );
};

let createElement = (~children as _, ~height, ~state: State.t, ()) =>
  component(hooks => {
    let mode = state.mode;
    let theme = state.theme;
    let editor =
      Selectors.getActiveEditorGroup(state) |> Selectors.getActiveEditor;
    let position =
      switch (editor) {
      | Some(v) => Some(v.cursorPosition)
      | None => None
      };

    let textStyle = getTextStyle(state.uiFont);

    let (background, foreground) = Theme.getColorsForMode(theme, mode);

    let toStatusBarElement = (statusBarItem: Item.t) => {
      <StatusBarItem height backgroundColor={theme.statusBarBackground}>
        <Text
          style=Style.[
            backgroundColor(theme.statusBarBackground),
            color(theme.statusBarForeground),
            ...textStyle,
          ]
          text={statusBarItem.text}
        />
      </StatusBarItem>;
    };

    let filterFunction = (alignment: Alignment.t, item: Item.t) => {
      item.alignment === alignment;
    };

    let buffer = Selectors.getActiveBuffer(state);
    let fileType =
      switch (buffer) {
      | Some(v) =>
        switch (Buffer.getMetadata(v).filePath) {
        | Some(fp) =>
          LanguageInfo.getLanguageFromFilePath(state.languageInfo, fp)
        | None => "plaintext"
        }
      | None => "plaintext"
      };

    let statusBarItems = state.statusBar;
    let leftItems =
      statusBarItems
      |> List.filter(filterFunction(Alignment.Left))
      |> List.map(toStatusBarElement);

    let rightItems =
      statusBarItems
      |> List.filter(filterFunction(Alignment.Right))
      |> List.map(toStatusBarElement);

    let indentation =
      Indentation.getForActiveBuffer(state) |> Indentation.toStatusString;

    (
      hooks,
      <View style=viewStyle>
        <StatusBarSection direction=`FlexStart>
          ...leftItems
        </StatusBarSection>
        <StatusBarSection direction=`Center />
        <StatusBarSection direction=`FlexEnd> ...rightItems </StatusBarSection>
        <StatusBarSection direction=`FlexEnd>
          <StatusBarItem
            height backgroundColor={theme.statusBarBackground}>
            <Text
              style=Style.[
                backgroundColor(theme.statusBarBackground),
                color(theme.statusBarForeground),
                ...textStyle,
              ]
              text=indentation
            />
          </StatusBarItem>
          <StatusBarItem
            height backgroundColor={theme.statusBarBackground}>
            <Text
              style=Style.[
                backgroundColor(theme.statusBarBackground),
                color(theme.statusBarForeground),
                ...textStyle,
              ]
              text=fileType
            />
          </StatusBarItem>
          <StatusBarItem
            height backgroundColor={theme.statusBarBackground}>
            <Text
              style=Style.[
                backgroundColor(theme.statusBarBackground),
                color(theme.statusBarForeground),
                ...textStyle,
              ]
              text={convertPositionToString(position)}
            />
          </StatusBarItem>
          <StatusBarItem height backgroundColor=background>
            <Text
              style=Style.[
                backgroundColor(background),
                color(foreground),
                ...textStyle,
              ]
              text={Vim.Mode.show(mode)}
            />
          </StatusBarItem>
        </StatusBarSection>
      </View>,
    );
  });

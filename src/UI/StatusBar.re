/*
 * StatusBar.re
 *
 * Container for StatusBar
 */

open EditorCoreTypes;
open Revery;
open Revery.UI;

open Oni_Core;
open Oni_Model;

open Oni_Model.StatusBarModel;

let getTextStyle = uiFont => {
  UiFont.(
    Style.[
      fontFamily(uiFont.fontFile),
      fontSize(11),
      textWrap(TextWrapping.NoWrap),
    ]
  );
};

let viewStyle = (bgColor, transition) =>
  Style.[
    backgroundColor(bgColor),
    flexDirection(`Row),
    alignItems(`Center),
    flexGrow(1),
    justifyContent(`FlexEnd),
    position(`Absolute),
    top(0),
    bottom(0),
    left(0),
    right(0),
    transform(Transform.[TranslateY(transition)]),
  ];

let convertPositionToString =
  fun
  | Some((loc: Location.t)) =>
    Printf.sprintf(
      "%n,%n",
      Index.toOneBased(loc.line),
      Index.toOneBased(loc.column),
    )
  | None => "";

module StatusBarSection = {
  let make = (~children=React.empty, ~direction, ()) =>
    <View
      style=Style.[
        flexDirection(`Row),
        justifyContent(direction),
        alignItems(direction),
        flexGrow(1),
      ]>
      children
    </View>;
};

module StatusBarItem = {
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

  let make = (~children, ~height, ~backgroundColor, ()) =>
    <View style={getStyle(height, backgroundColor)}> children </View>;
};

let animation =
  Revery.UI.Animation.(
    animate(Revery.Time.milliseconds(150))
    |> ease(Easing.ease)
    |> tween(50.0, 0.)
    |> delay(Revery.Time.milliseconds(0))
  );

let%component make = (~height, ~state: State.t, ()) => {
  let mode = state.mode;
  let theme = state.theme;
  let editor =
    Selectors.getActiveEditorGroup(state) |> Selectors.getActiveEditor;

  let position = editor |> Utility.Option.map(Editor.getPrimaryCursor);

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

  let%hook (transition, _animationState, _reset) =
    Hooks.animation(animation, ~active=true);

  let filterFunction = (alignment: Alignment.t, item: Item.t) => {
    item.alignment === alignment;
  };

  let buffer = Selectors.getActiveBuffer(state);
  let fileType =
    switch (buffer) {
    | Some(v) =>
      switch (Buffer.getFileType(v)) {
      | Some(fp) => fp
      | None => "plaintext"
      }
    | None => "plaintext"
    };

  let statusBarItems = state.statusBar;
  let leftItems =
    statusBarItems
    |> List.filter(filterFunction(Alignment.Left))
    |> List.map(toStatusBarElement);

  let diagnosticsCount =
    state.diagnostics |> Diagnostics.count |> string_of_int;

  let diagnosticsItem =
    <StatusBarItem height backgroundColor={theme.statusBarBackground}>
      <View
        style=Style.[
          flexDirection(`Row),
          justifyContent(`Center),
          alignItems(`Center),
        ]>
        <FontIcon
          icon=FontAwesome.timesCircle
          backgroundColor={theme.statusBarBackground}
          color={theme.statusBarForeground}
          margin=4
        />
        <Text
          style=Style.[
            backgroundColor(theme.statusBarBackground),
            color(theme.statusBarForeground),
            ...textStyle,
          ]
          text=diagnosticsCount
        />
      </View>
    </StatusBarItem>;

  let leftItems = leftItems @ [diagnosticsItem];

  let leftItems = leftItems |> React.listToElement;

  let rightItems =
    statusBarItems
    |> List.filter(filterFunction(Alignment.Right))
    |> List.map(toStatusBarElement)
    |> React.listToElement;

  let indentation =
    Indentation.getForActiveBuffer(state) |> Indentation.toStatusString;

  <View style={viewStyle(theme.statusBarBackground, transition)}>
    <StatusBarSection direction=`FlexStart> leftItems </StatusBarSection>
    <StatusBarSection direction=`Center />
    <StatusBarSection direction=`FlexEnd> rightItems </StatusBarSection>
    <StatusBarSection direction=`FlexEnd>
      <StatusBarItem height backgroundColor={theme.statusBarBackground}>
        <Text
          style=Style.[
            backgroundColor(theme.statusBarBackground),
            color(theme.statusBarForeground),
            ...textStyle,
          ]
          text=indentation
        />
      </StatusBarItem>
      <StatusBarItem height backgroundColor={theme.statusBarBackground}>
        <Text
          style=Style.[
            backgroundColor(theme.statusBarBackground),
            color(theme.statusBarForeground),
            ...textStyle,
          ]
          text=fileType
        />
      </StatusBarItem>
      <StatusBarItem height backgroundColor={theme.statusBarBackground}>
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
  </View>;
};

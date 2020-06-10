type windowDisplayMode =
  | Minimized
  | Windowed
  | Maximized
  | Fullscreen;

[@deriving show]
type msg =
  | WindowMinimizeClicked
  | WindowMaximizeClicked
  | WindowRestoreClicked
  | WindowCloseClicked
  | TitleDoubleClicked;

// VIEW
open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Components;

module UiFont = Oni_Core.UiFont;
module Colors = Feature_Theme.Colors.TitleBar;

module Styles = {
  open Style;

  module Mac = {
    let container = (~isFocused, ~theme) => [
      flexGrow(0),
      height(25),
      backgroundColor(
        isFocused
          ? Colors.activeBackground.from(theme)
          : Colors.inactiveBackground.from(theme),
      ),
      flexDirection(`Row),
      justifyContent(`Center),
      alignItems(`Center),
    ];
    let text = (~isFocused, ~theme) => [
      flexGrow(0),
      backgroundColor(
        isFocused
          ? Colors.activeBackground.from(theme)
          : Colors.inactiveBackground.from(theme),
      ),
      color(
        isFocused
          ? Colors.activeForeground.from(theme)
          : Colors.inactiveForeground.from(theme),
      ),
      textWrap(TextWrapping.NoWrap),
    ];
  };

  module Windows = {
    let container = (~isFocused, ~theme) => [
      flexGrow(0),
      height(30),
      backgroundColor(
        isFocused
          ? Colors.activeBackground.from(theme)
          : Colors.inactiveBackground.from(theme),
      ),
      flexDirection(`Row),
      justifyContent(`SpaceBetween),
      alignItems(`Center),
    ];

    let iconAndTitle = [
      flexDirection(`Row),
      alignItems(`Center),
      marginHorizontal(16),
    ];

    let title = (~isFocused, ~theme) => [
      flexGrow(0),
      marginLeft(16),
      marginTop(2),
      backgroundColor(
        isFocused
          ? Colors.activeBackground.from(theme)
          : Colors.inactiveBackground.from(theme),
      ),
      color(
        isFocused
          ? Colors.activeForeground.from(theme)
          : Colors.inactiveForeground.from(theme),
      ),
      textWrap(TextWrapping.NoWrap),
    ];

    let buttons = [flexDirection(`Row), alignItems(`Center)];

    module Button = {
      let container = [
        height(30),
        width(46),
        justifyContent(`Center),
        alignItems(`Center),
      ];

      let hoverClose = (~theme) => [
        height(30),
        width(46),
        backgroundColor(Colors.hoverCloseBackground.from(theme)),
        justifyContent(`Center),
        alignItems(`Center),
      ];

      let hover = (~theme) => [
        height(30),
        width(46),
        backgroundColor(Colors.hoverBackground.from(theme)),
        justifyContent(`Center),
        alignItems(`Center),
      ];
    };
  };
};

module View = {
  module Mac = {
    let make =
        (
          ~dispatch,
          ~isFocused,
          ~windowDisplayMode,
          ~title,
          ~theme,
          ~font: UiFont.t,
          (),
        ) =>
      if (windowDisplayMode == Fullscreen) {
        React.empty;
      } else {
        <Clickable
          onDoubleClick={_ => dispatch(TitleDoubleClicked)}
          style={Styles.Mac.container(~isFocused, ~theme)}>
          <Text
            style={Styles.Mac.text(~isFocused, ~theme)}
            fontFamily={font.family}
            fontWeight=Medium
            fontSize=12.
            text=title
          />
        </Clickable>;
      };
  };

  module Windows = {
    module Buttons = {
      module Close = {
        let%component make = (~dispatch, ~theme, ()) => {
          let%hook (isHovered, setHovered) = Hooks.state(false);

          let onMouseUp = _ => dispatch(WindowCloseClicked);

          <View
            style={
              isHovered
                ? Styles.Windows.Button.hoverClose(~theme)
                : Styles.Windows.Button.container
            }
            onMouseUp
            onMouseEnter={_ => setHovered(_ => true)}
            onMouseLeave={_ => setHovered(_ => false)}>
            <Codicon
              icon=Codicon.chromeClose
              color={Colors.activeForeground.from(theme)}
              fontSize=14.
            />
          </View>;
        };
      };

      module MaximizeRestore = {
        let%component make = (~dispatch, ~theme, ~windowDisplayMode, ()) => {
          let%hook (isHovered, setHovered) = Hooks.state(false);

          let onMouseUp = _ =>
            switch (windowDisplayMode) {
            | Maximized => dispatch(WindowRestoreClicked)
            | _ => dispatch(WindowMaximizeClicked)
            };

          <View
            style={
              isHovered
                ? Styles.Windows.Button.hover(~theme)
                : Styles.Windows.Button.container
            }
            onMouseUp
            onMouseEnter={_ => setHovered(_ => true)}
            onMouseLeave={_ => setHovered(_ => false)}>
            {windowDisplayMode == Maximized
               ? <Codicon
                   icon=Codicon.chromeRestore
                   color={Colors.activeForeground.from(theme)}
                   fontSize=14.
                 />
               : <Codicon
                   icon=Codicon.chromeMaximize
                   color={Colors.activeForeground.from(theme)}
                   fontSize=14.
                 />}
          </View>;
        };
      };

      module Minimize = {
        let%component make = (~dispatch, ~theme, ()) => {
          let%hook (isHovered, setHovered) = Hooks.state(false);

          let onMouseUp = _ => dispatch(WindowMinimizeClicked);

          <View
            style={
              isHovered
                ? Styles.Windows.Button.hover(~theme)
                : Styles.Windows.Button.container
            }
            onMouseUp
            onMouseEnter={_ => setHovered(_ => true)}
            onMouseLeave={_ => setHovered(_ => false)}>
            <Codicon
              icon=Codicon.chromeMinimize
              color={Colors.activeForeground.from(theme)}
              fontSize=14.
            />
          </View>;
        };
      };
    };

    let make =
        (
          ~dispatch,
          ~isFocused,
          ~windowDisplayMode,
          ~title,
          ~theme,
          ~font: UiFont.t,
          (),
        ) =>
      <View
        mouseBehavior=Draggable
        style={Styles.Windows.container(~isFocused, ~theme)}>
        <View mouseBehavior=Draggable style=Styles.Windows.iconAndTitle>
          <Image src=`File("./logo-titlebar.png") width=18 height=18 />
          <Text
            style={Styles.Windows.title(~isFocused, ~theme)}
            fontFamily={font.family}
            fontSize=12.
            text=title
          />
        </View>
        <View style=Styles.Windows.buttons>
          <Buttons.Minimize theme dispatch />
          <Buttons.MaximizeRestore theme windowDisplayMode dispatch />
          <Buttons.Close theme dispatch />
        </View>
      </View>;
  };

  let make =
      (
        ~dispatch,
        ~isFocused,
        ~windowDisplayMode,
        ~title,
        ~theme,
        ~font: UiFont.t,
        (),
      ) => {
    switch (Revery.Environment.os) {
    | Mac => <Mac isFocused windowDisplayMode font title theme dispatch />
    | Windows =>
      <Windows isFocused windowDisplayMode font title theme dispatch />
    | _ => React.empty
    };
  };
};

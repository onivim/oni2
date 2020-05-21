/*
 * Titlebar.re
 */

open Revery;
open Revery.UI;
open Revery.UI.Components;

open Oni_Core;
open Oni_Components;
module Model = Oni_Model;

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
    let text = (~isFocused, ~theme, ~font: UiFont.t) => [
      flexGrow(0),
      fontSize(12.),
      fontFamily(font.fontFileSemiBold),
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
      marginHorizontal(8),
    ];

    let title = (~isFocused, ~theme, ~font: UiFont.t) => [
      flexGrow(0),
      fontSize(12.),
      fontFamily(font.fontFile),
      marginLeft(4),
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

      let hoverClose = [
        height(30),
        width(46),
        backgroundColor(Color.rgba(0.91, 0.07, 0.14, 0.9)),
        justifyContent(`Center),
        alignItems(`Center),
      ];

      let hover = (~theme) => [
        height(30),
        width(46),
        backgroundColor(Colors.buttonActiveBackground.from(theme)),
        justifyContent(`Center),
        alignItems(`Center),
      ];
    };
  };
};

module Mac = {
  let make = (~isFocused, ~isFullscreen, ~title, ~theme, ~font: UiFont.t, ()) =>
    if (isFullscreen) {
      React.empty;
    } else {
      <Clickable
        onDoubleClick={_ =>
          GlobalContext.current().dispatch(Model.Actions.TitleDoubleClicked)
        }
        style={Styles.Mac.container(~isFocused, ~theme)}>
        <Text style={Styles.Mac.text(~isFocused, ~theme, ~font)} text=title />
      </Clickable>;
    };
};

module Windows = {
  module Buttons = {
    module Close = {
      let%component make = (~theme, ()) => {
        let%hook (isHovered, setHovered) = Hooks.state(false);

        <View
          style={
            isHovered
              ? Styles.Windows.Button.hoverClose
              : Styles.Windows.Button.container
          }
          onMouseEnter={_ => setHovered(_ => true)}
          onMouseLeave={_ => setHovered(_ => false)}>
          <FontIcon
            icon=Codicon.chromeClose
            fontFamily=Codicon.fontFamily
            color={Colors.activeForeground.from(theme)}
            fontSize=14.
          />
        </View>;
      };
    };

    module MaximizeRestore = {
      let%component make = (~theme, ~isMaximized, ()) => {
        let%hook (isHovered, setHovered) = Hooks.state(false);

        <View
          style={
            isHovered
              ? Styles.Windows.Button.hover(theme)
              : Styles.Windows.Button.container
          }
          onMouseEnter={_ => setHovered(_ => true)}
          onMouseLeave={_ => setHovered(_ => false)}>
          {isMaximized
             ? <FontIcon
                 icon=Codicon.chromeRestore
                 fontFamily=Codicon.fontFamily
                 color={Colors.activeForeground.from(theme)}
                 fontSize=14.
               />
             : <FontIcon
                 icon=Codicon.chromeMaximize
                 fontFamily=Codicon.fontFamily
                 color={Colors.activeForeground.from(theme)}
                 fontSize=14.
               />}
        </View>;
      };
    };

    module Minimize = {
      let%component make = (~theme, ()) => {
        let%hook (isHovered, setHovered) = Hooks.state(false);

        <View
          style={
            isHovered
              ? Styles.Windows.Button.hover(theme)
              : Styles.Windows.Button.container
          }
          onMouseEnter={_ => setHovered(_ => true)}
          onMouseLeave={_ => setHovered(_ => false)}>
          <FontIcon
            icon=Codicon.chromeMinimize
            fontFamily=Codicon.fontFamily
            color={Colors.activeForeground.from(theme)}
            fontSize=14.
          />
        </View>;
      };
    };
  };

  let make = (~isFocused, ~isMaximized, ~title, ~theme, ~font: UiFont.t, ()) =>
    <View
      mouseBehavior=Draggable
      style={Styles.Windows.container(~isFocused, ~theme)}>
      <View mouseBehavior=Draggable style=Styles.Windows.iconAndTitle>
        <Image src="./logo.png" width=18 height=18 />
        <Text
          style={Styles.Windows.title(~isFocused, ~theme, ~font)}
          text=title
        />
      </View>
      <View style=Styles.Windows.buttons>
        <Buttons.Minimize theme />
        <Buttons.MaximizeRestore theme isMaximized />
        <Buttons.Close theme />
      </View>
    </View>;
};

let make =
    (
      ~isFocused,
      ~isMaximized,
      ~isFullscreen,
      ~title,
      ~theme,
      ~font: UiFont.t,
      (),
    ) => {
  switch (Revery.Environment.os) {
  | Mac => <Mac isFocused isFullscreen font title theme />
  | Windows => <Windows isFocused isMaximized font title theme />
  | _ => React.empty
  };
};

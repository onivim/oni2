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
      marginHorizontal(16),
    ];

    let title = (~isFocused, ~theme, ~font: UiFont.t) => [
      flexGrow(0),
      fontSize(12.),
      fontFamily(font.fontFile),
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

module Mac = {
  let make =
      (~isFocused, ~windowDisplayMode, ~title, ~theme, ~font: UiFont.t, ()) =>
    if (windowDisplayMode == Model.State.Fullscreen) {
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

        let onMouseUp = _ =>
          GlobalContext.current().dispatch(Model.Actions.WindowCloseClicked);

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
      let%component make = (~theme, ~windowDisplayMode, ()) => {
        let%hook (isHovered, setHovered) = Hooks.state(false);

        let onMouseUp = _ =>
          switch (windowDisplayMode) {
          | Model.State.Maximized =>
            GlobalContext.current().dispatch(
              Model.Actions.WindowRestoreClicked,
            )
          | _ =>
            GlobalContext.current().dispatch(
              Model.Actions.WindowMaximizeClicked,
            )
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
          {windowDisplayMode == Model.State.Maximized
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
      let%component make = (~theme, ()) => {
        let%hook (isHovered, setHovered) = Hooks.state(false);

        let onMouseUp = _ =>
          GlobalContext.current().dispatch(
            Model.Actions.WindowMinimizeClicked,
          );

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
      (~isFocused, ~windowDisplayMode, ~title, ~theme, ~font: UiFont.t, ()) =>
    <View
      mouseBehavior=Draggable
      style={Styles.Windows.container(~isFocused, ~theme)}>
      <View mouseBehavior=Draggable style=Styles.Windows.iconAndTitle>
        <Image src="./logo-titlebar.png" width=18 height=18 />
        <Text
          style={Styles.Windows.title(~isFocused, ~theme, ~font)}
          text=title
        />
      </View>
      <View style=Styles.Windows.buttons>
        <Buttons.Minimize theme />
        <Buttons.MaximizeRestore theme windowDisplayMode />
        <Buttons.Close theme />
      </View>
    </View>;
};

let make =
    (~isFocused, ~windowDisplayMode, ~title, ~theme, ~font: UiFont.t, ()) => {
  switch (Revery.Environment.os) {
  | Mac => <Mac isFocused windowDisplayMode font title theme />
  | Windows => <Windows isFocused windowDisplayMode font title theme />
  | _ => React.empty
  };
};

open Oni_Core;
open Utility;

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

type model = {
  // We need to store this, as opposed to pulling it
  // from config, because we need to respect the value
  // used on initialization.
  useNativeTitleBar: bool,
};

let isNative = ({useNativeTitleBar, _}) => useNativeTitleBar;

let initial = (~useNativeTitleBar) => {useNativeTitleBar: useNativeTitleBar};

module Log = (val Log.withNamespace("Oni2.Feature.TitleBar"));

module Internal = {
  let withTag = (tag: string, value: option(string)) =>
    Option.map(v => (tag, v), value);
  let getTemplateVariables =
      (~activeBuffer, ~workspaceRoot, ~workspaceDirectory) => {
    let maybeBuffer = activeBuffer;
    let maybeFilePath = Option.bind(maybeBuffer, Buffer.getFilePath);

    let appName = Option.some("Onivim 2") |> withTag("appName");

    let dirty =
      Option.map(Buffer.isModified, maybeBuffer)
      |> (
        fun
        | Some(true) => Some("*")
        | _ => None
      )
      |> withTag("dirty");

    let activeEditorShort =
      Option.bind(maybeBuffer, Buffer.getShortFriendlyName)
      |> withTag("activeEditorShort");

    let activeEditorMedium =
      Option.bind(maybeBuffer, buf =>
        Buffer.getMediumFriendlyName(
          ~workingDirectory=workspaceDirectory,
          buf,
        )
      )
      |> withTag("activeEditorMedium");

    let activeEditorLong =
      Option.bind(maybeBuffer, Buffer.getLongFriendlyName)
      |> withTag("activeEditorLong");

    let activeFolderShort =
      Option.(
        maybeFilePath |> map(Filename.dirname) |> map(Filename.basename)
      )
      |> withTag("activeFolderShort");

    let activeFolderMedium =
      maybeFilePath
      |> Option.map(Filename.dirname)
      |> OptionEx.flatMap(fp =>
           Some(Path.toRelative(~base=workspaceDirectory, fp))
         )
      |> withTag("activeFolderMedium");

    let activeFolderLong =
      maybeFilePath
      |> Option.map(Filename.dirname)
      |> withTag("activeFolderLong");

    [
      appName,
      dirty,
      activeEditorShort,
      activeEditorMedium,
      activeEditorLong,
      activeFolderShort,
      activeFolderMedium,
      activeFolderLong,
      Some(("rootName", workspaceRoot)),
      Some(("rootPath", workspaceDirectory)),
    ]
    |> OptionEx.values
    |> List.to_seq
    |> StringMap.of_seq;
  };

  type titleClickBehavior =
    | Maximize
    | Minimize;

  let getTitleDoubleClickBehavior = () => {
    switch (Revery.Environment.os) {
    | Mac(_) =>
      try({
        let ic =
          Unix.open_process_in(
            "defaults read 'Apple Global Domain' AppleActionOnDoubleClick",
          );
        let operation = input_line(ic);
        switch (operation) {
        | "Maximize" => Maximize
        | "Minimize" => Minimize
        | _ => Maximize
        };
      }) {
      | _exn =>
        Log.warn(
          "
          Unable to read default behavior for AppleActionOnDoubleClick",
        );
        Maximize;
      }
    | _ => Maximize
    };
  };
};

// CONFIGURATION

module Configuration = {
  open Oni_Core;
  open Config.Schema;

  let windowTitle =
    setting(
      "window.title",
      string,
      ~default=
        "${dirty}${activeEditorShort}${separator}${rootName}${separator}${appName}",
    );
};

let title = (~activeBuffer, ~workspaceRoot, ~workspaceDirectory, ~config) => {
  let templateVariables =
    Internal.getTemplateVariables(
      ~activeBuffer,
      ~workspaceRoot,
      ~workspaceDirectory,
    );

  let titleTemplate = Configuration.windowTitle.get(config);
  let titleModel = titleTemplate |> Title.ofString;

  Title.toString(titleModel, templateVariables);
};

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update = (~maximize, ~minimize, ~restore, ~close, msg, model) => {
  let internalDoubleClickEffect =
    Isolinear.Effect.create(~name="window.doubleClick", () => {
      switch (Internal.getTitleDoubleClickBehavior()) {
      | Maximize => maximize()
      | Minimize => minimize()
      }
    });

  let internalWindowCloseEffect =
    Isolinear.Effect.create(~name="window.close", () => close());
  let internalWindowMaximizeEffect =
    Isolinear.Effect.create(~name="window.maximize", () => maximize());
  let internalWindowMinimizeEffect =
    Isolinear.Effect.create(~name="window.minimize", () => minimize());
  let internalWindowRestoreEffect =
    Isolinear.Effect.create(~name="window.restore", () => restore());

  switch (msg) {
  | TitleDoubleClicked => (model, Effect(internalDoubleClickEffect))
  | WindowCloseClicked => (model, Effect(internalWindowCloseEffect))
  | WindowMaximizeClicked => (model, Effect(internalWindowMaximizeEffect))
  | WindowRestoreClicked => (model, Effect(internalWindowRestoreEffect))
  | WindowMinimizeClicked => (model, Effect(internalWindowMinimizeEffect))
  };
};

// CONTRIBUTIONS

module Contributions = {
  let configuration = Configuration.[windowTitle.spec];
};

// VIEW
open Revery;
open Revery.UI;
open Revery.UI.Components;

module UiFont = Oni_Core.UiFont;
module Colors = Feature_Theme.Colors.TitleBar;

module Styles = {
  open Style;

  module Mac = {
    let container = (~isFocused, ~theme, ~height) => [
      flexGrow(0),
      Style.height(int_of_float(height)),
      backgroundColor(
        isFocused
          ? Colors.activeBackground.from(theme)
          : Colors.inactiveBackground.from(theme),
      ),
      flexDirection(`Row),
      justifyContent(`SpaceBetween),
      alignItems(`Center),
    ];

    let text = (~isFocused, ~theme, ~isRegistered) => [
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
      flexDirection(`Row),
      justifyContent(`Center),
      textWrap(TextWrapping.NoWrap),
      marginLeft(!isRegistered ? Feature_Registration.Constants.macWidth : 0),
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
      flexGrow(1),
      flexShrink(1),
      overflow(`Hidden),
    ];

    let icon = [pointerEvents(`Ignore)];

    let title = (~isFocused, ~theme) => [
      pointerEvents(`Ignore),
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
      textOverflow(`Ellipsis),
    ];

    let buttons = [
      flexDirection(`Row),
      flexGrow(0),
      flexShrink(0),
      alignItems(`Center),
    ];

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
    let title =
        (
          ~isFocused,
          ~isRegistered,
          ~theme,
          ~title,
          ~font: UiFont.t,
          ~majorVersion,
          (),
        ) =>
      /* The titlebar text style changed on Big Sur
         This will make sure that on all OS X versions (i.e. 10.x.y), the old style applies.
         On new versions we want the new style -- this can be easily augmented with other conditions,
         should the design change again.
         */
      if (majorVersion <= 10) {
        <Text
          style={Styles.Mac.text(~isFocused, ~theme, ~isRegistered)}
          fontFamily={font.family}
          fontWeight=Medium
          fontSize=12.
          text=title
        />;
      } else {
        <Text
          style={Styles.Mac.text(~isFocused, ~theme, ~isRegistered)}
          fontFamily={font.family}
          fontWeight=Bold
          fontSize=13.
          text=title
        />;
      };
    let make =
        (
          ~dispatch,
          ~registration,
          ~registrationDispatch,
          ~isFocused,
          ~windowDisplayMode,
          ~title as titleText,
          ~theme,
          ~font: UiFont.t,
          ~height,
          ~majorVersion,
          (),
        ) =>
      if (windowDisplayMode == Fullscreen) {
        React.empty;
      } else {
        let isRegistered = Feature_Registration.isRegistered(registration);

        <Clickable
          onDoubleClick={_ => dispatch(TitleDoubleClicked)}
          style={Styles.Mac.container(~isFocused, ~theme, ~height)}>
          <View
            style=Style.[
              flexDirection(`Row),
              justifyContent(`Center),
              flexGrow(1),
            ]>
            <title
              title=titleText
              isFocused
              isRegistered
              theme
              font
              majorVersion
            />
          </View>
          <Feature_Registration.View.TitleBar.Mac
            theme
            registration
            dispatch=registrationDispatch
            font
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
          ~menuBar,
          ~dispatch,
          ~registrationDispatch,
          ~registration,
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
          <Image
            style=Styles.Windows.icon
            src={`File("./logo-titlebar.png")}
            width=18
            height=18
          />
          <View style=Style.[paddingLeft(16)]> menuBar </View>
          <Text
            style={Styles.Windows.title(~isFocused, ~theme)}
            fontFamily={font.family}
            fontSize=12.
            text=title
          />
          <Feature_Registration.View.TitleBar.Windows
            theme
            registration
            dispatch=registrationDispatch
            font
            isFocused
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
        ~menuBar,
        ~activeBuffer,
        ~workspaceRoot,
        ~workspaceDirectory,
        ~registration,
        ~config,
        ~dispatch,
        ~registrationDispatch,
        ~isFocused,
        ~windowDisplayMode,
        ~theme,
        ~font: UiFont.t,
        ~height,
        ~model,
        (),
      ) => {
    let title =
      title(~activeBuffer, ~workspaceRoot, ~workspaceDirectory, ~config);
    switch (Revery.Environment.os) {
    | Mac(_) when isNative(model) => React.empty
    | Mac({major, _}) =>
      <Mac
        isFocused
        windowDisplayMode
        font
        title
        theme
        dispatch
        registration
        registrationDispatch
        height
        majorVersion=major
      />
    | Windows(_) when isNative(model) => menuBar
    | Windows(_) =>
      <Windows
        menuBar
        isFocused
        windowDisplayMode
        font
        title
        theme
        dispatch
        registrationDispatch
        registration
      />
    | Linux(_) => menuBar
    | _ => React.empty
    };
  };
};

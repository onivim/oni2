open Oni_Core;

module Constants = {
  let macWidth = 108;
};

type viewState =
  | Hidden
  | EnteringKey
  | Waiting
  | KeySuccess
  | KeyFailure
  | RequestFailure;

type model = {
  licenseKey: option(string),
  viewState,
  inputModel: Component_InputText.model,
};

let initialInputModel =
  Component_InputText.create(~placeholder="Enter your license key");

let initial = licenseKey => {
  licenseKey,
  viewState: Hidden,
  inputModel: initialInputModel,
};

let isActive = m => m.viewState != Hidden;
let isRegistered = m => !(m.licenseKey == None);
let getLicenseKey = m =>
  switch (m.licenseKey) {
  | Some(s) => s
  | None => ""
  };

[@deriving show]
type command =
  | EnterLicenseKey;

[@deriving show]
type msg =
  | Command(command)
  | Response(Service_Registration.response)
  | InputText(Component_InputText.msg)
  | KeyPressed(string)
  | Pasted(string);

type outmsg =
  | Nothing
  | LicenseKeyChanged(string)
  | Effect(Isolinear.Effect.t(msg));

module Commands = {
  open Feature_Commands.Schema;

  let enterLicenseKey =
    define(
      ~category="Oni2",
      ~title="Enter license key",
      "oni.app.enterLicenseKey",
      Command(EnterLicenseKey),
    );
};

module Contributions = {
  let contextKeys = (~isFocused, model) => {
    WhenExpr.ContextKeys.(
      isFocused
        ? Component_InputText.Contributions.contextKeys(model.inputModel)
        : empty
    );
  };
  let commands = Commands.[enterLicenseKey];
};

let update = (model, msg) =>
  switch (msg) {
  | Command(EnterLicenseKey) => (
      {...model, viewState: EnteringKey, inputModel: initialInputModel},
      Nothing,
    )
  | Response(LicenseValid(key, true)) => (
      {...model, licenseKey: Some(key), viewState: KeySuccess},
      LicenseKeyChanged(key),
    )
  | Response(LicenseValid(_, false)) => (
      {...model, viewState: KeyFailure},
      Nothing,
    )
  | Response(RequestFailed) => (
      {...model, viewState: RequestFailure},
      Nothing,
    )
  | InputText(msg) =>
    let (inputModel', _) = Component_InputText.update(msg, model.inputModel);
    ({...model, inputModel: inputModel'}, Nothing);
  | KeyPressed(key) =>
    let model' =
      switch (key) {
      | "<CR>" when model.viewState == EnteringKey => {
          ...model,
          viewState: Waiting,
        }
      | "<ESC>" => {...model, viewState: Hidden}
      | _ when model.viewState == EnteringKey =>
        let inputModel =
          Component_InputText.handleInput(~key, model.inputModel);
        {...model, inputModel};
      | _ => model
      };

    let outmsg =
      switch (key) {
      | "<CR>" when model.viewState == EnteringKey =>
        let licenseKey = Component_InputText.value(model.inputModel);
        let wrapEvent = evt => Response(evt);
        Effect(
          Service_Registration.Effect.checkLicenseKeyValidity(licenseKey)
          |> Isolinear.Effect.map(wrapEvent),
        );
      | _ => Nothing
      };

    (model', outmsg);
  | Pasted(text) =>
    let inputModel = Component_InputText.paste(~text, model.inputModel);
    ({...model, inputModel}, Nothing);
  };

module Styles = {
  open Revery;
  open Revery.UI;
  open Style;

  module Colors = Feature_Theme.Colors;

  let color = Color.rgba(0., 0., 0., 0.75);
  let boxShadow = [
    backgroundColor(color),
    boxShadow(
      ~xOffset=4.,
      ~yOffset=4.,
      ~blurRadius=12.,
      ~spreadRadius=0.,
      ~color,
    ),
  ];
  let container = (~theme) => [
    width(400),
    backgroundColor(Colors.Menu.background.from(theme)),
    Style.color(Colors.Menu.foreground.from(theme)),
  ];

  let message = [
    flexDirection(`Row),
    alignItems(`Center),
    justifyContent(`Center),
    marginVertical(8),
  ];

  let registrationText = (~theme, ~isFocused) => [
    Style.color(
      isFocused
        ? Colors.TitleBar.activeForeground.from(theme)
        : Colors.TitleBar.inactiveForeground.from(theme),
    ),
    marginTop(2),
    marginLeft(4),
  ];

  let messageIcon = [width(14), height(14), marginRight(8)];
  let messageText = [marginTop(2)];

  module Windows = {
    let container = (~theme, ~isHovered) => [
      flexDirection(`Row),
      height(30),
      justifyContent(`Center),
      alignItems(`Center),
      backgroundColor(
        isHovered
          ? Colors.TitleBar.hoverBackground.from(theme)
          : Revery.Colors.transparentWhite,
      ),
      marginLeft(8),
      paddingHorizontal(4),
    ];
  };

  module Mac = {
    let container = [
      flexDirection(`Row),
      height(25),
      width(Constants.macWidth),
      justifyContent(`Center),
      alignItems(`Center),
      flexGrow(0),
      paddingHorizontal(8),
    ];
  };
};

module View = {
  open Revery;
  open Revery.UI;

  module Colors = Feature_Theme.Colors;
  module FontIcon = Oni_Components.FontIcon;
  module FontAwesome = Oni_Components.FontAwesome;

  module Modal = {
    let modal = (~children, ~theme, ()) =>
      <View style=Styles.boxShadow>
        <View style={Styles.container(~theme)}>
          <View style=Style.[padding(5)]> children </View>
        </View>
      </View>;

    let loadingAnimation =
      Animation.(animate(Time.seconds(1)) |> tween(0., 6.28) |> repeat);

    let%component make =
                  (
                    ~theme: ColorTheme.Colors.t,
                    ~registration as model: model,
                    ~font: UiFont.t,
                    ~dispatch: msg => unit,
                    (),
                  ) => {
      let%hook (loadingRotation, _animationState, _resetRotation) =
        Hooks.animation(
          ~name="Loading Animation",
          loadingAnimation,
          ~active=model.viewState == Waiting,
        );

      switch (model.viewState) {
      | Hidden => React.empty
      | EnteringKey =>
        <modal theme>
          <Component_InputText.View
            model={model.inputModel}
            theme
            isFocused=true
            fontFamily={font.family}
            fontSize=14.
            dispatch={msg => dispatch(InputText(msg))}
          />
        </modal>
      | Waiting =>
        <modal theme>
          <View style=Styles.message>
            <View
              style=Style.[
                width(14),
                height(14),
                transform([
                  Transform.Rotate(Math.Angle.from_radians(loadingRotation)),
                ]),
                alignItems(`Center),
                justifyContent(`Center),
                marginRight(8),
              ]>
              <FontIcon
                icon=FontAwesome.spinner
                color={Colors.foreground.from(theme)}
                fontSize=14.
              />
            </View>
            <Text
              fontFamily={font.family}
              fontSize=14.
              style=Styles.messageText
              text="Validating license key..."
            />
          </View>
        </modal>
      | KeySuccess =>
        <modal theme>
          <View style=Styles.message>
            <View style=Styles.messageIcon>
              <FontIcon
                icon=FontAwesome.checkCircle
                fontSize=14.
                color={Colors.foreground.from(theme)}
              />
            </View>
            <Text
              fontFamily={font.family}
              fontSize=14.
              style=Styles.messageText
              text="Thank you for purchasing Onivim!"
            />
          </View>
        </modal>
      | KeyFailure =>
        <modal theme>
          <View style=Styles.message>
            <View style=Styles.messageIcon>
              <FontIcon
                icon=FontAwesome.timesCircle
                fontSize=14.
                color={Colors.foreground.from(theme)}
              />
            </View>
            <Text
              fontFamily={font.family}
              fontSize=14.
              style=Styles.messageText
              text="License key invalid."
            />
          </View>
        </modal>
      | RequestFailure =>
        <modal theme>
          <View style=Styles.message>
            <View style=Styles.messageIcon>
              <FontIcon
                icon=FontAwesome.timesCircle
                fontSize=14.
                color={Colors.foreground.from(theme)}
              />
            </View>
            <Text
              fontFamily={font.family}
              fontSize=14.
              style=Styles.messageText
              text="Validation failed. Please check your internet connection."
            />
          </View>
        </modal>
      };
    };
  };

  module TitleBar = {
    module Windows = {
      let%component make =
                    (
                      ~theme: ColorTheme.Colors.t,
                      ~registration as model: model,
                      ~font: UiFont.t,
                      ~dispatch: msg => unit,
                      ~isFocused: bool,
                      (),
                    ) => {
        let%hook (isHovered, setHovered) = Hooks.state(false);

        let onMouseUp = _ => dispatch(Command(EnterLicenseKey));

        switch (model.licenseKey) {
        | Some(_) => React.empty
        | None =>
          <View
            onMouseUp
            onMouseEnter={_ => setHovered(_ => true)}
            onMouseLeave={_ => setHovered(_ => false)}
            style={Styles.Windows.container(~isHovered, ~theme)}>
            <Codicon
              icon=Codicon.unlock
              color={
                isFocused
                  ? Colors.TitleBar.activeForeground.from(theme)
                  : Colors.TitleBar.inactiveForeground.from(theme)
              }
              fontSize=10.
            />
            <Text
              text="Unregistered"
              style={Styles.registrationText(~theme, ~isFocused)}
              fontSize=12.
              fontFamily={font.family}
            />
          </View>
        };
      };
    };

    module Mac = {
      let%component make =
                    (
                      ~theme: ColorTheme.Colors.t,
                      ~registration as model: model,
                      ~font: UiFont.t,
                      ~dispatch: msg => unit,
                      (),
                    ) => {
        let%hook () = Hooks.effect(OnMount, () => None);
        let onClick = _ => dispatch(Command(EnterLicenseKey));

        switch (model.licenseKey) {
        | Some(_) => React.empty
        | None =>
          <Components.Clickable onClick style=Styles.Mac.container>
            <FontIcon
              icon=FontAwesome.lockOpen
              color={Colors.TitleBar.inactiveForeground.from(theme)}
              fontSize=10.
            />
            <Text
              text="Unregistered"
              style={Styles.registrationText(~theme, ~isFocused=false)}
              fontSize=12.
              fontFamily={font.family}
            />
          </Components.Clickable>
        };
      };
    };
  };
};

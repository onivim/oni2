open Oni_Core;

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

let initial = {
  licenseKey: None,
  viewState: Hidden,
  inputModel: Component_InputText.empty,
};

let isActive = m => m.viewState != Hidden;

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
  let commands = Commands.[enterLicenseKey];
};

let update = (model, msg) =>
  switch (msg) {
  | Command(EnterLicenseKey) => (
      {
        ...model,
        viewState: EnteringKey,
        inputModel: Component_InputText.empty,
      },
      Nothing,
    )
  | Response(LicenseValid(key, true)) => (
      {...model, licenseKey: Some(key), viewState: KeySuccess},
      Nothing,
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
};

module View = {
  open Revery.UI;
  open Revery.UI.Components;

  module Colors = Feature_Theme.Colors;
  module FontIcon = Oni_Components.FontIcon;
  module FontAwesome = Oni_Components.FontAwesome;

  module Modal = {
    let%component make =
                  (
                    ~theme: ColorTheme.Colors.t,
                    ~registration as model: model,
                    ~font: UiFont.t,
                    ~dispatch: msg => unit,
                    (),
                  ) => {
      let%hook () = Hooks.effect(OnMount, () => None);

      switch (model.viewState) {
      | Hidden => React.empty
      | EnteringKey =>
        <View style=Styles.boxShadow>
          <View style={Styles.container(~theme)}>
            <View style=Style.[padding(5)]>
              <Component_InputText.View
                model={model.inputModel}
                theme
                isFocused=true
                fontFamily={font.family}
                fontSize=14.
                dispatch={msg => dispatch(InputText(msg))}
              />
            </View>
          </View>
        </View>
      | Waiting =>
        <View style=Styles.boxShadow>
          <View style={Styles.container(~theme)}>
            <View style=Style.[padding(5)]>
              <Text
                fontFamily={font.family}
                fontSize=14.
                text="Validating license key..."
              />
            </View>
          </View>
        </View>
      | KeySuccess =>
        <View style=Styles.boxShadow>
          <View style={Styles.container(~theme)}>
            <View style=Style.[padding(5)]>
              <Text
                fontFamily={font.family}
                fontSize=14.
                text="License key valid."
              />
            </View>
          </View>
        </View>
      | KeyFailure =>
        <View style=Styles.boxShadow>
          <View style={Styles.container(~theme)}>
            <View style=Style.[padding(5)]>
              <Text
                fontFamily={font.family}
                fontSize=14.
                text="License key invalid."
              />
            </View>
          </View>
        </View>
      | RequestFailure =>
        <View style=Styles.boxShadow>
          <View style={Styles.container(~theme)}>
            <View style=Style.[padding(5)]>
              <Text
                fontFamily={font.family}
                fontSize=14.
                text="Validation failed. Please check your internet connection."
              />
            </View>
          </View>
        </View>
      };
    };
  };
};

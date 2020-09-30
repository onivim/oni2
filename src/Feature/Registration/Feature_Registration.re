open Oni_Core;

type model = {
  licenseKey: option(string),
  shown: bool,
  inputModel: Component_InputText.model,
};

let initial = {
  licenseKey: None,
  shown: false,
  inputModel: Component_InputText.empty,
};

[@deriving show]
type command =
  | EnterLicenseKey;

[@deriving show]
type msg =
  | Command(command)
  | InputText(Component_InputText.msg);

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
  | Command(EnterLicenseKey) => ({...model, shown: true}, Nothing)
  | InputText(msg) =>
    let (inputModel', _) = Component_InputText.update(msg, model.inputModel);
    ({...model, inputModel: inputModel'}, Nothing);
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

  module Colors = Feature_Theme.Colors;

  let%component make =
                (
                  ~theme: ColorTheme.Colors.t,
                  ~registration as model: model,
                  ~font: UiFont.t,
                  ~dispatch: msg => unit,
                  (),
                ) => {
    let%hook () = Hooks.effect(OnMount, () => None);

    switch (model.shown) {
    | false => React.empty
    | true =>
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
    };
  };
};

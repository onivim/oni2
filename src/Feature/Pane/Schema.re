open Oni_Core;

type uniqueId = int;

let nextId = ref(0);

type t('model, 'msg) = {
  title: string,
  id: option(string),
  buttons:
    (
      ~font: UiFont.t,
      ~theme: ColorTheme.Colors.t,
      ~dispatch: 'msg => unit,
      ~model: 'model
    ) =>
    Revery.UI.element,
  contextKeys: (~isFocused: bool, 'model) => WhenExpr.ContextKeys.t,
  commands: 'model => list(Command.t('msg)),
  sub: (~isFocused: bool, 'model) => Isolinear.Sub.t('msg),
  view:
    (
      ~config: Config.resolver,
      ~editorFont: Service_Font.font,
      ~font: UiFont.t,
      ~isFocused: bool,
      ~iconTheme: IconTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~workingDirectory: string,
      ~theme: ColorTheme.Colors.t,
      ~dispatch: 'msg => unit,
      ~model: 'model
    ) =>
    Revery.UI.element,
  keyPressed: string => 'msg,
  uniqueId,
};

let panel =
    (~sub, ~title, ~id, ~buttons, ~contextKeys, ~commands, ~view, ~keyPressed) => {
  incr(nextId);
  {
    title,
    id,
    buttons,
    contextKeys,
    view,
    keyPressed,
    commands,
    uniqueId: nextId^,
    sub,
  };
};

let map = (~msg as mapMsg, ~model as mapModel, pane) => {
  let mapDispatch = (dispatch, msg) => {
    mapMsg(msg) |> dispatch;
  };

  let view' =
      (
        ~config,
        ~editorFont,
        ~font,
        ~isFocused,
        ~iconTheme,
        ~languageInfo,
        ~workingDirectory,
        ~theme,
        ~dispatch,
        ~model,
      ) => {
    let mappedModel = mapModel(model);
    let mappedDispatch = mapDispatch(dispatch);

    pane.view(
      ~config,
      ~editorFont,
      ~font,
      ~isFocused,
      ~iconTheme,
      ~languageInfo,
      ~workingDirectory,
      ~theme,
      ~dispatch=mappedDispatch,
      ~model=mappedModel,
    );
  };

  let buttons' = (~font, ~theme, ~dispatch, ~model) => {
    let mappedModel = mapModel(model);
    let mappedDispatch = mapDispatch(dispatch);
    pane.buttons(~font, ~theme, ~dispatch=mappedDispatch, ~model=mappedModel);
  };

  let contextKeys' = (~isFocused, model) => {
    let mappedModel = mapModel(model);
    pane.contextKeys(~isFocused, mappedModel);
  };

  let commands' = model => {
    let mappedModel = mapModel(model);
    let commands = pane.commands(mappedModel);
    commands |> List.map(Command.map(mapMsg));
  };

  let sub' = (~isFocused, model) => {
    let mappedModel = mapModel(model);

    let sub = pane.sub(~isFocused, mappedModel);

    sub |> Isolinear.Sub.map(mapMsg);
  };

  let keyPressed' = str => {
    pane.keyPressed(str) |> mapMsg;
  };

  {
    title: pane.title,
    id: pane.id,
    buttons: buttons',
    contextKeys: contextKeys',
    commands: commands',
    view: view',
    keyPressed: keyPressed',
    uniqueId: pane.uniqueId,
    sub: sub',
  };
};

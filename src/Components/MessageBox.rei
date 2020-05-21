// MODEL

type action('msg) = {
  label: string,
  msg: 'msg,
  shortcut,
}

and shortcut =
  | Key(string)
  | Sequence(string);

type model('msg) = {
  actions: list(action('msg)),
  input: string,
};

// UPDATE

type msg =
  | KeyPressed(string);

let update: (model('msg), msg) => (model('msg), Isolinear.Effect.t('msg));

// VIEW

open Oni_Core;
open Revery.UI;

let make:
  (
    ~children: React.element(React.node),
    ~theme: ColorTheme.Colors.t,
    ~font: UiFont.t,
    ~model: model('msg),
    ~onAction: 'msg => unit,
    unit
  ) =>
  React.element(React.node);

open Oni_Core;
module Options = {
  [@deriving show]
  type t = {
    placeholder: option(string),
    matchOnDescription: bool,
    matchOnDetail: bool,
    matchOnLabel: bool, // default true
    autoFocusOnList: bool, // default true
    ignoreFocusLost: bool,
    canPickMany: bool,
    // TODO:
    // https://github.com/onivim/vscode-exthost/blob/a25f426a04fe427beab7465be660f89a794605b5/src/vs/platform/quickinput/common/quickInput.ts#L78
    //quickNavigate
    contextKey: option(string),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          placeholder: field.optional("placeholder", string),
          matchOnDescription:
            field.withDefault("matchOnDescription", false, bool),
          matchOnDetail: field.withDefault("matchOnDetail", false, bool),
          matchOnLabel: field.withDefault("matchOnLabel", true, bool),
          autoFocusOnList: field.withDefault("autoFocusOnList", true, bool),
          ignoreFocusLost: field.withDefault("ignoreFocusLost", false, bool),
          canPickMany: field.withDefault("canPickMany", false, bool),
          contextKey: field.optional("contextKey", string),
        }
      )
    );
};

module Button = {
  [@deriving show]
  type icon = {
    dark: Oni_Core.Uri.t,
    light: option(Oni_Core.Uri.t),
  };

  [@deriving show]
  type t = {
    iconPath: option(icon),
    iconClass: option(string),
    tooltip: option(string),
  };

  module Decode = {
    open Json.Decode;

    let icon =
      obj(({field, _}) =>
        {
          dark: field.required("dark", Oni_Core.Uri.decode),
          light: field.optional("light", Oni_Core.Uri.decode),
        }
      );
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          iconPath: field.optional("iconPath", Decode.icon),
          iconClass: field.optional("iconClass", string),
          tooltip: field.optional("tooltip", string),
        }
      )
    );
};

module Item = {
  [@deriving show]
  type t = {
    // TransferQuickPickItems
    handle: int,
    // IQuickPickItem
    id: option(string),
    label: string,
    description: option(string),
    detail: option(string),
    iconClasses: list(string),
    buttons: list(Button.t),
    picked: bool,
    alwaysShow: bool,
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          handle: field.required("handle", int),
          id: field.optional("id", string),
          label: field.required("label", string),
          description: field.optional("description", string),
          detail: field.optional("detail", string),
          iconClasses: field.withDefault("iconClasses", [], list(string)),
          buttons: field.withDefault("buttons", [], list(Button.decode)),
          picked: field.withDefault("picked", false, bool),
          alwaysShow: field.withDefault("alwaysShow", false, bool),
        }
      )
    );
};

module QuickPick = {
  [@deriving show]
  type t = {
    // BaseTransferQuickInput
    id: int,
    enabled: bool,
    busy: bool,
    visible: bool,
    // TransferQuickPick
    value: option(string),
    placeholder: option(string),
    buttons: list(Button.t),
    items: list(Item.t),
    // TODO:
    // activeItems
    // selectedItems
    // canSelectMany
    // ignoreFocusOut
    // matchOnDescription
    // Match on Detail
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", int),
          enabled: field.withDefault("enabled", true, bool),
          busy: field.withDefault("busy", false, bool),
          visible: field.withDefault("visible", true, bool),
          value: field.optional("value", string),
          placeholder: field.optional("placeholder", string),
          buttons: field.withDefault("buttons", [], list(Button.decode)),
          items: field.withDefault("items", [], list(Item.decode)),
        }
      )
    );
};

module QuickInput = {
  [@deriving show]
  type t = {
    // BaseTransferQuickInput
    id: int,
    enabled: bool,
    busy: bool,
    visible: bool,
    // TransferInputBox
    value: option(string),
    placeholder: option(string),
    buttons: list(Button.t),
    prompt: option(string),
    validationMessage: option(string),
  };

  let decode =
    Json.Decode.(
      obj(({field, _}) =>
        {
          id: field.required("id", int),
          enabled: field.withDefault("enabled", true, bool),
          busy: field.withDefault("busy", false, bool),
          visible: field.withDefault("visible", true, bool),
          value: field.optional("value", string),
          placeholder: field.optional("placeholder", string),
          prompt: field.optional("prompt", string),
          buttons: field.withDefault("buttons", [], list(Button.decode)),
          validationMessage: field.optional("validationMessage", string),
        }
      )
    );
};

[@deriving show]
type t =
  | QuickPick(QuickPick.t)
  | QuickInput(QuickInput.t);

let decode = {
  Json.Decode.(
    obj(({field, whatever, _}) => {
      let pickType = field.required("type", string);

      if (pickType == "quickPick") {
        QuickPick(whatever(QuickPick.decode));
      } else {
        QuickInput(whatever(QuickInput.decode));
      };
    })
  );
};

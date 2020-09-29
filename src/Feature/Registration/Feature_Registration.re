open Oni_Core;

type model = {
  licenseKey: option(string),
  shown: bool,
};

let initial = {licenseKey: None, shown: false};

[@deriving show]
type command =
  | EnterLicenseKey;

[@deriving show]
type msg =
  | Command(command);

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
  };

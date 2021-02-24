open Oni_Core;
open Utility;

let defaultUpdateChannel =
  if (Oni_Core.BuildInfo.version |> StringEx.endsWith(~postfix="-staging")) {
    "staging";
  } else if (Oni_Core.BuildInfo.version
             |> StringEx.endsWith(~postfix="-nightly")) {
    "master";
  } else {
    "stable";
  };

type model = {
  automaticallyChecksForUpdates: bool,
  releaseChannel: string,
};

let initial = {
  automaticallyChecksForUpdates: true,
  releaseChannel: defaultUpdateChannel,
};

[@deriving show({with_path: false})]
type command =
  | CheckForUpdates;

[@deriving show({with_path: false})]
type msg =
  | Service(Service_AutoUpdate.msg)
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | ErrorMessage(string);

let platformStr =
  switch (Revery.Environment.os) {
  | Mac(_) => "macos"
  | Linux(_) => "linux"
  | Windows(_) => "windows"
  | _ => ""
  };

module Configuration = {
  open Config.Schema;

  let automaticallyChecksForUpdates =
    setting("oni.app.automaticallyChecksForUpdates", bool, ~default=true);

  let releaseChannelCodec =
    custom(~decode=Json.Decode.(string), ~encode=Json.Encode.(string));

  let releaseChannel =
    setting(
      "oni.app.updateReleaseChannel",
      releaseChannelCodec,
      ~default=defaultUpdateChannel,
    );
};

module Commands = {
  open Feature_Commands.Schema;

  let checkForUpdates =
    define(
      ~category="Auto-Update",
      ~title="Check for updates",
      "oni.app.checkForUpdates",
      Command(CheckForUpdates),
    );
};

module Contributions = {
  let configuration = [
    Configuration.automaticallyChecksForUpdates.spec,
    Configuration.releaseChannel.spec,
  ];

  let commands = Commands.[checkForUpdates];
};

let sub = (~config) => {
  let automaticallyChecksForUpdates =
    Configuration.automaticallyChecksForUpdates.get(config);
  let releaseChannel = Configuration.releaseChannel.get(config);

  Service_AutoUpdate.Sub.autoUpdate(
    ~uniqueId="autoUpdate",
    ~releaseChannel,
    ~automaticallyChecksForUpdates,
  )
  |> Isolinear.Sub.map(msg => Service(msg));
};

let update = (~getLicenseKey, model, msg) =>
  switch (msg) {
  | Service(AutoCheckChanged(automaticallyChecksForUpdates)) => (
      {...model, automaticallyChecksForUpdates},
      Effect(
        Service_AutoUpdate.Effect.setAutomaticallyChecksForUpdates(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~automaticallyChecksForUpdates,
        ),
      ),
    )
  | Service(ReleaseChannelChanged(releaseChannel)) =>
    let newModel = {...model, releaseChannel};
    (
      newModel,
      Effect(
        Service_AutoUpdate.Effect.setFeed(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~licenseKey=getLicenseKey(),
          ~releaseChannel=newModel.releaseChannel,
          ~platform=platformStr,
        ),
      ),
    );
  | Service(LicenseKeyChanged(licenseKey)) => (
      model,
      Effect(
        Service_AutoUpdate.Effect.setFeed(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
          ~licenseKey,
          ~releaseChannel=model.releaseChannel,
          ~platform=platformStr,
        ),
      ),
    )
  | Command(CheckForUpdates) =>
    if (StringEx.isEmpty(getLicenseKey())) {
      (
        model,
        ErrorMessage(
          "A valid license key is needed to check for updates. Please register and try again.",
        ),
      );
    } else {
      (
        model,
        Effect(
          Service_AutoUpdate.Effect.checkForUpdates(
            ~updater=Oni2_Sparkle.Updater.getInstance(),
          ),
        ),
      );
    }
  };

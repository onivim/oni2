open Oni_Core;

type model = {
  automaticallyChecksForUpdates: bool,
  releaseChannel: [ | `Nightly | `Master | `Test],
};

let initial = {automaticallyChecksForUpdates: true, releaseChannel: `Master};

[@deriving show({with_path: false})]
type command =
  | CheckForUpdates;

[@deriving show({with_path: false})]
type msg =
  | Service(Service_AutoUpdate.msg)
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let releaseChannelToString =
  fun
  | `Nightly => "nightly"
  | `Master => "master"
  | `Test => "test-update";

let platformStr =
  switch (Revery.Environment.os) {
  | Mac => "macos"
  | Linux => "linux"
  | Windows => "windows"
  | _ => ""
  };

module Configuration = {
  open Config.Schema;

  let automaticallyChecksForUpdates =
    setting("oni.app.automaticallyChecksForUpdates", bool, ~default=true);

  let releaseChannelCodec =
    custom(
      ~decode=
        Json.Decode.(
          string
          |> map(
               fun
               | "nightly" => `Nightly
               | "test-update"
               | "test" => `Test
               | "master"
               | _ => `Master,
             )
        ),
      ~encode=
        Json.Encode.(
          fun
          | `Nightly => string("nightly")
          | `Master => string("master")
          | `Test => string("test-update")
        ),
    );

  let releaseChannel =
    setting(
      "oni.app.updateReleaseChannel",
      releaseChannelCodec,
      ~default=`Master,
    );
};

module Commands = {
  open Feature_Commands.Schema;

  let checkForUpdates =
    define(
      ~category="Oni2",
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
          ~releaseChannel=releaseChannelToString(newModel.releaseChannel),
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
          ~releaseChannel=releaseChannelToString(model.releaseChannel),
          ~platform=platformStr,
        ),
      ),
    )
  | Command(CheckForUpdates) => (
      model,
      Effect(
        Service_AutoUpdate.Effect.checkForUpdates(
          ~updater=Oni2_Sparkle.Updater.getInstance(),
        ),
      ),
    )
  };

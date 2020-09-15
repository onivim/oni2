[@deriving show({with_path: false})]
type t = {
  automaticallyChecksForUpdates: bool,
  licenseKey: string,
};

[@deriving show({with_path: false})]
type msg =
  | AutoCheckChanged(bool)
  | ReleaseChannelChanged([ | `Nightly | `Master | `Test])
  | LicenseKeyChanged(string);

module Sub = {
  type params = {
    automaticallyChecksForUpdates: bool,
    licenseKey: string,
    releaseChannel: [ | `Nightly | `Master | `Test],
    uniqueId: string,
  };

  module AutoUpdateSubscription =
    Isolinear.Sub.Make({
      type state = {
        automaticallyChecksForUpdates: bool,
        licenseKey: string,
        releaseChannel: [ | `Nightly | `Master | `Test],
      };
      type nonrec msg = msg;
      type nonrec params = params;

      let name = "AutoUpdate";

      let id = ({uniqueId, _}) => uniqueId;

      let init = (~params: params, ~dispatch: msg => unit) => {
        dispatch(AutoCheckChanged(params.automaticallyChecksForUpdates));
        dispatch(LicenseKeyChanged(params.licenseKey));
        dispatch(ReleaseChannelChanged(params.releaseChannel));

        {
          automaticallyChecksForUpdates: params.automaticallyChecksForUpdates,
          licenseKey: params.licenseKey,
          releaseChannel: params.releaseChannel,
        };
      };

      let update = (~params: params, ~state: state, ~dispatch: msg => unit) => {
        if (params.automaticallyChecksForUpdates
            != state.automaticallyChecksForUpdates) {
          dispatch(AutoCheckChanged(params.automaticallyChecksForUpdates));
        };

        if (params.licenseKey != state.licenseKey) {
          dispatch(LicenseKeyChanged(params.licenseKey));
        };

        if (params.releaseChannel != state.releaseChannel) {
          dispatch(ReleaseChannelChanged(params.releaseChannel));
        };

        {
          licenseKey: params.licenseKey,
          automaticallyChecksForUpdates: params.automaticallyChecksForUpdates,
          releaseChannel: params.releaseChannel,
        };
      };

      let dispose = (~params as _, ~state as _) => ();
    });

  let autoUpdate =
      (
        ~uniqueId,
        ~automaticallyChecksForUpdates,
        ~licenseKey,
        ~releaseChannel,
      ) =>
    AutoUpdateSubscription.create({
      uniqueId,
      automaticallyChecksForUpdates,
      licenseKey,
      releaseChannel,
    });
};

module Effect = {
  let setAutomaticallyChecksForUpdates =
      (~updater, ~automaticallyChecksForUpdates) =>
    Isolinear.Effect.create(
      ~name="autoupdate.setAutomaticallyChecksForUpdates", () => {
      Oni2_Sparkle.Updater.setAutomaticallyChecksForUpdates(
        updater,
        automaticallyChecksForUpdates,
      )
    });

  let setFeedUrl = (~updater, ~url) =>
    Isolinear.Effect.create(~name="autoupdate.setFeedUrl", () => {
      Oni2_Sparkle.Updater.setFeedURL(updater, url)
    });

  let checkForUpdates = (~updater) =>
    Isolinear.Effect.create(~name="autoupdate.checkForUpdates", () => {
      Oni2_Sparkle.Updater.checkForUpdates(updater)
    });
};

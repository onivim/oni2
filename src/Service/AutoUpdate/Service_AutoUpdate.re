[@deriving show({with_path: false})]
type t = {
  automaticallyChecksForUpdates: bool,
  licenseKey: string,
};

[@deriving show({with_path: false})]
type msg =
  | AutoCheckChanged(bool)
  | LicenseKeyChanged(string);

module Sub = {
  type params = {
    automaticallyChecksForUpdates: bool,
    licenseKey: string,
    uniqueId: string,
  };

  module AutoUpdateSubscription =
    Isolinear.Sub.Make({
      type state = {
        automaticallyChecksForUpdates: bool,
        licenseKey: string,
      };
      type nonrec msg = msg;
      type nonrec params = params;

      let name = "AutoUpdate";

      let id = ({uniqueId, _}) => uniqueId;

      let init = (~params: params, ~dispatch: msg => unit) => {
        dispatch(AutoCheckChanged(params.automaticallyChecksForUpdates));
        dispatch(LicenseKeyChanged(params.licenseKey));

        {
          automaticallyChecksForUpdates: params.automaticallyChecksForUpdates,
          licenseKey: params.licenseKey,
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

        {
          licenseKey: params.licenseKey,
          automaticallyChecksForUpdates: params.automaticallyChecksForUpdates,
        };
      };

      let dispose = (~params as _, ~state as _) => ();
    });

  let autoUpdate = (~uniqueId, ~automaticallyChecksForUpdates, ~licenseKey) =>
    AutoUpdateSubscription.create({
      uniqueId,
      automaticallyChecksForUpdates,
      licenseKey,
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
};

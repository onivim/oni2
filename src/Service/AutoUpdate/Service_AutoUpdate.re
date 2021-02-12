[@deriving show({with_path: false})]
type msg =
  | AutoCheckChanged(bool)
  | LicenseKeyChanged([@opaque] string)
  | ReleaseChannelChanged(string);

module Sub = {
  type params = {
    automaticallyChecksForUpdates: bool,
    releaseChannel: string,
    uniqueId: string,
  };

  module AutoUpdateSubscription =
    Isolinear.Sub.Make({
      type state = {
        automaticallyChecksForUpdates: bool,
        releaseChannel: string,
      };
      type nonrec msg = msg;
      type nonrec params = params;

      let name = "AutoUpdate";

      let id = ({uniqueId, _}) => uniqueId;

      let init = (~params: params, ~dispatch: msg => unit) => {
        dispatch(AutoCheckChanged(params.automaticallyChecksForUpdates));
        dispatch(ReleaseChannelChanged(params.releaseChannel));

        {
          automaticallyChecksForUpdates: params.automaticallyChecksForUpdates,
          releaseChannel: params.releaseChannel,
        };
      };

      let update = (~params: params, ~state: state, ~dispatch: msg => unit) => {
        if (params.automaticallyChecksForUpdates
            != state.automaticallyChecksForUpdates) {
          dispatch(AutoCheckChanged(params.automaticallyChecksForUpdates));
        };

        if (params.releaseChannel != state.releaseChannel) {
          dispatch(ReleaseChannelChanged(params.releaseChannel));
        };

        {
          automaticallyChecksForUpdates: params.automaticallyChecksForUpdates,
          releaseChannel: params.releaseChannel,
        };
      };

      let dispose = (~params as _, ~state as _) => ();
    });

  let autoUpdate =
      (~uniqueId, ~automaticallyChecksForUpdates, ~releaseChannel) =>
    AutoUpdateSubscription.create({
      uniqueId,
      automaticallyChecksForUpdates,
      releaseChannel,
    });
};

module Effect = {
  open {
         module Constants = {
           let baseUrl = "https://v2.onivim.io/appcast";
         };

         let urlOfAttrs = (~licenseKey, ~releaseChannel, ~platform) =>
           Constants.baseUrl
           ++ "?channel="
           ++ releaseChannel
           ++ "&licenseKey="
           ++ licenseKey
           ++ "&platform="
           ++ platform;
       };
  let setAutomaticallyChecksForUpdates =
      (~updater, ~automaticallyChecksForUpdates) =>
    Isolinear.Effect.create(
      ~name="autoupdate.setAutomaticallyChecksForUpdates", () => {
      Oni2_Sparkle.Updater.setAutomaticallyChecksForUpdates(
        updater,
        automaticallyChecksForUpdates,
      )
    });

  let setFeed = (~updater, ~licenseKey, ~releaseChannel, ~platform) =>
    Isolinear.Effect.create(~name="autoupdate.setFeedUrl", () => {
      let url = urlOfAttrs(~licenseKey, ~releaseChannel, ~platform);
      Oni2_Sparkle.Updater.setFeedURL(updater, url);
    });

  let checkForUpdates = (~updater) =>
    Isolinear.Effect.create(~name="autoupdate.checkForUpdates", () => {
      Oni2_Sparkle.Updater.checkForUpdates(updater)
    });

  let updateLicenseKey = (~licenseKey) =>
    Isolinear.Effect.createWithDispatch(
      ~name="autoupdate.updateLicenseKey", dispatch => {
      dispatch(LicenseKeyChanged(licenseKey))
    });
};

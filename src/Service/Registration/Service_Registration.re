open Oni_Core;

[@deriving show({with_path: false})]
type response =
  | LicenseValid(string, bool)
  | RequestFailed;

module Effect = {
  let checkLicenseKeyValidity = (~proxy, licenseKey) =>
    Isolinear.Effect.createWithDispatch(
      ~name="registration.checkLicenseKeyValidity", dispatch => {
      let licenseKey = licenseKey |> String.trim;
      let url =
        "https://v2.onivim.io/api/isLicenseKeyValid?licenseKey=" ++ licenseKey;
      let setup = Setup.init();
      let response =
        Service_Net.Request.json(
          ~proxy,
          ~setup,
          ~decoder=Json.Decode.value,
          url,
        );

      Lwt.on_success(response, json =>
        switch (json) {
        | `Bool(b) => dispatch(LicenseValid(licenseKey, b))
        | _ => dispatch(RequestFailed)
        }
      );

      Lwt.on_failure(response, _ => dispatch(RequestFailed));
    });
};

open Oni_Core;

module Log = (val Log.withNamespace("Service_Exthost"));

module Effects = {
  module SCM = {
    let provideOriginalResource = (~handles, extHostClient, path, toMsg) =>
      Isolinear.Effect.createWithDispatch(~name="scm.getOriginalUri", dispatch => {
        // Try our luck with every provider. If several returns Last-Writer-Wins
        // TODO: Is there a better heuristic? Perhaps use rootUri to choose the "nearest" provider?
        handles
        |> List.iter(handle => {
             let promise =
               Exthost.Request.SCM.provideOriginalResource(
                 ~handle,
                 ~uri=Uri.fromPath(path),
                 extHostClient,
               );

             Lwt.on_success(
               promise,
               Option.iter(uri => dispatch(toMsg(uri))),
             );
           })
      });

    let onInputBoxValueChange = (~handle, ~value, extHostClient) =>
      Isolinear.Effect.create(~name="scm.onInputBoxValueChange", () =>
        Exthost.Request.SCM.onInputBoxValueChange(
          ~handle,
          ~value,
          extHostClient,
        )
      );
  };
};

module MutableState = {
  let activatedFileTypes: Hashtbl.t(string, bool) = Hashtbl.create(16);

};

module Internal = {
  let bufferMetadataToModelAddedDelta =
      (~version, ~filePath, ~fileType: option(string)) =>
    switch (filePath, fileType) {
    | (Some(fp), Some(ft)) =>
      Log.trace("Creating model for filetype: " ++ ft);

      Some(
        Exthost.ModelAddedDelta.create(
          ~versionId=version,
          ~lines=[""],
          ~modeId=ft,
          ~isDirty=true,
          Uri.fromPath(fp),
        ),
      );
    | _ => None
    };

  let activateFileType = (~client,fileType: option(string)) =>
    fileType
    |> Option.iter(ft =>
         if (!Hashtbl.mem(MutableState.activatedFileTypes, ft)) {
           // If no entry, we haven't activated yet
           Exthost.Request.ExtensionService.activateByEvent(
             ~event="onLanguage:" ++ ft,
             client,
           );
           Hashtbl.add(MutableState.activatedFileTypes, ft, true);
         }
       );
};

module Sub = {
  let buffer = (
    ~buffer,
    ~client,
  ) => Isolinear.Sub.none;
}

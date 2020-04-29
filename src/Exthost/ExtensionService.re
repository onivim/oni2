[@deriving show]
type msg =
  | ActivateExtension({
      extensionId: string,
      activationEvent: option(string),
    })
  | WillActivateExtension({extensionId: string})
  | DidActivateExtension({
      extensionId: string,
      //startup: bool,
      codeLoadingTime: int,
      activateCallTime: int,
      activateResolvedTime: int,
    })
  //activationEvent: option(string),
  | ExtensionActivationError({
      extensionId: string,
      errorMessage: string,
    })
  | ExtensionRuntimeError({extensionId: string});
// TODO: Error?

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | ("$activateExtension", `List([`String(extensionId)])) =>
    Ok(ActivateExtension({extensionId, activationEvent: None}))
  | (
      "$activateExtension",
      `List([`String(extensionId), activationEventJson]),
    ) =>
    let activationEvent =
      switch (activationEventJson) {
      | `String(v) => Some(v)
      | _ => None
      };

    Ok(ActivateExtension({extensionId, activationEvent}));
  | (
      "$onExtensionActivationError",
      `List([`String(extensionId), `String(errorMessage)]),
    ) =>
    Ok(ExtensionActivationError({extensionId, errorMessage}))
  | ("$onWillActivateExtension", `List([`String(extensionId)])) =>
    Ok(WillActivateExtension({extensionId: extensionId}))
  | (
      "$onDidActivateExtension",
      `List([
        `String(extensionId),
        `Int(codeLoadingTime),
        `Int(activateCallTime),
        `Int(activateResolvedTime),
        ..._args,
      ]),
    ) =>
    Ok(
      DidActivateExtension({
        extensionId,
        codeLoadingTime,
        activateCallTime,
        activateResolvedTime,
      }),
    )
  | ("$onExtensionRuntimeError", `List([`String(extensionId), ..._args])) =>
    Ok(ExtensionRuntimeError({extensionId: extensionId}))
  | _ => Error("Unhandled method: " ++ method)
  };
};

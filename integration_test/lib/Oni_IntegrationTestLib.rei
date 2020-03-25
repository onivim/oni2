module Core = Oni_Core;
module Model = Oni_Model;
module TextSynchronization = TextSynchronization;
module ExtensionHelpers = ExtensionHelpers;
open Types;

let runTest:
  (
    ~configuration: option(string)=?,
    ~keybindings: option(string)=?,
    ~cliOptions: option(Core.Cli.t)=?,
    ~name: string=?,
    ~onAfterDispatch: Model.Actions.t => unit=?,
    testCallback
  ) =>
  unit;

let runTestWithInput:
  (
    ~configuration: option(string)=?,
    ~keybindings: option(string)=?,
    ~name: string,
    ~onAfterDispatch: Model.Actions.t => unit=?,
    testCallbackWithInput
  ) =>
  unit;

let setUserSettings: Core.Config.Settings.t => unit;
let setClipboard: option(string) => unit;
let getClipboard: unit => option(string);
let setTime: float => unit;
let getTitle: unit => string;

/* [getAssetPath(path)] returns the path for a bundled asset for testing.
      This path is different on developer machines vs CI environments
   */
let getAssetPath: string => string;

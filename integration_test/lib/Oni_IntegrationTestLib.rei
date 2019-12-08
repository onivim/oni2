module Core = Oni_Core;
module Log = Core.Log;
module Model = Oni_Model;
module TextSynchronization = TextSynchronization;
open Types;

let runTest:
  (
    ~configuration: option(string)=?,
    ~cliOptions: option(Core.Cli.t)=?,
    ~name: string=?,
    testCallback
  ) =>
  unit;

let runTestWithInput: (~name: string, testCallbackWithInput) => unit;

let setClipboard: option(string) => unit;
let getClipboard: unit => option(string);
let setTime: float => unit;
let getTitle: unit => string;

/* [getAssetPath(path)] returns the path for a bundled asset for testing.
      This path is different on developer machines vs CI environments
   */
let getAssetPath: string => string;

/*
 * BufferPath.re
 */

open Oniguruma;

module OptionEx = Utility.OptionEx;

type t =
  | FilePath(string)
  | Terminal({
      bufferId: int,
      cmd: string,
    })
  | Welcome;

let welcome = "oni://Welcome";
let terminalRegex = OnigRegExp.create("oni://terminal/([0-9]*)/(.*)");

let parse = bufferPath =>
  if (String.equal(bufferPath, welcome)) {
    Welcome;
  } else {
    terminalRegex
    |> Result.to_option
    |> OptionEx.flatMap(regex => {
         let matches = regex |> OnigRegExp.search(bufferPath, 0);

         if (Array.length(matches) < 3) {
           None;
         } else {
           let bufferId =
             OnigRegExp.Match.getText(matches[1]) |> int_of_string;
           let cmd = OnigRegExp.Match.getText(matches[2]);
           Some(Terminal({bufferId, cmd}));
         };
       })
    |> Option.value(~default=FilePath(bufferPath));
  };

open Oni_Core;

type t = {onData: string => unit};

let start = (~env, ~cwd, ~rows, ~cols, ~cmd, onData) => {
  let _: unit => unit =
    Revery.Tick.interval(
      ~name="test-pty",
      _current => {
        prerr_endline("Dropping some data...");
        onData("Hello, world");
      },
      Revery.Time.seconds(2),
    );
  Ok({onData: onData});
};

let write = ({onData, _}, data) => {
  onData(data);
};

let resize = (~rows, ~cols, _pty) => ();

let close = pty => ();

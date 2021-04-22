open Oni_Core;
module NamedPipe = Exthost.NamedPipe;

type model = {namedPipe: NamedPipe.t};

let create = () => {
  namedPipe:
    NamedPipe.create("clientserver:" ++ string_of_int(Luv.Pid.getpid())),
};

[@deriving show]
type msg =
  | Server(Exthost.Transport.msg);

type outmsg =
  | Nothing
  | OpenFile({filePath: FpExp.t(FpExp.absolute)})
  | OpenFolder({folderPath: FpExp.t(FpExp.absolute)});

let update = (msg, model) => {
  prerr_endline("MSG: " ++ show_msg(msg));
  (model, Nothing);
};

let pipe = ({namedPipe, _}) => namedPipe;

type params = NamedPipe.t;
module Sub =
  Isolinear.Sub.Make({
    type nonrec msg = Exthost.Transport.msg;
    type nonrec params = params;
    type state = unit;

    let name = "Feature_ClientServer.sub";

    let id = namedPipe => NamedPipe.toString(namedPipe);

    let init = (~params as namedPipe, ~dispatch) => {
      prerr_endline("Creating server: " ++ NamedPipe.toString(namedPipe));
      let res =
        Exthost.Transport.start(
          ~namedPipe=NamedPipe.toString(namedPipe),
          ~dispatch,
        );

      switch (res) {
      | Ok(_transport) => prerr_endline("Started!")
      | Error(msg) => prerr_endline("Failed to start: " ++ msg)
      };
      ();
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state as _) => ();
  });

let sub = ({namedPipe, _}) =>
  Sub.create(namedPipe) |> Isolinear.Sub.map(msg => Server(msg));

module RemoteCommand = {
  type t = unit;

  let cli = _cli => ();
};

module Client = {
  let send = (~server: string, cmd) => {
    Lwt.return();
  };
};

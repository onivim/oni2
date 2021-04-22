open Oni_Core;
module NamedPipe = Exthost.NamedPipe;

type model = {namedPipe: NamedPipe.t};

let create = () => {
  namedPipe:
    NamedPipe.create("clientserver-" ++ string_of_int(Luv.Pid.getpid())),
};

[@deriving show]
type msg = unit;

type outmsg =
  | Nothing
  | OpenFile({filePath: FpExp.t(FpExp.absolute)})
  | OpenFolder({folderPath: FpExp.t(FpExp.absolute)});

let update = (_msg, model) => (model, Nothing);

let pipe = ({namedPipe, _}) => namedPipe;

let sub = _model => Isolinear.Sub.none;

module RemoteCommand = {
  type t = unit;

  let cli = _cli => ();
};

module Client = {
  let send = (~server: string, cmd) => {
    Lwt.return();
  };
};

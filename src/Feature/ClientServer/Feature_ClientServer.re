open Oni_Core;
module NamedPipe = Exthost.NamedPipe;

type model = {namedPipe: NamedPipe.t};

let create = () => {
  namedPipe:
    NamedPipe.create("clientserver:" ++ string_of_int(Luv.Pid.getpid())),
};

module Protocol = {
  [@deriving show]
  type t =
    | OpenFilesAndFolder({
        files: list(string),
        folderToOpen: option(string),
      });

  let fromBytes = (bytes): t => Marshal.from_bytes(bytes, 0);

  let toBytes = protocol => Marshal.to_bytes(protocol, []);
};

[@deriving show]
type msg =
  | Server(Exthost.Transport.msg);

type outmsg =
  | Nothing
  | OpenFilesAndFolders({
      files: list(FpExp.t(FpExp.absolute)),
      folder: option(FpExp.t(FpExp.absolute)),
    });

let update = (msg, model) => {
  switch (msg) {
  | Server(Received({body, _}: Exthost.Transport.Packet.t)) =>
    let protocolMsg = body |> Protocol.fromBytes;

    let eff =
      switch (protocolMsg) {
      | Protocol.OpenFilesAndFolder({files, folderToOpen}) =>
        let files' = files |> List.filter_map(FpExp.absoluteCurrentPlatform);

        let folder' =
          folderToOpen
          |> Utility.OptionEx.flatMap(FpExp.absoluteCurrentPlatform);

        OpenFilesAndFolders({files: files', folder: folder'});
      };
    (model, eff);
  | Server(_) => (model, Nothing)
  | _ =>
    prerr_endline("MSG: " ++ show_msg(msg));
    (model, Nothing);
  };
};

let pipe = ({namedPipe, _}) => namedPipe;

type params = NamedPipe.t;
module Sub =
  Isolinear.Sub.Make({
    type nonrec msg = Exthost.Transport.msg;
    type nonrec params = params;
    type state = result(Exthost.Transport.t, string);

    let name = "Feature_ClientServer.sub";

    let id = namedPipe => NamedPipe.toString(namedPipe);

    let init = (~params as namedPipe, ~dispatch) => {
      let res =
        Exthost.Transport.start(
          ~namedPipe=NamedPipe.toString(namedPipe),
          ~dispatch,
        );

      switch (res) {
      | Ok(_transport) => prerr_endline("Started!")
      | Error(msg) => prerr_endline("Failed to start: " ++ msg)
      };
      res;
    };

    let update = (~params as _, ~state, ~dispatch as _) => {
      state;
    };

    let dispose = (~params as _, ~state) => {
      switch (state) {
      | Ok(transport) => Exthost.Transport.close(transport)
      | Error(_) => ()
      };
    };
  });

let sub = ({namedPipe, _}) =>
  Sub.create(namedPipe) |> Isolinear.Sub.map(msg => Server(msg));

module RemoteCommand = {
  type t = unit;

  let cli = _cli => ();
};

module Client = {
  let openFiles = (~server: string, ~files, ~folderToOpen) => {
    let (promise, resolve) = Lwt.task();
    let dispatch = msg =>
      Exthost.Transport.(
        {
          switch (msg) {
          | Connected =>
            print_endline("Connected!");
            Lwt.wakeup(resolve, ());
          | Received({body, _}: Packet.t) =>
            ();
            body |> Protocol.fromBytes |> Protocol.show |> prerr_endline;
          | Error(msg) => prerr_endline(msg)
          | Disconnected => print_endline("Disconnected")
          };
        }
      );
    switch (Exthost.Transport.connect(~namedPipe=server, ~dispatch)) {
    | Ok(transport) =>
      promise
      |> Utility.LwtEx.flatMap(() => {
           transport
           |> Exthost.Transport.(
                send(
                  ~packet=
                    Packet.create(
                      ~id=0,
                      ~packetType=Packet.Regular,
                      ~bytes=
                        Protocol.(
                          toBytes(OpenFilesAndFolder({files, folderToOpen}))
                        ),
                    ),
                )
              );

           Exthost.Transport.close(transport);
           Lwt.return();
         })
    | Error(msg) => Lwt.fail_with(msg)
    };
  };
};

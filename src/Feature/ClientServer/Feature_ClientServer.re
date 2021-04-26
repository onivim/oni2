open Oni_Core;
module NamedPipe = Exthost.NamedPipe;

type model = {namedPipe: NamedPipe.t};

module Log = (val Oni_Core.Log.withNamespace("Feature_ClientServer"));

let create = () => {
  namedPipe:
    NamedPipe.create("clientserver:" ++ string_of_int(Luv.Pid.getpid())),
};

module Protocol = {
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
      let namedPipeStr = NamedPipe.toString(namedPipe);
      Log.infof(m => m("Starting server: %s", namedPipeStr));
      let res = Exthost.Transport.start(~namedPipe=namedPipeStr, ~dispatch);

      switch (res) {
      | Ok(_transport) => Log.info("Started pipe successfully")
      | Error(msg) => Log.errorf(m => m("Failed to start pipe: %s", msg))
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

module Client = {
  let openFiles = (~server: string, ~files, ~folderToOpen) => {
    let (promise, resolve) = Lwt.task();
    let dispatch = msg =>
      Exthost.Transport.(
        {
          switch (msg) {
          | Connected =>
            Log.info("Connected!");
            Lwt.wakeup(resolve, ());
          | Received(_) => ()
          | Error(msg) => Log.error(msg)
          | Disconnected => Log.info("Disconnected")
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

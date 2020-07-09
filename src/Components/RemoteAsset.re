open Oni_Core;
open Oni_Core.Utility;

open Revery;
open Revery.UI;

type state = {
   downloadedFileSrc: option(string),
};

let initial = {
   downloadedFileSrc: None
};

type action =
| DownloadSuccess(string);

let reducer = (action, model) => switch(action) {
| DownloadSuccess(filePath) => {downloadedFileSrc: Some(filePath)}
};

let%component make = (
    ~src: string,
    ~flatMap: string => Lwt.t('a),
    ~children as renderItem: 'a => React.element(React.node),
    (),
) => {
   ignore(renderItem);

   let%hook (state, localDispatch) = Hooks.reducer(~initialState=initial, reducer)

   let%hook () = Hooks.effect(OnMountAndIf((!=), src), () => {
    prerr_endline ("Mounted or src changed: " ++ src);
    let promise = Service_Net.Request.download(
    ~setup=Oni_Core.Setup.init(),
    src)
    |> LwtEx.flatMap(flatMap);

    Lwt.on_success(promise, (res) =>{
       localDispatch(DownloadSuccess(res));
       prerr_endline ("RESULT: " ++ res);
    });

    None
   });

   switch (state.downloadedFileSrc) {
   | None => <View />
   | Some(filePath) => renderItem(filePath)
   };
}

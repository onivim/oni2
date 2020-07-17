
[@deriving show]
type msg = 
| MessageReceived({
    msg: Exthost.Msg.MessageService.msg,
    resolver: [@opaque] option(Lwt.u(Exthost.Message.handle)),
})

module Msg = {
    let exthost = (~dispatch, msg) => {
        dispatch(MessageReceived({
            msg,
            resolver: None
        }));
        
        Lwt.fail_with("Not hooked up");
    };
};

type model = unit;

let initial = ();

type outmsg =
| Nothing
| Effect(Isolinear.Effect.t(msg));

let update = (msg, model) => {
    prerr_endline ("hit update");
    failwith("update");
    (model, Nothing);
}

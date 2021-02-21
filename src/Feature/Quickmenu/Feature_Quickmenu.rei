// SCHEMA

module Schema: {
    type menu('outmsg);

    let menu: (
        ~onItemFocused: 'item => 'outmsg =?,
        ~onItemSelected: 'item' => 'outmsg =? ,
        ~onCancelled: unit => 'outmsg =?,
        ~initialItems: list('item),
    ) => menu('outmsg);

    let map: ('outmsg => 'b, menu('outmsg)) => menu('b);
};

// MODEL

[@deriving show]
type msg;

type model('outmsg);

let initial: model(_);

let show: (~menu: Schema.menu('outmsg), model('outmsg)) => model('outmsg);

// UPDATE

type outmsg('action) = 
| Action('action)
| Nothing;

let update: (msg, model('action)) => (model('action), outmsg('action));

// SUBSCRIPTION

let sub: (model('action)) => Isolinear.Sub.t(msg);

module Schema = {
    type menu('outmsg) = unit;

    let menu = (
        ~onItemFocused =?,
        ~onItemSelected =?,
        ~onCancelled =?,
        ~initialItems
    ) => ();

    let map = (f, model) => model;
};

// MODEL

[@deriving show]
type msg = unit;

type model('outmsg) = unit;

let initial = ();

let show = (~menu, model) => model;

// UPDATE

type outmsg('action) = 
| Action('action)
| Nothing;

let update = (_msg, model) => (model, Nothing)

// SUBSCRIPTION

let sub = (_model) => Isolinear.Sub.none;

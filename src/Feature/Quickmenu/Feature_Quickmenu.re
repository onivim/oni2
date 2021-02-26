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

let focus = (~index, model) => {
    prerr_endline ("!! FOCUS");
    model
};

let next = (model) => {
    prerr_endline("!! NEXT");
    model
};

let prev = (model) => {
    prerr_endline("!! PREV");
    model;
}

let cancel = (model) => {
    prerr_endline ("!! CANCEL");
    model;
}

let select = (model) => {
    prerr_endline ("!! SELECT");
    (model, Isolinear.Effect.none);
}

// UPDATE

type outmsg('action) = 
| Action('action)
| Nothing;

let update = (_msg, model) => (model, Nothing)

// SUBSCRIPTION

let sub = (_model) => Isolinear.Sub.none;

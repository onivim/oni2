module Model = Oni_Model;

let onStateChange = (updater, initialState, action, onChange) => {
  let (storeDispatch, _) =
    Isolinear.Store.create(~initialState, ~updater, ());

  action |> storeDispatch |> fst |> onChange;
};

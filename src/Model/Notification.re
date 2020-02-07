[@deriving show({with_path: false})]
type t = {
  id: int,
  kind,
  message: string,
}

and kind =
  | Success
  | Info
  | Warning
  | Error;

module Internal = {
  let generateId = {
    let lastId = ref(0);

    () => {
      lastId := lastId^ + 1;
      lastId^;
    };
  };
};

let create = (~kind=Info, message) => {
  id: Internal.generateId(),
  kind,
  message,
};

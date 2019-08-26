open Actions;

let nextId = ref(0);

type t = Actions.notification;

let create = (~notificationType=Actions.Info, ~message, ~title, ()) => {
  let id = nextId^;
  incr(nextId);

  {id, notificationType, message, title};
};

open Oni_Core.Utility;

module M = Msgpck;

let convertNeovimExtType = (buffer: M.t) =>
  switch (buffer) {
  | M.Ext(kind, id) => Some((kind, convertUTF8string(id)))
  | _ => None
  };

let getAtomicCallsResponse = response =>
  switch (response) {
  | M.List(result) =>
    /**
    The last argument in an atomic call is either NIL or an
    array of three items  a three-element array with the zero-based
    index of the call which resulted in an error, the error type
    and the error message. If an error occurred,
    the values from all preceding calls will still be returned.
   */
    (
      switch (List.rev(result)) {
      | [] => (None, None)
      | [M.Nil, M.List(responseItems)] => (None, Some(responseItems))
      | [errors, M.List(responseItems)] => (
          Some(errors),
          Some(responseItems),
        )
      | _ => (None, None)
      }
    )
  | _ => (None, None)
  };

  open Oni_Core;
  type t = {
    uri: Uri.t,
    range: OneBasedRange.t,
  }

  let decode = {
    open Json.Decode;
    obj(({field, _}) => {
      uri: field.required("uri", Uri.decode), 
      range: field.required("range", OneBasedRange.decode)
    });
  };

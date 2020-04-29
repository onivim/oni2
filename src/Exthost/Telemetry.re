[@deriving show]
type msg =
  | PublicLog({
      eventName: string,
      data: Yojson.Safe.t,
    })
  | PublicLog2({
      eventName: string,
      data: Yojson.Safe.t,
    });

let handle = (method, args: Yojson.Safe.t) => {
  switch (method, args) {
  | ("$publicLog", `List([`String(eventName), data, ..._])) =>
    Ok(PublicLog({eventName, data}))
  | ("$publicLog2", `List([`String(eventName), data, ..._])) =>
    Ok(PublicLog2({eventName, data}))
  | _ => Error("Unhandled method: " ++ method)
  };
};

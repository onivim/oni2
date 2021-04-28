// SingleInstance.rei
//
// This is essentially a port of the `node-single-instance` library, into native reason:
// https://github.com/pierrefourgeaud/node-single-instance/blob/master/index.js

let lock:
  (
    ~name: string,
    ~arguments: 'a,
    ~serialize: 'a => Bytes.t,
    ~deserialize: Bytes.t => 'a,
    ~firstInstance: 'a => unit,
    ~additionalInstance: 'a => unit
  ) =>
  unit;

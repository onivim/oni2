type callback = unit => unit;
type t = callback => unit;

let mainThread = Revery.App.runOnMainThread;
let immediate = cb => cb();

let run = (cb, scheduler) => scheduler(cb);

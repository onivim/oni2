let waitForCondition = (~timeout=1.0, f) => {
   let thread = Thread.create(() => {
        
  let s = Unix.gettimeofday();
  while (!f() && Unix.gettimeofday() -. s < timeout) {
    Unix.sleepf(0.0005);
  };
    }, ());

  Thread.join(thread);
};


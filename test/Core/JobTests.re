open Oni_Core;
open TestFramework;

describe("Job", ({describe, _}) =>
  describe("tick", ({test, _}) =>
    test("does multiple iterations of work", ({expect}) => {
      let f = ((), c) =>
        if (c == 3) {
          (true, (), 3);
        } else {
          (false, (), c + 1);
        };

      let job =
        Job.create(~f, ~initialCompletedWork=0, ~budget=Time.ms(8), ());

      // Tick will run the job for 8 ms.
      // That should be plenty of time to do a few iterations of [f]...
      let job = Job.tick(job);

      expect.bool(Job.isComplete(job)).toBe(true);
    })
  )
);

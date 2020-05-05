open TestFramework;

open Revery;
open Oni_Core.Utility;

describe("FunEx", ({describe, _}) => {
  describe("throttle", ({test, describe, _}) => {
    let setup = () => {
      let time: ref(Time.t) = ref(Time.zero);
      module TestTick =
        Revery.Internal.Tick.Make({
          let time = () => time^;
        });

      let setTime = timeFloat => {
        time := Time.ofFloatSeconds(timeFloat);
        TestTick.pump();
      };

      let throttle =
        FunEx.throttle(~timeout=TestTick.timeout, ~time=Time.seconds(1));
      (setTime, throttle);
    };

    describe("leading=false", ({test, _}) => {
  
    });

    describe("trailing=false", ({test, _}) => {
  
    });

    test("call once, let time expire, call again", ({expect, _}) => {
      let (setTime, throttle) = setup();

      let calls = ref([]);
      let f = v => calls := [v, ...calls^];

      let throttledF = throttle(f);
      throttledF(1);

      expect.equal([1], calls^);

      // Increment 1 second + 1 millisecond
      setTime(1.001);

      throttledF(2);
      expect.equal([2, 1], calls^);
    });

    test(
      "call a bunch of times, only first and last should get called",
      ({expect, _}) => {
      let (setTime, throttle) = setup();

      let calls = ref([]);
      let f = v => calls := [v, ...calls^];

      let throttledF = throttle(f);
      throttledF(1);
      throttledF(2);
      throttledF(3);
      throttledF(4);

      expect.equal([1], calls^);

      // Increment 1 second + 1 millisecond
      setTime(1.001);

      expect.equal([4, 1], calls^);

      throttledF(5);
      expect.equal([5, 4, 1], calls^);
    });
  })
});

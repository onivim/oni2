open TestFramework;

open Revery;
open Oni_Core.Utility;

describe("FunEx", ({describe, _}) => {
  describe("throttle", ({test, describe, _}) => {
    let setup = (~leading=true, ~trailing=true, ()) => {
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
        FunEx.throttle(
          ~leading,
          ~trailing,
          ~timeout=TestTick.timeout,
          ~time=Time.seconds(1),
        );
      (setTime, throttle);
    };

    describe("leading=false", ({test, _}) => {
      test(
        "leading edge does not get fired, only trailing edge", ({expect, _}) => {
        let (setTime, throttle) = setup(~leading=false, ());

        let calls = ref([]);
        let f = v => calls := [v, ...calls^];

        let throttledF = throttle(f);
        throttledF(1);

        // Because leading is off, first call shouldn't get dispatched yet
        expect.equal([], calls^);

        throttledF(2);

        // Increment 1 second + 1 millisecond
        setTime(1.001);

        expect.equal([2], calls^);
      })
    });

    describe("trailing=false", ({test, _}) => {
      test(
        "trailing edge does not get fired, only leading edge", ({expect, _}) => {
        let (setTime, throttle) = setup(~trailing=false, ());

        let calls = ref([]);
        let f = v => calls := [v, ...calls^];

        let throttledF = throttle(f);
        throttledF(1);

        expect.equal([1], calls^);

        throttledF(2);

        // Increment 1 second + 1 millisecond
        setTime(1.001);

        // Because trailing is off, the [2] shouldn't have gotten dispatched
        expect.equal([1], calls^);
      })
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

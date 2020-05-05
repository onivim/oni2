open TestFramework;

open Revery;
open Oni_Core.Utility;

describe("FunEx", ({describe, _}) => {
  describe("debounce1", ({test, _}) => {
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

      let debounce1 =
        FunEx.debounce1(~timeout=TestTick.timeout, ~time=Time.seconds(1));
      (setTime, debounce1);
    };

    test("call once, let time expire, call again", ({expect, _}) => {
      let (setTime, debounce1) = setup();

      let calls = ref([]);
      let f = v => calls := [v, ...calls^];

      let debouncedF = debounce1(f);
      debouncedF(1);

      expect.equal([1], calls^);

      // Increment 1 second + 1 millisecond
      setTime(1.001);

      debouncedF(2);
      expect.equal([2, 1], calls^);
    });

    test(
      "call a bunch of times, only first and last should get called",
      ({expect, _}) => {
      let (setTime, debounce1) = setup();

      let calls = ref([]);
      let f = v => calls := [v, ...calls^];

      let debouncedF = debounce1(f);
      debouncedF(1);
      debouncedF(2);
      debouncedF(3);
      debouncedF(4);

      expect.equal([1], calls^);

      // Increment 1 second + 1 millisecond
      setTime(1.001);

      expect.equal([4, 1], calls^);

      debouncedF(5);
      expect.equal([5, 4, 1], calls^);
    });
  })
});

open Oni_Core;
open TestFramework;

describe("RingBuffer", ({test, _}) => {
  test("single item", ({expect, _}) => {
    let ringBuffer = RingBuffer.make(~capacity=1, "a");

    expect.equal(RingBuffer.size(ringBuffer), 0);

    RingBuffer.push("b", ringBuffer);

    expect.equal(RingBuffer.size(ringBuffer), 1);
    expect.equal(RingBuffer.getAt(0, ringBuffer), "b");

    RingBuffer.push("c", ringBuffer);
    expect.equal(RingBuffer.size(ringBuffer), 1);
    expect.equal(RingBuffer.getAt(0, ringBuffer), "c");
  });

  test("two item", ({expect, _}) => {
    let ringBuffer = RingBuffer.make(~capacity=2, "a");

    expect.equal(RingBuffer.size(ringBuffer), 0);

    RingBuffer.push("b", ringBuffer);
    RingBuffer.push("c", ringBuffer);

    expect.equal(RingBuffer.size(ringBuffer), 2);
    expect.equal(RingBuffer.getAt(0, ringBuffer), "b");
    expect.equal(RingBuffer.getAt(1, ringBuffer), "c");

    RingBuffer.push("d", ringBuffer);
    expect.equal(RingBuffer.size(ringBuffer), 2);
    expect.equal(RingBuffer.getAt(0, ringBuffer), "c");
    expect.equal(RingBuffer.getAt(1, ringBuffer), "d");
  });
});

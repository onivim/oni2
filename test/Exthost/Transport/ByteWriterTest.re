open TestFramework;
open Exthost;

module Packet = Transport.Packet;
module ByteWriter = Transport.ByteWriter;

let buf0 = Bytes.make(0, 'a') |> Luv.Buffer.from_bytes;
let buf1 = Bytes.make(1, 'a') |> Luv.Buffer.from_bytes;
let buf5 = Bytes.make(5, 'b') |> Luv.Buffer.from_bytes;

describe("Transport.ByteWriter", ({test, _}) => {
  test("single item buffer, empty write", ({expect, _}) => {
    let bw = ByteWriter.create(1);
    let (newByteWriter, outBuffer) = ByteWriter.write(buf0, bw);

    expect.bool(ByteWriter.isFull(newByteWriter)).toBe(false);

    expect.int(Luv.Buffer.size(outBuffer)).toBe(0);
  });
  test("zero item buffer, overflow", ({expect, _}) => {
    let bw = ByteWriter.create(0);
    let (newByteWriter, outBuffer) = ByteWriter.write(buf1, bw);

    expect.bool(ByteWriter.isFull(newByteWriter)).toBe(true);

    expect.int(Luv.Buffer.size(outBuffer)).toBe(1);
  });
  test("single item buffer, single write", ({expect, _}) => {
    let bw = ByteWriter.create(1);
    let (newByteWriter, outBuffer) = ByteWriter.write(buf1, bw);

    expect.bool(ByteWriter.isFull(newByteWriter)).toBe(true);
    expect.int(Luv.Buffer.size(outBuffer)).toBe(0);
  });

  test("single item buffer, overflow", ({expect, _}) => {
    let bw = ByteWriter.create(1);
    let (newByteWriter, outBuffer) = ByteWriter.write(buf5, bw);

    expect.bool(ByteWriter.isFull(newByteWriter)).toBe(true);
    expect.int(Luv.Buffer.size(outBuffer)).toBe(4);
  });

  test("larger buffer", ({expect, _}) => {
    let bw = ByteWriter.create(10);
    let (newByteWriter, outBuffer) = ByteWriter.write(buf5, bw);

    expect.bool(ByteWriter.isFull(newByteWriter)).toBe(false);
    expect.int(Luv.Buffer.size(outBuffer)).toBe(0);

    let (newByteWriter, outBuffer) = ByteWriter.write(buf5, newByteWriter);
    expect.bool(ByteWriter.isFull(newByteWriter)).toBe(true);
    expect.int(Luv.Buffer.size(outBuffer)).toBe(0);
  });
});

/* open Oni_Core; */
open TestFramework;

open Helpers;

open Oni_Core.Types;
module Buffer = Oni_Core.Buffer;

describe("Buffer", ({describe, _}) =>
  describe("update", ({test, _}) => {
    test("empty buffer w/ update", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let update =
        BufferUpdate.create(
          ~startLine=0,
          ~endLine=1,
          ~lines=["a"],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a"|]);
    });

    test("update single line", ({expect}) => {
      let buffer = Buffer.ofLines([|"a"|]);
      let update =
        BufferUpdate.create(
          ~startLine=0,
          ~endLine=1,
          ~lines=["abc"],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"abc"|]);
    });

    test("delete line", ({expect}) => {
      let buffer = Buffer.ofLines([|"a"|]);
      let update =
        BufferUpdate.create(
          ~startLine=0,
          ~endLine=1,
          ~lines=[],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [||]);
    });

    test("update single line", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.create(
          ~startLine=1,
          ~endLine=2,
          ~lines=["d", "e", "f"],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
    });

    test("add new line after buffer", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.create(
          ~startLine=3,
          ~endLine=3,
          ~lines=["d"],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "b", "c", "d"|]);
    });

    test("version gets updated with buffer update", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.create(
          ~startLine=3,
          ~endLine=3,
          ~lines=["d"],
          ~version=5,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "b", "c", "d"|]);
      expect.int(updatedBuffer.version).toBe(5);
    });

    test("buffer update with lower version gets rejected", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|], ~version=6);
      let update =
        BufferUpdate.create(
          ~startLine=3,
          ~endLine=3,
          ~lines=["d"],
          ~version=5,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "b", "c"|]);
      expect.int(updatedBuffer.version).toBe(6);
    });
  })
);

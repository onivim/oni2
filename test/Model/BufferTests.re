/* open Oni_Core; */
open TestFramework;

open Helpers;

open Oni_Core.Types;
module Buffer = Oni_Model.Buffer;

describe("Buffer", ({describe, _}) =>
  describe("update", ({test, _}) => {
    test("empty buffer w/ update", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=0,
          ~endLine=1,
          ~lines=[|"a"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a"|]);
    });

    test("BufEnter update does not duplicate content", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "d", "e", "f", "c"|]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~isFull=true,
          ~startLine=0,
          ~endLine=-1,
          ~lines=[|"a", "d", "e", "f", "c"|],
          ~version=2,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
    });

    test(
      "BufEnter update does not duplicate content, 1-based indices",
      ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "d", "e", "f", "c"|]);
      let update =
        BufferUpdate.createFromOneBasedIndices(
          ~isFull=true,
          ~startLine=1,
          ~endLine=-1,
          ~lines=[|"a", "d", "e", "f", "c"|],
          ~version=2,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
    });

    test("update single line", ({expect}) => {
      let buffer = Buffer.ofLines([|"a"|]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=0,
          ~endLine=1,
          ~lines=[|"abc"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"abc"|]);
    });

    test("delete line", ({expect}) => {
      let buffer = Buffer.ofLines([|"a"|]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=0,
          ~endLine=1,
          ~lines=[||],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [||]);
    });

    test("update single line", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=1,
          ~endLine=2,
          ~lines=[|"d", "e", "f"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
    });

    test("add new line after buffer", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=3,
          ~endLine=3,
          ~lines=[|"d"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "b", "c", "d"|]);
    });

    test("version gets updated with buffer update", ({expect}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=3,
          ~endLine=3,
          ~lines=[|"d"|],
          ~version=5,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "b", "c", "d"|]);
      expect.int(Buffer.getVersion(updatedBuffer)).toBe(5);
    });

    test("buffer update with lower version gets rejected", ({expect}) => {
      let buffer = Buffer.ofLines([||]);
      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=3,
          ~endLine=3,
          ~lines=[|"d"|],
          ~version=6,
          (),
        );
      let bufferUpdate1 = Buffer.update(buffer, update);

      let update =
        BufferUpdate.createFromZeroBasedIndices(
          ~startLine=3,
          ~endLine=3,
          ~lines=[|"e"|],
          ~version=5,
          (),
        );

      let updatedBuffer = Buffer.update(bufferUpdate1, update);

      validateBuffer(expect, updatedBuffer, [|"d"|]);
      expect.int(Buffer.getVersion(updatedBuffer)).toBe(6);
    });
  })
);

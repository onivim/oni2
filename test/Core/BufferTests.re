open Oni_Core;

module LineNumber = EditorCoreTypes.LineNumber;

open TestFramework;
open Helpers;

describe("Buffer", ({describe, _}) =>
  describe("update", ({test, _}) => {
    test("empty buffer w/ update", ({expect, _}) => {
      let buffer = Buffer.ofLines([||]);
      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.zero,
          ~endLine=LineNumber.ofZeroBased(1),
          ~lines=[|"a"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a"|]);
    });

    test("BufEnter update does not duplicate content", ({expect, _}) => {
      let buffer = Buffer.ofLines([|"a", "d", "e", "f", "c"|]);
      let update =
        BufferUpdate.create(
          ~isFull=true,
          ~startLine=LineNumber.zero,
          ~endLine=LineNumber.ofZeroBased(-1),
          ~lines=[|"a", "d", "e", "f", "c"|],
          ~version=2,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
    });

    test(
      "BufEnter update does not duplicate content, 1-based indices",
      ({expect, _}) => {
      let buffer = Buffer.ofLines([|"a", "d", "e", "f", "c"|]);
      let update =
        BufferUpdate.create(
          ~isFull=true,
          ~startLine=LineNumber.ofZeroBased(1),
          ~endLine=LineNumber.ofZeroBased(-1),
          ~lines=[|"a", "d", "e", "f", "c"|],
          ~version=2,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
    });

    test("update single line", ({expect, _}) => {
      let buffer = Buffer.ofLines([|"a"|]);
      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.zero,
          ~endLine=LineNumber.ofZeroBased(1),
          ~lines=[|"abc"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"abc"|]);
    });

    test("delete line", ({expect, _}) => {
      let buffer = Buffer.ofLines([|"a"|]);
      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.zero,
          ~endLine=LineNumber.ofZeroBased(1),
          ~lines=[||],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [||]);
    });

    test("update single line", ({expect, _}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.ofZeroBased(1),
          ~endLine=LineNumber.ofZeroBased(2),
          ~lines=[|"d", "e", "f"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
    });

    test("add new line after buffer", ({expect, _}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.ofZeroBased(3),
          ~endLine=LineNumber.ofZeroBased(3),
          ~lines=[|"d"|],
          ~version=1,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "b", "c", "d"|]);
    });

    test("version gets updated with buffer update", ({expect, _}) => {
      let buffer = Buffer.ofLines([|"a", "b", "c"|]);
      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.ofZeroBased(3),
          ~endLine=LineNumber.ofZeroBased(3),
          ~lines=[|"d"|],
          ~version=5,
          (),
        );
      let updatedBuffer = Buffer.update(buffer, update);
      validateBuffer(expect, updatedBuffer, [|"a", "b", "c", "d"|]);
      expect.int(Buffer.getVersion(updatedBuffer)).toBe(5);
    });

    test("buffer update with lower version gets rejected", ({expect, _}) => {
      let buffer = Buffer.ofLines([||]);
      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.ofZeroBased(3),
          ~endLine=LineNumber.ofZeroBased(3),
          ~lines=[|"d"|],
          ~version=6,
          (),
        );
      let bufferUpdate1 = Buffer.update(buffer, update);

      let update =
        BufferUpdate.create(
          ~startLine=LineNumber.ofZeroBased(3),
          ~endLine=LineNumber.ofZeroBased(3),
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

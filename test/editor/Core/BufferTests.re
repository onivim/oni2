/* open Oni_Core; */
open TestFramework;

open Helpers;

open Oni_Core.Types;
module Buffer = Oni_Core.Buffer;

describe("update", ({test, _}) => {
  test("empty buffer w/ update", ({expect}) => {
    let buffer = Buffer.ofLines([||]);
    let update = BufferUpdate.create(~startLine=0, ~endLine=1, ~lines=["a"], ());
    let updatedBuffer = Buffer.update(buffer, update);
    validateBuffer(expect, updatedBuffer, [|"a"|]);
  });

  test("update single line", ({expect}) => {
    let buffer = Buffer.ofLines([|"a"|]);
    let update = BufferUpdate.create(~startLine=0, ~endLine=1, ~lines=["abc"], ());
    let updatedBuffer = Buffer.update(buffer, update);
    validateBuffer(expect, updatedBuffer, [|"abc"|]);
  });

  test("delete line", ({expect}) => {
    let buffer = Buffer.ofLines([|"a"|]);
    let update = BufferUpdate.create(~startLine=0, ~endLine=1, ~lines=[], ());
    let updatedBuffer = Buffer.update(buffer, update);
    validateBuffer(expect, updatedBuffer, [||]);
  });

  test("update single line", ({expect}) => {
    let buffer = Buffer.ofLines([|"a", "b", "c"|]);
    let update = BufferUpdate.create(~startLine=1, ~endLine=2, ~lines=["d", "e", "f"], ());
    let updatedBuffer = Buffer.update(buffer, update);
    validateBuffer(expect, updatedBuffer, [|"a", "d", "e", "f", "c"|]);
  });

  test("add new line after buffer", ({expect}) => {
    let buffer = Buffer.ofLines([|"a", "b", "c"|]);
    prerr_endline ("BEFORE");
    let update = BufferUpdate.create(~startLine=3, ~endLine=3, ~lines=["d"], ());
    let updatedBuffer = Buffer.update(buffer, update);
    prerr_endline ("AFTER");
    validateBuffer(expect, updatedBuffer, [|"a", "b", "c", "d"|]);
  });
});

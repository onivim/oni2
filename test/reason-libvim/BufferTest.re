open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));
let key = s => ignore(Vim.key(s));

describe("Buffer", ({describe, _}) => {
  describe("load", ({test, _}) => {
    test("loaded buffer does not change current buffer", ({expect, _}) => {
      let originalBuffer = resetBuffer();

      let loadedBuffer = Buffer.loadFile("test/reason-libvim/lines_100.txt");

      expect.equal(Buffer.getCurrent(), originalBuffer);
      expect.bool(loadedBuffer != originalBuffer).toBe(true);
    });

    test("can read lines of loaded buffer", ({expect, _}) => {
      let originalBuffer = resetBuffer();

      let loadedBuffer = Buffer.loadFile("test/reason-libvim/lines_100.txt");

      expect.int(Buffer.getLineCount(loadedBuffer)).toBe(100);
      expect.equal(Buffer.getCurrent(), originalBuffer);
    });
  });
  describe("fileformats", ({test, _}) => {
    test("get / set", ({expect, _}) => {
      let buffer = resetBuffer();

      Buffer.setLineEndings(buffer, CRLF);
      let lineEndings = Buffer.getLineEndings(buffer);
      expect.equal(lineEndings, Some(CRLF));

      Buffer.setLineEndings(buffer, LF);
      let lineEndings = Buffer.getLineEndings(buffer);
      expect.equal(lineEndings, Some(LF));
    });
    test("get: crlf", ({expect, _}) => {
      let buffer = Helpers.resetBuffer("integration_test/test.crlf");
      let lineEndings = Buffer.getLineEndings(buffer);

      expect.equal(lineEndings, Some(CRLF));
    });
    test("get: lf", ({expect, _}) => {
      let buffer = Helpers.resetBuffer("integration_test/test.lf");
      let lineEndings = Buffer.getLineEndings(buffer);

      expect.equal(lineEndings, Some(LF));
    });
  });
  describe("read-only", ({test, _}) => {
    test("get / set", ({expect, _}) => {
      let buffer = resetBuffer();

      Buffer.setReadOnly(~readOnly=true, buffer);
      expect.equal(Buffer.isReadOnly(buffer), true);

      Buffer.setReadOnly(~readOnly=false, buffer);
      expect.equal(Buffer.isReadOnly(buffer), false);
    })
  });
  describe("modifiable", ({test, _}) => {
    test("get / set", ({expect, _}) => {
      let buffer = resetBuffer();
      Buffer.setModifiable(~modifiable=false, buffer);
      expect.equal(Buffer.isModifiable(buffer), false);

      Buffer.setModifiable(~modifiable=true, buffer);
      expect.equal(Buffer.isModifiable(buffer), true);
    })
  });
  describe("setLines", ({test, _}) => {
    test("add a line at beginning", ({expect, _}) => {
      let buffer = resetBuffer();

      let updates: ref(list(BufferUpdate.t)) = ref([]);
      let dispose = Buffer.onUpdate(upd => updates := [upd, ...updates^]);

      Buffer.setLines(
        ~shouldAdjustCursors=false,
        ~stop=LineNumber.zero,
        ~lines=[|"abc"|],
        buffer,
      );
      let line0 = Buffer.getLine(buffer, LineNumber.zero);
      let line1 = Buffer.getLine(buffer, LineNumber.(zero + 1));
      let lineCount = Buffer.getLineCount(buffer);
      expect.int(lineCount).toBe(4);
      expect.string(line0).toEqual("abc");
      expect.string(line1).toEqual("This is the first line of a test file");

      expect.int(List.length(updates^)).toBe(1);
      let bu: BufferUpdate.t = List.hd(updates^);

      expect.int(bu.startLine).toBe(1);
      expect.int(bu.endLine).toBe(1);
      expect.int(Array.length(bu.lines)).toBe(1);
      expect.string(bu.lines[0]).toEqual("abc");

      dispose();
    });
    test("change line in middle", ({expect, _}) => {
      let buffer = resetBuffer();

      let line1Index = LineNumber.(zero + 1);
      let line2Index = LineNumber.(zero + 2);

      let lines = [|"abc"|];
      Buffer.setLines(
        ~shouldAdjustCursors=false,
        ~start=line1Index,
        ~stop=line2Index,
        ~lines,
        buffer,
      );
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, LineNumber.zero);
      let line1 = Buffer.getLine(buffer, LineNumber.(zero + 1));
      let line2 = Buffer.getLine(buffer, LineNumber.(zero + 2));
      expect.int(lineCount).toBe(3);
      expect.string(line0).toEqual("This is the first line of a test file");
      expect.string(line1).toEqual("abc");
      expect.string(line2).toEqual("This is the third line of a test file");
    });
    test("should increment version", ({expect, _}) => {
      let buffer = resetBuffer();

      let line1Index = LineNumber.(zero + 1);
      let line2Index = LineNumber.(zero + 2);

      let startVersion = Buffer.getVersion(buffer);

      let lines = [|"abc"|];
      Buffer.setLines(
        ~shouldAdjustCursors=false,
        ~start=line1Index,
        ~stop=line2Index,
        ~lines,
        buffer,
      );
      let newVersion = Buffer.getVersion(buffer);
      expect.equal(newVersion > startVersion, true);
    });
    test("should be marked as modified", ({expect, _}) => {
      let buffer = resetBuffer();

      expect.bool(Buffer.isModified(buffer)).toBe(false);

      let line1Index = LineNumber.(zero + 1);
      let line2Index = LineNumber.(zero + 2);

      let lines = [|"abc"|];
      Buffer.setLines(
        ~shouldAdjustCursors=false,
        ~start=line1Index,
        ~stop=line2Index,
        ~lines,
        buffer,
      );
      expect.bool(Buffer.isModified(buffer)).toBe(true);
    });
    test("replace whole buffer - set both start / stop", ({expect, _}) => {
      let buffer = resetBuffer();

      Buffer.setLines(
        ~shouldAdjustCursors=false,
        ~start=LineNumber.zero,
        ~stop=LineNumber.(zero + 4),
        ~lines=[|"abc"|],
        buffer,
      );
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, LineNumber.zero);
      expect.int(lineCount).toBe(1);
      expect.string(line0).toEqual("abc");
    });
    test("replace whole buffer - set just start", ({expect, _}) => {
      let buffer = resetBuffer();

      Buffer.setLines(
        ~shouldAdjustCursors=false,
        ~start=LineNumber.zero,
        ~lines=[|"abc"|],
        buffer,
      );
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, LineNumber.zero);
      expect.int(lineCount).toBe(1);
      expect.string(line0).toEqual("abc");
    });
    test("replace whole buffer - not setting start / stop", ({expect, _}) => {
      let buffer = resetBuffer();

      let updates: ref(list(BufferUpdate.t)) = ref([]);
      let dispose = Buffer.onUpdate(upd => updates := [upd, ...updates^]);

      Buffer.setLines(~shouldAdjustCursors=false, ~lines=[|"abc"|], buffer);
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, LineNumber.zero);
      expect.int(lineCount).toBe(1);
      expect.string(line0).toEqual("abc");

      expect.int(List.length(updates^)).toBe(1);
      let bu: BufferUpdate.t = List.hd(updates^);

      expect.int(bu.startLine).toBe(1);
      expect.int(bu.endLine).toBe(4);
      expect.int(Array.length(bu.lines)).toBe(1);
      expect.string(bu.lines[0]).toEqual("abc");

      dispose();
    });
    test("add a line at end", ({expect, _}) => {
      let buffer = resetBuffer();

      let endPoint = Buffer.getLineCount(buffer) |> LineNumber.ofZeroBased;

      Buffer.setLines(
        ~shouldAdjustCursors=false,
        ~start=endPoint,
        ~lines=[|"abc"|],
        buffer,
      );
      let line3 = Buffer.getLine(buffer, LineNumber.(zero + 2));
      let line4 = Buffer.getLine(buffer, LineNumber.(zero + 3));
      let lineCount = Buffer.getLineCount(buffer);
      expect.int(lineCount).toBe(4);
      expect.string(line3).toEqual("This is the third line of a test file");
      expect.string(line4).toEqual("abc");
    });
  });
  describe("applyEdits", ({test, _}) => {
    let range = (startLine, startColumn, endLine, endColumn) =>
      CharacterRange.{
        start:
          CharacterPosition.{
            line: LineNumber.(zero + startLine),
            character: CharacterIndex.(zero + startColumn),
          },
        stop:
          CharacterPosition.{
            line: LineNumber.(zero + endLine),
            character: CharacterIndex.(zero + endColumn),
          },
      };

    test("insert string inside line", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit = Edit.{range: range(0, 1, 0, 1), text: [|"a"|]};

      let () =
        Buffer.applyEdits(~shouldAdjustCursors=false, ~edits=[edit], buffer)
        |> Result.get_ok;

      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(1));
      expect.string(line).toEqual("Tahis is the first line of a test file");
      expect.int(Buffer.getLineCount(buffer)).toBe(3);
    });
    test("insert string, adding a line", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit =
        Edit.{
          range: range(0, 1, 0, 1),
          text: [|"his is a whole new line", "T"|],
        };

      let () =
        Buffer.applyEdits(~shouldAdjustCursors=false, ~edits=[edit], buffer)
        |> Result.get_ok;

      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(1));
      expect.string(line).toEqual("This is a whole new line");
      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(2));
      expect.string(line).toEqual("This is the first line of a test file");
      expect.int(Buffer.getLineCount(buffer)).toBe(4);
    });
    test("insert string, verify onModified gets called", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let modifiedEvents = ref([]);
      let dispose =
        Buffer.onModifiedChanged((bufferId, modified) => {
          modifiedEvents := [(bufferId, modified), ...modifiedEvents^]
        });

      let edit =
        Edit.{
          range: range(0, 1, 0, 1),
          text: [|"his is a whole new line", "T"|],
        };

      let () =
        Buffer.applyEdits(~shouldAdjustCursors=false, ~edits=[edit], buffer)
        |> Result.get_ok;

      expect.int(modifiedEvents^ |> List.length).toBe(1);

      dispose();
    });
    test("delete line", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit = Edit.{range: range(0, 0, 1, 0), text: [||]};

      let () =
        Buffer.applyEdits(~shouldAdjustCursors=false, ~edits=[edit], buffer)
        |> Result.get_ok;

      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(1));
      expect.string(line).toEqual("This is the second line of a test file");

      expect.int(Buffer.getLineCount(buffer)).toBe(2);
    });
    test("delete multiple lines", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit = Edit.{range: range(0, 0, 2, 0), text: [||]};

      let () =
        Buffer.applyEdits(~shouldAdjustCursors=false, ~edits=[edit], buffer)
        |> Result.get_ok;

      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(1));
      expect.string(line).toEqual("This is the third line of a test file");

      expect.int(Buffer.getLineCount(buffer)).toBe(1);
    });
    test("utf8: delete character", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile-utf8.txt");

      let edit = Edit.{range: range(0, 14, 0, 15), text: [||]};

      let () =
        Buffer.applyEdits(~shouldAdjustCursors=false, ~edits=[edit], buffer)
        |> Result.get_ok;

      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(1));
      expect.string(line).toEqual("import 'κόσμε'");

      expect.int(Buffer.getLineCount(buffer)).toBe(1);
    });
  });
  describe("getLine", ({test, _}) =>
    test("single file", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");
      let line = Buffer.getLine(buffer, LineNumber.ofOneBased(1));
      expect.string(line).toEqual("This is the first line of a test file");
    })
  );
  describe("getLineCount", ({test, _}) =>
    test("single file", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");
      expect.int(Buffer.getLineCount(buffer)).toBe(3);
    })
  );
  describe("onModifiedChanged", ({test, _}) =>
    test(
      "switching to a new file should not trigger an onFilenameChanged event",
      ({expect, _}) => {
      let _ = resetBuffer();

      let modifyVals = ref([]);
      let dispose =
        Buffer.onModifiedChanged((_id, modified) =>
          modifyVals := [modified, ...modifyVals^]
        );

      input("i");

      // Switching modes shouldn't trigger a modified change..
      expect.int(List.length(modifyVals^)).toBe(0);

      // Typing a character should trigger a modified change...
      input("a");

      expect.int(List.length(modifyVals^)).toBe(1);
      expect.bool(List.hd(modifyVals^)).toBe(true);

      // Switching mode shouldn't trigger a modified change
      key("<esc>");
      expect.int(List.length(modifyVals^)).toBe(1);

      // ..undo should, though
      input("u");
      expect.int(List.length(modifyVals^)).toBe(2);
      expect.bool(List.hd(modifyVals^)).toBe(false);

      // ..and redo
      key("<c-r>");
      expect.int(List.length(modifyVals^)).toBe(3);
      expect.bool(List.hd(modifyVals^)).toBe(true);

      dispose();
    })
  );
  describe("onFilenameChanged", ({test, _}) => {
    test(
      "switching to a new file should not trigger an onFilenameChanged event",
      ({expect, _}) => {
      let _ = resetBuffer();

      let updates = ref([]);
      let dispose =
        Buffer.onFilenameChanged(meta => updates := [meta, ...updates^]);

      ignore(command("e! some-new-file.txt"): (Context.t, list(Effect.t)));

      /* A filename changed event should not have been triggered */
      expect.int(List.length(updates^)).toBe(0);

      dispose();
    });
    test(
      "saving to a new file should trigger an onFilenameChanged event",
      ({expect, _}) => {
      let _ = resetBuffer();

      let updates = ref([]);
      let dispose =
        Buffer.onFilenameChanged(meta => updates := [meta, ...updates^]);

      let string_opt = s =>
        switch (s) {
        | None => ""
        | Some(v) => v
        };

      let previousFilename =
        Buffer.getCurrent() |> Buffer.getFilename |> string_opt;
      ignore(
        command("sav! some-new-file-2.txt"): (Context.t, list(Effect.t)),
      );
      let newFilename =
        Buffer.getCurrent() |> Buffer.getFilename |> string_opt;

      expect.bool(String.equal(previousFilename, newFilename)).toBe(false);

      expect.int(List.length(updates^)).toBe(1);

      dispose();
    });
  });
  describe("onFiletypeChanged", ({test, _}) => {
    test("switching filetype should trigger event", ({expect, _}) => {
      let _ = resetBuffer();

      let updates = ref([]);
      let dispose =
        Buffer.onFiletypeChanged(meta =>
          updates := [meta.fileType, ...updates^]
        );

      ignore(command("set filetype=derp"): (Context.t, list(Effect.t)));

      /* A filename changed event should not have been triggered */
      expect.int(List.length(updates^)).toBe(1);
      /* An enter event should've been triggered */
      expect.bool(List.hd(updates^) == Some("derp")).toBe(true);

      dispose();
    })
  });
  describe("onWrite", ({test, _}) => {
    test("saving the file should trigger an onWrite event", ({expect, _}) => {
      let _ = resetBuffer();

      let writes = ref([]);
      let dispose = Buffer.onWrite(id => writes := [id, ...writes^]);

      ignore(
        command("w! some-new-file-again-write.txt"): (
                                                      Context.t,
                                                      list(Effect.t),
                                                    ),
      );

      expect.int(List.length(writes^)).toBe(1);

      dispose();
    })
  });
});

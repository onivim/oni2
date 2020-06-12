open EditorCoreTypes;
open Vim;
open TestFramework;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/testfile.txt");
let input = s => ignore(Vim.input(s));

describe("Buffer", ({describe, _}) => {
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

      Buffer.setLines(~stop=Index.zero, ~lines=[|"abc"|], buffer);
      let line0 = Buffer.getLine(buffer, Index.zero);
      let line1 = Buffer.getLine(buffer, Index.(zero + 1));
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

      let line1Index = Index.(zero + 1);
      let line2Index = Index.(zero + 2);

      let lines = [|"abc"|];
      Buffer.setLines(~start=line1Index, ~stop=line2Index, ~lines, buffer);
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, Index.zero);
      let line1 = Buffer.getLine(buffer, Index.(zero + 1));
      let line2 = Buffer.getLine(buffer, Index.(zero + 2));
      expect.int(lineCount).toBe(3);
      expect.string(line0).toEqual("This is the first line of a test file");
      expect.string(line1).toEqual("abc");
      expect.string(line2).toEqual("This is the third line of a test file");
    });
    test("should increment version", ({expect, _}) => {
      let buffer = resetBuffer();

      let line1Index = Index.(zero + 1);
      let line2Index = Index.(zero + 2);

      let startVersion = Buffer.getVersion(buffer);

      let lines = [|"abc"|];
      Buffer.setLines(~start=line1Index, ~stop=line2Index, ~lines, buffer);
      let newVersion = Buffer.getVersion(buffer);
      expect.equal(newVersion > startVersion, true);
    });
    test("should be marked as modified", ({expect, _}) => {
      let buffer = resetBuffer();

      expect.bool(Buffer.isModified(buffer)).toBe(false);

      let line1Index = Index.(zero + 1);
      let line2Index = Index.(zero + 2);

      let lines = [|"abc"|];
      Buffer.setLines(~start=line1Index, ~stop=line2Index, ~lines, buffer);
      expect.bool(Buffer.isModified(buffer)).toBe(true);
    });
    test("replace whole buffer - set both start / stop", ({expect, _}) => {
      let buffer = resetBuffer();

      Buffer.setLines(
        ~start=Index.zero,
        ~stop=Index.(zero + 4),
        ~lines=[|"abc"|],
        buffer,
      );
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, Index.zero);
      expect.int(lineCount).toBe(1);
      expect.string(line0).toEqual("abc");
    });
    test("replace whole buffer - set just start", ({expect, _}) => {
      let buffer = resetBuffer();

      Buffer.setLines(~start=Index.zero, ~lines=[|"abc"|], buffer);
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, Index.zero);
      expect.int(lineCount).toBe(1);
      expect.string(line0).toEqual("abc");
    });
    test("replace whole buffer - not setting start / stop", ({expect, _}) => {
      let buffer = resetBuffer();

      let updates: ref(list(BufferUpdate.t)) = ref([]);
      let dispose = Buffer.onUpdate(upd => updates := [upd, ...updates^]);

      Buffer.setLines(~lines=[|"abc"|], buffer);
      let lineCount = Buffer.getLineCount(buffer);
      let line0 = Buffer.getLine(buffer, Index.zero);
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

      let endPoint = Buffer.getLineCount(buffer) |> Index.fromZeroBased;

      Buffer.setLines(~start=endPoint, ~lines=[|"abc"|], buffer);
      let line3 = Buffer.getLine(buffer, Index.(zero + 2));
      let line4 = Buffer.getLine(buffer, Index.(zero + 3));
      let lineCount = Buffer.getLineCount(buffer);
      expect.int(lineCount).toBe(4);
      expect.string(line3).toEqual("This is the third line of a test file");
      expect.string(line4).toEqual("abc");
    });
  });
  describe("applyEdits", ({test, _}) => {

    let range = (startLine, startColumn, endLine, endColumn) => Range.{
      start: Location.create(
      ~line=Index.(zero + startLine),
      ~column=Index.(zero + startColumn)),
      stop: Location.create(
      ~line=Index.(zero+endLine),
      ~column=Index.(zero+endColumn)
      )
    };

    test("insert string inside line", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit = Edit.{
        range: range(0, 1, 0, 1),
        text: [|"a"|]
      };

      Buffer.applyEdits(~edits=[edit], buffer);

      let line = Buffer.getLine(buffer, Index.fromOneBased(1));
      expect.string(line).toEqual("Tahis is the first line of a test file");
      expect.int(Buffer.getLineCount(buffer)).toBe(3);
    });
    test("insert string, adding a line line", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit = Edit.{
        range: range(0, 1, 0, 1),
        text: [|"his is a whole new line", "T"|]
      };

      Buffer.applyEdits(~edits=[edit], buffer);

      let line = Buffer.getLine(buffer, Index.fromOneBased(1));
      expect.string(line).toEqual("This is a whole new line");
      let line = Buffer.getLine(buffer, Index.fromOneBased(2));
      expect.string(line).toEqual("This is the first line of a test file");
      expect.int(Buffer.getLineCount(buffer)).toBe(4);
    });
    test("delete line", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit = Edit.{
        range: range(0, 0, 1, 0),
        text: [||]
      };

      Buffer.applyEdits(~edits=[edit], buffer);

      let line = Buffer.getLine(buffer, Index.fromOneBased(1));
      expect.string(line).toEqual("This is the second line of a test file");

      expect.int(Buffer.getLineCount(buffer)).toBe(2);
    });
    test("delete multiple lines", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");

      let edit = Edit.{
        range: range(0, 0, 2, 0),
        text: [||]
      };

      Buffer.applyEdits(~edits=[edit], buffer);

      let line = Buffer.getLine(buffer, Index.fromOneBased(1));
      expect.string(line).toEqual("This is the third line of a test file");

      expect.int(Buffer.getLineCount(buffer)).toBe(1);
    });
  });
  describe("getLine", ({test, _}) =>
    test("single file", ({expect, _}) => {
      let _ = resetBuffer();
      let buffer = Buffer.openFile("test/reason-libvim/testfile.txt");
      let line = Buffer.getLine(buffer, Index.fromOneBased(1));
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
      input("<esc>");
      expect.int(List.length(modifyVals^)).toBe(1);

      // ..undo should, though
      input("u");
      expect.int(List.length(modifyVals^)).toBe(2);
      expect.bool(List.hd(modifyVals^)).toBe(false);

      // ..and redo
      input("<c-r>");
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
      let onEnter = ref([]);
      let dispose =
        Buffer.onFilenameChanged(meta => updates := [meta, ...updates^]);
      let dispose2 = Buffer.onEnter(v => onEnter := [v, ...onEnter^]);

      let _: Context.t = command("e! some-new-file.txt");

      /* A filename changed event should not have been triggered */
      expect.int(List.length(updates^)).toBe(0);
      /* An enter event should've been triggered */
      expect.int(List.length(onEnter^)).toBe(1);

      dispose();
      dispose2();
    });
    test(
      "saving to a new file should trigger an onFilenameChanged event",
      ({expect, _}) => {
      let _ = resetBuffer();

      let updates = ref([]);
      let onEnter = ref([]);
      let dispose =
        Buffer.onFilenameChanged(meta => updates := [meta, ...updates^]);
      let dispose2 = Buffer.onEnter(v => onEnter := [v, ...onEnter^]);

      let string_opt = s =>
        switch (s) {
        | None => ""
        | Some(v) => v
        };

      let previousFilename =
        Buffer.getCurrent() |> Buffer.getFilename |> string_opt;
      let _: Context.t = command("sav! some-new-file-2.txt");
      let newFilename =
        Buffer.getCurrent() |> Buffer.getFilename |> string_opt;

      expect.bool(String.equal(previousFilename, newFilename)).toBe(false);

      expect.int(List.length(updates^)).toBe(1);

      /* A buffer enter event should not have been triggered */
      expect.int(List.length(onEnter^)).toBe(0);

      dispose();
      dispose2();
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

      let _: Context.t = command("set filetype=derp");

      /* A filename changed event should not have been triggered */
      expect.int(List.length(updates^)).toBe(1);
      /* An enter event should've been triggered */
      expect.bool(List.hd(updates^) == Some("derp")).toBe(true);

      dispose();
    })
  });
  describe("onEnter", ({test, _}) => {
    test(
      "editing a new file should trigger a buffer enter event", ({expect, _}) => {
      let _ = resetBuffer();

      let updates: ref(list(Buffer.t)) = ref([]);
      let dispose = Buffer.onEnter(upd => updates := [upd, ...updates^]);

      let _ = Buffer.openFile("test/lines_100.txt");

      expect.int(List.length(updates^)).toBe(1);

      dispose();
    });

    test(
      "editing a new file via ':e' should trigger a buffer enter event",
      ({expect, _}) => {
      let _ = resetBuffer();

      let updates: ref(list(Buffer.t)) = ref([]);
      let dispose = Buffer.onEnter(upd => updates := [upd, ...updates^]);

      let _ = Buffer.openFile("test/reason-libvim/some_random_file.txt");

      expect.int(List.length(updates^)).toBe(1);
      dispose();
    });

    test(
      "jumping to a previous file via '<c-o>' should trigger buffer enter",
      ({expect, _}) => {
      let _ = resetBuffer();

      let buf1 = Buffer.openFile("test/reason-libvim/lines_100.txt");

      let buf2 = Buffer.openFile("test/reason-libvim/some_random_file.txt");

      let updates: ref(list(Buffer.t)) = ref([]);
      let dispose = Buffer.onEnter(upd => updates := [upd, ...updates^]);

      expect.bool(Buffer.getCurrent() == buf2).toBe(true);

      input("<c-o>");
      expect.bool(Buffer.getCurrent() == buf1).toBe(true);
      expect.int(List.length(updates^)).toBe(1);
      expect.bool(List.hd(updates^) == buf1).toBe(true);

      input("<c-i>");
      expect.bool(Buffer.getCurrent() == buf2).toBe(true);
      expect.int(List.length(updates^)).toBe(2);
      expect.bool(List.hd(updates^) == buf2).toBe(true);

      dispose();
    });
  });
  describe("onWrite", ({test, _}) => {
    test("saving the file should trigger an onWrite event", ({expect, _}) => {
      let _ = resetBuffer();

      let writes = ref([]);
      let dispose = Buffer.onWrite(id => writes := [id, ...writes^]);

      let _context: Context.t = command("w! some-new-file-again-write.txt");

      expect.int(List.length(writes^)).toBe(1);

      dispose();
    })
  });
});

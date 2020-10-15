open EditorCoreTypes;
open TestFramework;

open Vim;

let resetBuffer = () =>
  Helpers.resetBuffer("test/reason-libvim/lines_100.txt");

describe("MotionTest", ({describe, _}) => {
  describe("H / L / M", ({test, _}) => {
    test("uses context value", ({expect, _}) => {
      let _ = resetBuffer();

      let viewLineMotion = (~motion, ~count as _, ~startLine as _) => {
        switch (motion) {
        | ViewLineMotion.MotionH => 1 |> LineNumber.ofOneBased
        | ViewLineMotion.MotionM => 50 |> LineNumber.ofOneBased
        | ViewLineMotion.MotionL => 100 |> LineNumber.ofOneBased
        };
      };

      let context = {...Vim.Context.current(), viewLineMotion};

      let context = Vim.input(~context, "L");
      expect.equal(
        context.mode,
        Normal({
          cursor:
            BytePosition.{
              line: LineNumber.ofOneBased(100),
              byte: ByteIndex.zero,
            },
        }),
      );

      let context = Vim.input(~context={...context, viewLineMotion}, "M");
      expect.equal(
        context.mode,
        Normal({
          cursor:
            BytePosition.{
              line: LineNumber.ofOneBased(50),
              byte: ByteIndex.zero,
            },
        }),
      );

      let context: Vim.Context.t =
        Vim.input(~context={...context, viewLineMotion}, "H");
      expect.equal(
        context.mode,
        Normal({
          cursor:
            BytePosition.{
              line: LineNumber.ofOneBased(1),
              byte: ByteIndex.zero,
            },
        }),
      );
    });
    test("GC stress test", ({expect, _}) => {
      let _ = resetBuffer();

      for (_idx in 0 to 1000) {
        let viewLineMotion = (~motion, ~count as _, ~startLine as _) => {
          // Simulate large allocation
          let _str = String.make(1024 * 1024, 'a');
          let ret =
            switch (motion) {
            | ViewLineMotion.MotionH => 1 |> LineNumber.ofOneBased
            | ViewLineMotion.MotionM => 50 |> LineNumber.ofOneBased
            | ViewLineMotion.MotionL => 100 |> LineNumber.ofOneBased
            };
          // ...and compact.
          Gc.compact();
          ret;
        };
        let _context: Vim.Context.t =
          Vim.input(
            ~context={...Vim.Context.current(), viewLineMotion},
            "M",
          );
        ();
      };

      let latestContext = Vim.Context.current();
      expect.equal(
        latestContext.mode,
        Normal({
          cursor:
            BytePosition.{
              line: LineNumber.ofOneBased(50),
              byte: ByteIndex.zero,
            },
        }),
      );
    });
  });
  describe("gj / gk", ({test, _}) => {
    test("uses context value", ({expect, _}) => {
      let _ = resetBuffer();

      let screenCursorMotion =
          (~direction, ~count, ~line as lnum, ~currentByte, ~wantByte as _) => {
        let byteIdx = ByteIndex.toInt(currentByte);
        switch (direction) {
        | `Up =>
          BytePosition.{byte: ByteIndex.ofInt(byteIdx - count), line: lnum}
        | `Down => {byte: ByteIndex.ofInt(byteIdx + count), line: lnum}
        };
      };

      let context = {...Vim.Context.current(), screenCursorMotion};

      let context = Vim.input(~context, "5gj");
      expect.equal(
        context.mode,
        Normal({
          cursor:
            BytePosition.{
              line: LineNumber.ofOneBased(1),
              byte: ByteIndex.ofInt(5),
            },
        }),
      );

      let context = {...Vim.Context.current(), screenCursorMotion};
      let context = Vim.input(~context, "gk");
      expect.equal(
        context.mode,
        Normal({
          cursor:
            BytePosition.{
              line: LineNumber.ofOneBased(1),
              byte: ByteIndex.ofInt(4),
            },
        }),
      );
    });
    test("gc stress test", ({expect, _}) => {
      let _ = resetBuffer();

      let screenCursorMotion =
          (~direction, ~count, ~line as lnum, ~currentByte, ~wantByte as _) => {
        let _str = String.make(1024 * 1024, 'a');

        let count = count < 1 ? 1 : count;
        let byteIdx = ByteIndex.toInt(currentByte);
        let ret =
          switch (direction) {
          | `Up =>
            BytePosition.{byte: ByteIndex.ofInt(byteIdx - count), line: lnum}
          | `Down => {byte: ByteIndex.ofInt(byteIdx + count), line: lnum}
          };
        Gc.compact();
        ret;
      };

      for (_i in 0 to 1000) {
        let context = {...Vim.Context.current(), screenCursorMotion};
        let _context = Vim.input(~context, "gj");
        let context = {...Vim.Context.current(), screenCursorMotion};
        let _context = Vim.input(~context, "gk");
        ();
      };
      let context = {...Vim.Context.current(), screenCursorMotion};
      expect.equal(
        context.mode,
        Normal({
          cursor:
            BytePosition.{
              line: LineNumber.ofOneBased(1),
              byte: ByteIndex.ofInt(0),
            },
        }),
      );
    });
  });
});

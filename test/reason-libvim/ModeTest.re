open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");

describe("Mode", ({describe, _}) => {
  describe("replace mode", ({test, _}) => {
    test("replace mode is reported correctly", ({expect, _}) => {
      let _ = resetBuffer();

      // Enter replace mode
      let _ = Vim.input("R");

      expect.equal(Vim.Mode.current(), Vim.Mode.Replace);
    })
  });
  describe("select mode", ({test, _}) => {
    test("select mode is reported correctly", ({expect, _}) => {
      let _ = resetBuffer();

      // Enter replace mode
      let _ = Vim.input("V");
      expect.equal(Vim.Mode.isVisual(Vim.Mode.current()), true);

      let _ = Vim.key("<C-g>");

      expect.equal(Vim.Mode.isSelect(Vim.Mode.current()), true);
    })
  });

  describe("operator pending mode mode", ({test, _}) => {
    test("d", ({expect, _}) => {
      let _ = resetBuffer();

      // delete pending op
      let _ = Vim.input("d");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          pending: Vim.Operator.{operation: Delete, count: 0, register: 0},
        }),
      );
    });
    test("5d", ({expect, _}) => {
      let _ = resetBuffer();

      // delete pending op, with multiplier
      let _ = Vim.input("5d");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          pending: Vim.Operator.{operation: Delete, count: 5, register: 0},
        }),
      );
    });
    test("10gc", ({expect, _}) => {
      let _ = resetBuffer();

      // Comment operator
      let _ = Vim.input("10gc");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          pending: Vim.Operator.{operation: Comment, count: 10, register: 0},
        }),
      );
    });
    test("yank with register", ({expect, _}) => {
      let _ = resetBuffer();

      // Yank to register a
      let _ = Vim.input("\"ay");

      let mode = Vim.Mode.current();
      expect.equal(
        mode,
        Vim.Mode.Operator({
          pending:
            Vim.Operator.{
              operation: Yank,
              count: 0,
              register: int_of_char('a'),
            },
        }),
      );
    });
  });
});

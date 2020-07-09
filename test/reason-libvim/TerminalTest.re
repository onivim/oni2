open TestFramework;

let resetBuffer = () => Helpers.resetBuffer("test/testfile.txt");

describe("Terminal", ({test, _}) => {
  test("Empty command works", ({expect, _}) => {
    let _ = resetBuffer();

    let hitCount = ref(0);
    let unsubscribe =
      Vim.onTerminal(terminalRequest => {
        expect.equal(terminalRequest.cmd, None);
        expect.equal(terminalRequest.curwin, false);
        expect.equal(terminalRequest.closeOnFinish, true);
        incr(hitCount);
      });

    // Enter term command
    let _ = Vim.input(":term");
    let _ = Vim.input("<CR>");

    expect.equal(hitCount^, 1);

    unsubscribe();
  });
  test("Command is reported correctly", ({expect, _}) => {
    let _ = resetBuffer();

    let hitCount = ref(0);
    let unsubscribe =
      Vim.onTerminal(terminalRequest => {
        expect.equal(terminalRequest.cmd, Some("bash"));
        expect.equal(terminalRequest.curwin, false);
        expect.equal(terminalRequest.closeOnFinish, true);
        incr(hitCount);
      });

    // Enter term command
    let _ = Vim.input(":term bash");
    let _ = Vim.input("<CR>");

    expect.equal(hitCount^, 1);

    unsubscribe();
  });

  test("Curwin flag is set", ({expect, _}) => {
    let _ = resetBuffer();

    let hitCount = ref(0);
    let unsubscribe =
      Vim.onTerminal(terminalRequest => {
        expect.equal(terminalRequest.cmd, Some("bash"));
        expect.equal(terminalRequest.curwin, true);
        expect.equal(terminalRequest.closeOnFinish, true);
        incr(hitCount);
      });

    // Enter term command
    let _ = Vim.input(":term ++curwin bash");
    let _ = Vim.input("<CR>");

    expect.equal(hitCount^, 1);

    unsubscribe();
  });
  test("No close flag is set", ({expect, _}) => {
    let _ = resetBuffer();

    let hitCount = ref(0);
    let unsubscribe =
      Vim.onTerminal(terminalRequest => {
        expect.equal(terminalRequest.cmd, Some("bash"));
        expect.equal(terminalRequest.curwin, false);
        expect.equal(terminalRequest.closeOnFinish, false);
        incr(hitCount);
      });

    // Enter term command
    let _ = Vim.input(":term ++noclose bash");
    let _ = Vim.input("<CR>");

    expect.equal(hitCount^, 1);

    unsubscribe();
  });
  test("GC stress test", ({expect, _}) => {
    let _ = resetBuffer();

    let hitCount = ref(0);
    let unsubscribe =
      Vim.onTerminal(terminalRequest => {
        Gc.full_major();
        expect.equal(terminalRequest.cmd, Some("bash"));
        incr(hitCount);
      });

    // Enter term command
    let iter = () => {
      let _ = Vim.input(":term ++curwin bash");
      let _ = Vim.input("<CR>");
      ();
    };

    for (_ in 1 to 1000) {
      iter();
    };

    expect.equal(hitCount^, 1000);

    unsubscribe();
  });
});

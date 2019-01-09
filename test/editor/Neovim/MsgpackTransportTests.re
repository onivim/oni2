
/* open Rench; */

/* open Oni_Neovim; */
open TestFramework;

describe("MsgpackTransport", ({test, _}) =>
  test("writing a simple message dispatches message event", ({expect}) => {
      expect.int(0).toBe(1);
  })
);

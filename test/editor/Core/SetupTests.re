open Oni_Core;
open TestFramework;

describe("Setup", ({test, _}) => {
  test("ofString", ({expect}) => {

    let setupInfo = "{neovim:\"/path/to/neovim\",node:\"/path/to/node\"}";
    let setup = Setup.ofString(setupInfo)
    expect.string(setup.neovimPath).toEqual("/path/to/neovim");
    expect.string(setup.nodePath).toEqual("/path/to/node");
  });
});

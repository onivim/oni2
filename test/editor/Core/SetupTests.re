open Oni_Core;
open TestFramework;

describe("Setup", ({test, _}) =>
  test("ofString", ({expect}) => {
    let setupInfo = "{neovim:\"/path/to/neovim\",node:\"/path/to/node\",textmateService:\"/path/to/textmate\",bundledExtensions:\"/path/to/extensions\",configuration:\"/path/to/config\",keybindings:\"/path/to/keybindings\"}";
    let setup = Setup.ofString(setupInfo);
    expect.string(setup.neovimPath).toEqual("/path/to/neovim");
    expect.string(setup.nodePath).toEqual("/path/to/node");
    expect.string(setup.textmateServicePath).toEqual("/path/to/textmate");
    expect.string(setup.bundledExtensionsPath).toEqual(
      "/path/to/extensions",
    );
    expect.string(setup.configPath).toEqual("/path/to/config");
    expect.string(setup.keybindingsPath).toEqual("/path/to/keybindings");
  })
);

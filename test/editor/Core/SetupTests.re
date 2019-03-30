open Oni_Core;
open TestFramework;

describe("Setup", ({test, _}) =>
  test("ofString", ({expect}) => {
    let setupInfo = "{neovim:\"/path/to/neovim\",node:\"/path/to/node\",textmateService:\"/path/to/textmate\",bundledExtensions:\"/path/to/extensions\",configuration:\"/path/to/config\",keybindings:\"/path/to/keybindings\",rg:\"/path/to/rg\", extensionHost: \"/path/to/exthost\"}";
    let setup = Setup.ofString(setupInfo);
    expect.string(setup.neovimPath).toEqual("/path/to/neovim");
    expect.string(setup.nodePath).toEqual("/path/to/node");
    expect.string(setup.textmateServicePath).toEqual("/path/to/textmate");
    expect.string(setup.bundledExtensionsPath).toEqual(
      "/path/to/extensions",
    );
    expect.string(setup.extensionHostPath).toEqual("/path/to/exthost");
    expect.string(setup.configPath).toEqual("/path/to/config");
    expect.string(setup.keybindingsPath).toEqual("/path/to/keybindings");
    expect.string(setup.rgPath).toEqual("/path/to/rg");
  })
);

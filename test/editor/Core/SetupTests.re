open Oni_Core;
open TestFramework;

describe("Setup", ({test, _}) =>
  test("ofString", ({expect}) => {
    let setupInfo = "{node:\"/path/to/node\",textmateService:\"/path/to/textmate\",bundledExtensions:\"/path/to/extensions\",rg:\"/path/to/rg\", extensionHost: \"/path/to/exthost\"}";
    let setup = Setup.ofString(setupInfo);
    expect.string(setup.neovimPath).toEqual("/path/to/neovim");
    expect.string(setup.textmateServicePath).toEqual("/path/to/textmate");
    expect.string(setup.bundledExtensionsPath).toEqual(
      "/path/to/extensions",
    );
    expect.string(setup.extensionHostPath).toEqual("/path/to/exthost");
    expect.string(setup.rgPath).toEqual("/path/to/rg");
  })
);

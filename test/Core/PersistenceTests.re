open Oni_Core;
open TestFramework;

module Store = Persistence.Store;
open Persistence.Schema;

type testContext = {
  storeFolder: string,
  store: Store.t(bool),
  testBool: item(bool, bool),
};

describe("Persistence", ({test, _}) => {
  let setup = () => {
    let tempDir = Filename.get_temp_dir_name();
    prerr_endline("Persistence.setup - creating temp folder: " ++ tempDir);
    let _: result(unit, Luv.Error.t) = tempDir |> Luv.File.Sync.mkdir;

    let storeFolderTemplate = tempDir ++ "store-testXXXXXX";
    prerr_endline(
      "Persistence.setup - creating storeFolderTemplate: "
      ++ storeFolderTemplate,
    );
    let storeFolder =
      storeFolderTemplate |> Luv.File.Sync.mkdtemp |> Result.get_ok;
    prerr_endline("Persistence.setup - created storeFolder: " ++ storeFolder);

    let instantiate = Store.instantiate(~storeFolder);

    let testBool = define("testBool", bool, false, state => state);

    let store = instantiate("global", () => [Store.entry(testBool)]);

    {storeFolder, store, testBool};
  };

  test("get defaults", ({expect, _}) => {
    let {store, testBool, _} = setup();
    expect.bool(Store.get(testBool, store)).toBe(false);
  });

  test("empty file (regression test for #1766)", ({expect, _}) => {
    let {storeFolder, testBool, _} = setup();

    // We'll write out an empty file...
    let oc = open_out(storeFolder ++ "/global/store.json");
    Printf.fprintf(oc, "\n");
    close_out(oc);

    // ...and hydrate a new store to exercise #1766
    let store =
      Store.instantiate(~storeFolder, "global", () =>
        [Store.entry(testBool)]
      );

    expect.bool(Store.get(testBool, store)).toBe(false);
  });
});

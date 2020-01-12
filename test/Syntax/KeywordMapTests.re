open TestFramework;

module KeywordMap = Oni_Syntax.KeywordMap;

describe("KeywordMap", ({test, _}) => {
  test("simple case", ({expect, _}) => {
    let keywords =
      KeywordMap.empty
      |> KeywordMap.set(
           ~bufferId=1,
           ~scope="reason",
           ~line=1,
           ~words=["a", "b", "c"],
         )
      |> KeywordMap.get(~scope="reason");

    expect.equal(["a", "b", "c"], keywords);
  });
  test("overwrite line", ({expect, _}) => {
    let keywords =
      KeywordMap.empty
      |> KeywordMap.set(
           ~bufferId=1,
           ~scope="reason",
           ~line=1,
           ~words=["a", "b", "c"],
         )
      |> KeywordMap.set(
           ~bufferId=1,
           ~scope="reason",
           ~line=1,
           ~words=["a", "b", "d"],
         )
      |> KeywordMap.get(~scope="reason");

    expect.equal(["a", "b", "d"], keywords);
  });
  test("mutiple buffers", ({expect, _}) => {
    let keywords =
      KeywordMap.empty
      |> KeywordMap.set(
           ~bufferId=1,
           ~scope="reason",
           ~line=1,
           ~words=["a", "b", "c"],
         )
      |> KeywordMap.set(
           ~bufferId=2,
           ~scope="reason",
           ~line=1,
           ~words=["d", "e", "f"],
         )
      |> KeywordMap.get(~scope="reason");

    expect.equal(["a", "b", "c", "d", "e", "f"], keywords);
  });
  test("mutiple buffers, ovewrite line", ({expect, _}) => {
    let keywords =
      KeywordMap.empty
      |> KeywordMap.set(
           ~bufferId=1,
           ~scope="reason",
           ~line=1,
           ~words=["a", "b", "c"],
         )
      |> KeywordMap.set(
           ~bufferId=2,
           ~scope="reason",
           ~line=1,
           ~words=["d", "e", "f"],
         )
      |> KeywordMap.set(
           ~bufferId=2,
           ~scope="reason",
           ~line=1,
           ~words=["d", "g", "f"],
         )
      |> KeywordMap.get(~scope="reason");

    expect.equal(["a", "b", "c", "d", "f", "g"], keywords);
  });
});

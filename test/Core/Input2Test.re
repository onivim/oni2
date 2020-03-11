open TestFramework;
open Oni_Core;
open Oni_Core.Input2;
describe("Input 2", ({describe, _}) => {
  describe("key matching", ({test, _}) => {
    test("matches keycode", ({expect}) => {
		let (bindings, _id) =
		   Input2.empty
		   |> Input2.addBinding(
		   	Keycode(0, Modifiers.none),
				(_) => true,
				"payload1");

		let (_bindings, effects) = 
			Input2.keyDown({
				scancode: -1,
				keycode: 1,
				modifiers: Modifiers.none,
				text: "a"
			}, bindings);

		expect.equal(effects, [Execute("payload1")]);
    });
  })
});

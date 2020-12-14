
type model = unit;

let initial = ();

module View = {
	open Revery;
	open Revery.UI;
	open Revery.UI.Components;


	let make = (
		~theme as _,
		~config as _,
		~model as _,
		()
	) => {
		<Text text="Hello, world!" />
	}
}

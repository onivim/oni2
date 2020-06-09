let forceReload: unit => Isolinear.Effect.t(_);
let forceOverwrite: unit => Isolinear.Effect.t(_);
let reload: unit => Isolinear.Effect.t(_);
let saveAllAndQuit: unit => Isolinear.Effect.t(_);
let quitAll: unit => Isolinear.Effect.t(_);

module Effects: {
	let applyEdits: (~bufferId: int, ~version: int, 
	~edits: list(Oni_Core.SingleEdit.t), 
	result(unit, string) => 'msg) => Isolinear.Effect.t('msg);
};

open Oni_Core;
open Oni_Core.Utility;
open Oni_Extensions;
open Timber;

module Log = (val Log.withNamespace("SyntaxServerExceptionTest"));
Timber.App.enable();
Timber.App.setLevel(Timber.Level.debug);

let wait = (~name="TODO",f) => {
	ThreadEx.waitForCondition(f);

	if (!f()) {
		let msg = "Failed: " ++ name;
		Log.error(msg);
		failwith(msg);
	}
};

let connected = ref(false);
let onConnected = () => {
	Log.info("Connected to syntax server");
	connected := true;
};

Oni_Syntax_Client.start(
	~executablePath=Revery.Environment.executingDirectory ++ "SyntaxServer.exe",
	~onConnected,
	~onClose=(code) => Log.infof(m => m("Syntax server closed with code: %d", code)),
	~onHighlights=_ => (),
	~onHealthCheckResult=_ => (),
	~scheduler=Scheduler.immediate,
	LanguageInfo.initial,
	Setup.default()
);

wait(~name="Connected", () => connected^)

prerr_endline ("Done!");

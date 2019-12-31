module Log = (val Oni_Core.Log.withNamespace("Oni2.ShellUtility"));

module Internal = {

	let getDefaultShell = () => {
		switch(Sys.getenv_opt("SHELL")) {
		| Some(v) => v
		| None => Sys.win32 ? "cmd.exe" : "/usr/bin/bash"
		}
	};

	let getShellPath = Lazy.from_fun(() => {
		let res = if (Sys.win32) {
			Sys.getenv("PATH");
		} else {
			let shell = getDefaultShell();
			let shellCmd = Printf.sprintf("%s -lc 'echo $PATH'", shell);
			let ic = Unix.open_process_in(shellCmd);
			let path = input_line(ic);
			let () = close_in(ic);
			path;
		}

		Log.info("Path is: " ++ res);
		res;
	});
}

let getShellPath = Lazy.force(Internal.getShellPath);

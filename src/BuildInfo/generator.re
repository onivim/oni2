let getCommitId = () => {
	let ic = Unix.open_process_in("git rev-parse --short HEAD");
	let commitId = input_line(ic);
	let () = close_in(ic);
	commitId;
};

let getVersion = () => {
	Yojson.Safe.from_file("../../package.json")
	|> Yojson.Safe.Util.member("version")
	|> Yojson.Safe.Util.to_string;
};
	
let oc = open_out("foo.re");

Printf.fprintf(oc, 
{|
let hello = () => prerr_endline("hello2!");
let commitId = "%s";
let version = "%s";
|}, 
getCommitId(),
getVersion(),
);
close_out(oc);



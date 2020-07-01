open Oni_Core;
module Request : {
	let json: (
		~decoder: Json.decoder('a),
		string
	) => Lwt.t(result('a, string));
}

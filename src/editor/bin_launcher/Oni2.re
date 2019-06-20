/*
 * Oni2.re
 *
 * This is the launcher for the editor.
 */


let launch = () => {

    let (pstdin, stdin) = Unix.pipe();
    let (stdout, pstdout) = Unix.pipe();
    let (stderr, pstderr) = Unix.pipe();

    Unix.set_close_on_exec(pstdin);
    Unix.set_close_on_exec(stdin);
    Unix.set_close_on_exec(pstdout);
    Unix.set_close_on_exec(stdout);
    Unix.set_close_on_exec(pstderr);
    Unix.set_close_on_exec(stderr);


    let executingDirectory = Filename.dirname(Sys.argv[0]);
    Unix.create_process(executingDirectory ++ "/" ++ "Oni2_editor.exe", Sys.argv, pstdin, pstdout, pstderr);
    Unix.close(pstdout);
    Unix.close(pstdin);
    Unix.close(pstderr);

    print_endline ("Hello, world:" ++ executingDirectory);
};


launch();

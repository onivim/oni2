Printexc.record_backtrace(true);

prerr_endline ("STARTING")
Printexc.set_uncaught_exception_handler((exn, bt) => prerr_endline(Printexc.to_string(exn)));
Oni_Model_Test.TestFramework.cli();

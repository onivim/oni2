open Oni_Core.Utility;
open TestFramework;

open Luv.File;
open Service_OS.Api;

let bind = (fst, snd) => Lwt.bind(snd, fst);

describe("Service.OS.Api", ({describe, _}) => {
  describe("mkdir", ({test, _}) => {
    test("creates a directory successfully", ({expect, _}) => {
      let test =
        mktempdir()
        |> bind(tempDir => {
             let nestedDir = Rench.Path.join(tempDir, "another-dir");
             mkdir(nestedDir) |> Lwt.map(_ => nestedDir);
           })
        |> bind(stat)
        |> Lwt.map((statResult: Luv.File.Stat.t) => {
             expect.equal(Mode.test([`IFDIR], statResult.mode), true);
             ();
           })
        |> LwtEx.sync;

      expect.result(test).toBeOk();
    })
  });
  describe("rmdir", ({test, _}) => {
    test("removes a directory successfully", ({expect, _}) => {
      let test =
        mktempdir()
        |> bind(path => rmdir(path) |> Lwt.map(_ => path))
        |> bind(stat)
        |> LwtEx.sync;

      expect.result(test).toBeError();
    })
  });
  describe("stat", ({test, _}) => {
    test("stats a file correctly", ({expect, _}) => {
      let test =
        mktempdir()
        |> bind(tempPath => {
             let filePath = Rench.Path.join(tempPath, "testfile.txt");
             File.write(~contents="Hello, world", filePath);
             Lwt.return(filePath);
           })
        |> bind(path => {stat(path)})
        |> Lwt.map((statResult: Luv.File.Stat.t) => {
             expect.equal(Mode.test([`IFREG], statResult.mode), true);
             ();
           })
        |> LwtEx.sync;

      expect.result(test).toBeOk();
    })
  });
  describe("writeFile", ({test, _}) => {
    test("basic file write", ({expect, _}) => {
      let outPath =
        mktempdir()
        |> bind(tempPath => {
             let filePath = Rench.Path.join(tempPath, "testfile.txt");
             writeFile(~contents=Bytes.of_string("Hello, world!"), filePath)
             |> Lwt.map(_ => filePath);
           })
        |> LwtEx.sync
        |> Result.get_ok;

      expect.equal(File.readAllLines(outPath), ["Hello, world!"]);
    });
    test("bigger file write", ({expect, _}) => {
      let contents = String.make(128000, 'a');
      let outPath =
        mktempdir()
        |> bind(tempPath => {
             let filePath = Rench.Path.join(tempPath, "testfile.txt");
             writeFile(~contents=Bytes.of_string(contents), filePath)
             |> Lwt.map(_ => filePath);
           })
        |> LwtEx.sync
        |> Result.get_ok;

      expect.equal(File.readAllLines(outPath), [contents]);
    });
  });
  describe("readFile", ({test, _}) => {
    test("basic file read", ({expect, _}) => {
      let str =
        mktempdir()
        |> bind(tempPath => {
             let filePath = Rench.Path.join(tempPath, "testfile.txt");
             File.write(~contents="Hello, world!", filePath);
             Lwt.return(filePath);
           })
        |> bind(readFile)
        |> LwtEx.sync
        |> Result.get_ok
        |> Bytes.to_string;

      expect.equal(str, "Hello, world!");
    });
    test("basic file read, chunksize 1", ({expect, _}) => {
      let str =
        mktempdir()
        |> bind(tempPath => {
             let filePath = Rench.Path.join(tempPath, "testfile.txt");
             File.write(~contents="Hello, world!", filePath);
             Lwt.return(filePath);
           })
        |> bind(readFile(~chunkSize=1))
        |> LwtEx.sync
        |> Result.get_ok
        |> Bytes.to_string;

      expect.equal(str, "Hello, world!");
    });
  });
  describe("copy file", ({test, _}) => {
    test("copy simple file", ({expect, _}) => {
      let copiedFilePath =
        mktempdir()
        |> bind(tempPath => {
             let filePath = Rench.Path.join(tempPath, "testfile.txt");
             File.write(~contents="Hello, world!", filePath);
             Lwt.return((tempPath, filePath));
           })
        |> bind(((tempPath, filePath)) => {
             let copiedFilePath = Rench.Path.join(tempPath, "copied.txt");
             copy(~overwrite=true, ~source=filePath, ~target=copiedFilePath)
             |> Lwt.map(_ => copiedFilePath);
           })
        |> LwtEx.sync
        |> Result.get_ok;

      expect.equal(File.readAllLines(copiedFilePath), ["Hello, world!"]);
    })
  });
});

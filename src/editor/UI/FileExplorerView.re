open Revery_UI;
open Revery.UI.Components;

let component = React.component("FileExplorerView");

let dummyFiles =
  Tree.(
    Node(
      {data: "root", id: 1, status: Open},
      [
        Node(
          {data: "subfolder 1", id: 2, status: Open},
          [
            Node(
              {data: "subdirectory 1", id: 3, status: Closed},
              [Empty, Empty],
            ),
          ],
        ),
        Node(
          {data: "home", id: 4, status: Open},
          [
            Node({status: Closed, id: 5, data: "downloads"}, [Empty, Empty]),
            Node(
              {data: "desktop", id: 6, status: Open},
              [
                Node(
                  {status: Open, id: 7, data: "subfolder 2"},
                  [
                    Node(
                      {status: Open, id: 8, data: "pictures"},
                      [
                        Node({status: Closed, id: 12, data: "Images"}, []),
                        Node(
                          {status: Closed, id: 10, data: "holiday 2018"},
                          [],
                        ),
                        Node(
                          {status: Closed, id: 11, data: "Graduation 2017"},
                          [],
                        ),
                      ],
                    ),
                    Empty,
                  ],
                ),
                Node(
                  {data: "subfolder 3", id: 9, status: Closed},
                  [Empty, Empty],
                ),
              ],
            ),
          ],
        ),
      ],
    )
  );

let createElement = (~children, ~state, ()) =>
  component(hooks => {
    let (dir, setDir, hooks) = React.Hooks.state(None, hooks);
    let hooks =
      React.Hooks.effect(
        OnMount,
        () => {
          let promise = {
            let cwd = Rench.Environment.getWorkingDirectory();
            Lwt_unix.files_of_directory(cwd)
            |> Lwt_stream.map(Filename.concat(cwd))
            |> Lwt_stream.to_list;
          };

          let dir = Lwt_main.run(promise);
          setDir(Some(dir));
          None;
        },
        hooks,
      );
    (hooks, <TreeView tree=dummyFiles title="File Explorer" state />);
  });

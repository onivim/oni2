/*
 * FlatList.re
 *
 * Virtualized list helper
 */

/* open Oni_Core; */
open Revery.UI;

type renderFunction('a) = ('a) => React.syntheticElement;

let component = React.component("FlatList");

let createElement = (
    ~height as height_,
    ~width as width_,
    ~rowHeight: int,
    ~render: renderFunction('a),
    ~data: array('a),
    (),
) => component((hooks) => {
   

    let rowsToRender = rowHeight / height_;

    let i = ref(0);

    let items: ref(list(React.syntheticElement)) = ref([]);

    while(i^ < rowsToRender) {

        let item = data[i^];
        let v = render(item);

        items := List.append([v], items^);

        i := i^ + 1;
    }

    items := List.rev(items^);

    let style = Style.[
        position(`Absolute),
        top(0),
        left(0),
        width(width_),
        height(height_),
    ];

    (hooks, <View style>
        ...(items^)
    </View>)
});

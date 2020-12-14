// open Kernel;

// module Schema = {
// 	type item = {
// 		title: string,
// 		command: string,
// 		parentId: string,
// 		uniqueId: string,
// 	};

// 	type menu = {
// 		uniqueId: string,
// 		title: string,
// 	};

// 	let item = (
// 		~title,
// 		~command,
// 		~parent,
// 	) => {
// 		title,
// 		command,
// 		parentId: parent.uniqueId,
// 		uniqueId: parent.uniqueId ++ "." ++ title,
// 	};

// 	type t = {
// 		menus: StringMap.t(menu),
// 		items: list(item),
// 	};

// 	let toSchema = (~menus, ~items) => {
// 		let menuMap = menus
// 		|> List.fold_left((acc, curr: menu) => {
// 			acc
// 			|> StringMap.add(curr.uniqueId, curr)
// 		},
// 		StringMap.empty);

// 		{
// 			menus: menuMap,
// 			items
// 		};
// 	}

// 	let union = (map1, map2) => {
// 		let items' = map1.items @ map2.items;

// 		let menus' = StringMap.merge((key, maybeA, maybeB) => {
// 			switch ((maybeA, maybeB)) {
// 			| (Some(_) as a, _) => a
// 			| (None, Some(_) as b) => b
// 			| (None, None) => None
// 			}
// 		}, map1.menus, map2.menus);

// 		{
// 			menus: menus',
// 			items: items'
// 		}
// 	}
// };

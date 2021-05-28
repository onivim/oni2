"use strict";
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
Object.defineProperty(exports, "__esModule", { value: true });
require("mocha");
const assert = require("assert");
const vscode_1 = require("vscode");
const testUtils_1 = require("./testUtils");
const toggleComment_1 = require("../toggleComment");
function toggleComment() {
    const result = toggleComment_1.toggleComment();
    assert.ok(result);
    return result;
}
suite('Tests for Toggle Comment action from Emmet (HTML)', () => {
    teardown(testUtils_1.closeAllEditors);
    const contents = `
	<div class="hello">
		<ul>
			<li><span>Hello</span></li>
			<li><span>There</span></li>
			<div><li><span>Bye</span></li></div>
		</ul>
		<ul>
			<!--<li>Previously Commented Node</li>-->
			<li>Another Node</li>
		</ul>
		<span/>
		<style>
			.boo {
				margin: 10px;
				padding: 20px;
			}
			.hoo {
				margin: 10px;
				padding: 20px;
			}
		</style>
	</div>
	`;
    test('toggle comment with multiple cursors, but no selection (HTML)', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><!--<span>Hello</span>--></li>
			<!--<li><span>There</span></li>-->
			<!--<div><li><span>Bye</span></li></div>-->
		</ul>
		<!--<ul>
			<li>Previously Commented Node</li>
			<li>Another Node</li>
		</ul>-->
		<span/>
		<style>
			.boo {
				/*margin: 10px;*/
				padding: 20px;
			}
			/*.hoo {
				margin: 10px;
				padding: 20px;
			}*/
		</style>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 17, 3, 17),
                new vscode_1.Selection(4, 5, 4, 5),
                new vscode_1.Selection(5, 35, 5, 35),
                new vscode_1.Selection(7, 3, 7, 3),
                new vscode_1.Selection(14, 8, 14, 8),
                new vscode_1.Selection(18, 3, 18, 3) // cursor inside the css rule inside the style tag
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('toggle comment with multiple cursors and whole node selected (HTML)', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><!--<span>Hello</span>--></li>
			<!--<li><span>There</span></li>-->
			<div><li><span>Bye</span></li></div>
		</ul>
		<!--<ul>
			<li>Previously Commented Node</li>
			<li>Another Node</li>
		</ul>-->
		<span/>
		<style>
			.boo {
				/*margin: 10px;*/
				padding: 20px;
			}
			/*.hoo {
				margin: 10px;
				padding: 20px;
			}*/
		</style>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 7, 3, 25),
                new vscode_1.Selection(4, 3, 4, 30),
                new vscode_1.Selection(7, 2, 10, 7),
                new vscode_1.Selection(14, 4, 14, 17),
                new vscode_1.Selection(17, 3, 20, 4) // the css rule inside the style tag
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('toggle comment when multiple nodes are completely under single selection (HTML)', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<!--<li><span>Hello</span></li>
			<li><span>There</span></li>-->
			<div><li><span>Bye</span></li></div>
		</ul>
		<ul>
			<!--<li>Previously Commented Node</li>-->
			<li>Another Node</li>
		</ul>
		<span/>
		<style>
			.boo {
				/*margin: 10px;
				padding: 20px;*/
			}
			.hoo {
				margin: 10px;
				padding: 20px;
			}
		</style>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 4, 4, 30),
                new vscode_1.Selection(14, 4, 15, 18) // 2 css properties inside the style tag
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('toggle comment when multiple nodes are partially under single selection (HTML)', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<!--<li><span>Hello</span></li>
			<li><span>There</span></li>-->
			<div><li><span>Bye</span></li></div>
		</ul>
		<!--<ul>
			<li>Previously Commented Node</li>
			<li>Another Node</li>
		</ul>-->
		<span/>
		<style>
			.boo {
				margin: 10px;
				padding: 20px;
			}
			.hoo {
				margin: 10px;
				padding: 20px;
			}
		</style>
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 24, 4, 20),
                new vscode_1.Selection(7, 2, 9, 10) // The <ul> one of whose children is already commented
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('toggle comment with multiple cursors selecting parent and child nodes', () => {
        const expectedContents = `
	<div class="hello">
		<ul>
			<li><!--<span>Hello</span>--></li>
			<!--<li><span>There</span></li>-->
			<div><li><span>Bye</span></li></div>
		</ul>
		<!--<ul>
			<li>Previously Commented Node</li>
			<li>Another Node</li>
		</ul>-->
		<span/>
		<!--<style>
			.boo {
				margin: 10px;
				padding: 20px;
			}
			.hoo {
				margin: 10px;
				padding: 20px;
			}
		</style>-->
	</div>
	`;
        return testUtils_1.withRandomFileEditor(contents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 17, 3, 17),
                new vscode_1.Selection(4, 5, 4, 5),
                new vscode_1.Selection(4, 17, 4, 17),
                new vscode_1.Selection(7, 3, 7, 3),
                new vscode_1.Selection(9, 10, 9, 10),
                new vscode_1.Selection(12, 3, 12, 3),
                new vscode_1.Selection(14, 8, 14, 8),
                new vscode_1.Selection(18, 3, 18, 3),
                new vscode_1.Selection(19, 8, 19, 8) // 		and the fourth inside the css property inside the style tag
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
    test('toggle comment within script template', () => {
        const templateContents = `
	<script type="text/template">
		<li><span>Hello</span></li>
		<li><!--<span>There</span>--></li>
		<div><li><span>Bye</span></li></div>
		<span/>
	</script>
	`;
        const expectedContents = `
	<script type="text/template">
		<!--<li><span>Hello</span></li>-->
		<li><span>There</span></li>
		<div><li><!--<span>Bye</span>--></li></div>
		<span/>
	</script>
	`;
        return testUtils_1.withRandomFileEditor(templateContents, 'html', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 2, 2, 28),
                new vscode_1.Selection(3, 17, 3, 17),
                new vscode_1.Selection(4, 18, 4, 18),
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return Promise.resolve();
            });
        });
    });
});
suite('Tests for Toggle Comment action from Emmet (CSS)', () => {
    teardown(testUtils_1.closeAllEditors);
    const contents = `
	.one {
		margin: 10px;
		padding: 10px;
	}
	.two {
		height: 42px;
		display: none;
	}
	.three {
		width: 42px;
	}`;
    test('toggle comment with multiple cursors, but no selection (CSS)', () => {
        const expectedContents = `
	.one {
		/*margin: 10px;*/
		padding: 10px;
	}
	/*.two {
		height: 42px;
		display: none;
	}*/
	.three {
		width: 42px;
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 5, 2, 5),
                new vscode_1.Selection(5, 4, 5, 4),
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment with multiple cursors and whole node selected (CSS)', () => {
        const expectedContents = `
	.one {
		/*margin: 10px;*/
		/*padding: 10px;*/
	}
	/*.two {
		height: 42px;
		display: none;
	}*/
	.three {
		width: 42px;
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 2, 2, 15),
                new vscode_1.Selection(3, 0, 3, 16),
                new vscode_1.Selection(5, 1, 8, 2),
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                //return toggleComment().then(() => {
                //assert.equal(doc.getText(), contents);
                return Promise.resolve();
                //});
            });
        });
    });
    test('toggle comment when multiple nodes of same parent are completely under single selection (CSS)', () => {
        const expectedContents = `
	.one {
/*		margin: 10px;
		padding: 10px;*/
	}
	/*.two {
		height: 42px;
		display: none;
	}
	.three {
		width: 42px;
	}*/`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 0, 3, 16),
                new vscode_1.Selection(5, 1, 11, 2),
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment when start and end of selection is inside properties of separate rules (CSS)', () => {
        const expectedContents = `
	.one {
		margin: 10px;
		/*padding: 10px;
	}
	.two {
		height: 42px;*/
		display: none;
	}
	.three {
		width: 42px;
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 7, 6, 6)
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment when selection spans properties of separate rules, with start in whitespace and end inside the property (CSS)', () => {
        const expectedContents = `
	.one {
		margin: 10px;
		/*padding: 10px;
	}
	.two {
		height: 42px;*/
		display: none;
	}
	.three {
		width: 42px;
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 0, 6, 6)
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment when selection spans properties of separate rules, with end in whitespace and start inside the property (CSS)', () => {
        const expectedContents = `
	.one {
		margin: 10px;
		/*padding: 10px;
	}
	.two {
		height: 42px;*/
		display: none;
	}
	.three {
		width: 42px;
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 7, 7, 0)
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment when selection spans properties of separate rules, with both start and end in whitespace (CSS)', () => {
        const expectedContents = `
	.one {
		margin: 10px;
		/*padding: 10px;
	}
	.two {
		height: 42px;*/
		display: none;
	}
	.three {
		width: 42px;
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(3, 0, 7, 0)
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment when multiple nodes of same parent are partially under single selection (CSS)', () => {
        const expectedContents = `
	.one {
		/*margin: 10px;
		padding: 10px;*/
	}
	/*.two {
		height: 42px;
		display: none;
	}
	.three {
		width: 42px;
*/	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 7, 3, 10),
                new vscode_1.Selection(5, 2, 11, 0),
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
});
suite('Tests for Toggle Comment action from Emmet in nested css (SCSS)', () => {
    teardown(testUtils_1.closeAllEditors);
    const contents = `
	.one {
		height: 42px;

		.two {
			width: 42px;
		}

		.three {
			padding: 10px;
		}
	}`;
    test('toggle comment with multiple cursors selecting nested nodes (SCSS)', () => {
        const expectedContents = `
	.one {
		/*height: 42px;*/

		/*.two {
			width: 42px;
		}*/

		.three {
			/*padding: 10px;*/
		}
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 5, 2, 5),
                new vscode_1.Selection(4, 4, 4, 4),
                new vscode_1.Selection(5, 5, 5, 5),
                new vscode_1.Selection(9, 5, 9, 5) // cursor inside a property inside a nested rule
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment with multiple cursors selecting several nested nodes (SCSS)', () => {
        const expectedContents = `
	/*.one {
		height: 42px;

		.two {
			width: 42px;
		}

		.three {
			padding: 10px;
		}
	}*/`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(1, 3, 1, 3),
                new vscode_1.Selection(2, 5, 2, 5),
                new vscode_1.Selection(4, 4, 4, 4),
                new vscode_1.Selection(5, 5, 5, 5),
                new vscode_1.Selection(9, 5, 9, 5) // cursor inside a property inside a nested rule
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment with multiple cursors, but no selection (SCSS)', () => {
        const expectedContents = `
	.one {
		/*height: 42px;*/

		/*.two {
			width: 42px;
		}*/

		.three {
			/*padding: 10px;*/
		}
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 5, 2, 5),
                new vscode_1.Selection(4, 4, 4, 4),
                new vscode_1.Selection(9, 5, 9, 5) // cursor inside a property inside a nested rule
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                //return toggleComment().then(() => {
                //	assert.equal(doc.getText(), contents);
                return Promise.resolve();
                //});
            });
        });
    });
    test('toggle comment with multiple cursors and whole node selected (CSS)', () => {
        const expectedContents = `
	.one {
		/*height: 42px;*/

		/*.two {
			width: 42px;
		}*/

		.three {
			/*padding: 10px;*/
		}
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 2, 2, 15),
                new vscode_1.Selection(4, 2, 6, 3),
                new vscode_1.Selection(9, 3, 9, 17) // A property inside a nested rule completely selected
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment when multiple nodes are completely under single selection (CSS)', () => {
        const expectedContents = `
	.one {
		/*height: 42px;

		.two {
			width: 42px;
		}*/

		.three {
			padding: 10px;
		}
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 2, 6, 3),
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
    test('toggle comment when multiple nodes are partially under single selection (CSS)', () => {
        const expectedContents = `
	.one {
		/*height: 42px;

		.two {
			width: 42px;
	*/	}

		.three {
			padding: 10px;
		}
	}`;
        return testUtils_1.withRandomFileEditor(contents, 'css', (editor, doc) => {
            editor.selections = [
                new vscode_1.Selection(2, 6, 6, 1),
            ];
            return toggleComment().then(() => {
                assert.equal(doc.getText(), expectedContents);
                return toggleComment().then(() => {
                    assert.equal(doc.getText(), contents);
                    return Promise.resolve();
                });
            });
        });
    });
});
//# sourceMappingURL=toggleComment.test.js.map
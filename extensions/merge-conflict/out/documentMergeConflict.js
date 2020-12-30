"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.DocumentMergeConflict = void 0;
class DocumentMergeConflict {
    constructor(descriptor) {
        this.range = descriptor.range;
        this.current = descriptor.current;
        this.incoming = descriptor.incoming;
        this.commonAncestors = descriptor.commonAncestors;
        this.splitter = descriptor.splitter;
    }
    commitEdit(type, editor, edit) {
        if (edit) {
            this.applyEdit(type, editor.document, edit);
            return Promise.resolve(true);
        }
        return editor.edit((edit) => this.applyEdit(type, editor.document, edit));
    }
    applyEdit(type, document, edit) {
        // Each conflict is a set of ranges as follows, note placements or newlines
        // which may not in spans
        // [ Conflict Range             -- (Entire content below)
        //   [ Current Header ]\n       -- >>>>> Header
        //   [ Current Content ]        -- (content)
        //   [ Splitter ]\n             -- =====
        //   [ Incoming Content ]       -- (content)
        //   [ Incoming Header ]\n      -- <<<<< Incoming
        // ]
        if (type === 0 /* Current */) {
            // Replace [ Conflict Range ] with [ Current Content ]
            let content = document.getText(this.current.content);
            this.replaceRangeWithContent(content, edit);
        }
        else if (type === 1 /* Incoming */) {
            let content = document.getText(this.incoming.content);
            this.replaceRangeWithContent(content, edit);
        }
        else if (type === 2 /* Both */) {
            // Replace [ Conflict Range ] with [ Current Content ] + \n + [ Incoming Content ]
            const currentContent = document.getText(this.current.content);
            const incomingContent = document.getText(this.incoming.content);
            edit.replace(this.range, currentContent.concat(incomingContent));
        }
    }
    replaceRangeWithContent(content, edit) {
        if (this.isNewlineOnly(content)) {
            edit.replace(this.range, '');
            return;
        }
        // Replace [ Conflict Range ] with [ Current Content ]
        edit.replace(this.range, content);
    }
    isNewlineOnly(text) {
        return text === '\n' || text === '\r\n';
    }
}
exports.DocumentMergeConflict = DocumentMergeConflict;
//# sourceMappingURL=documentMergeConflict.js.map
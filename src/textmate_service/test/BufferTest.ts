/*
 * BufferTest.ts
 */

import * as Buffer from "./../src/Buffer"

describe("Buffer", () => {
    it("gets created correctly", () => {
        let buf = Buffer.create(["a", "b"], 1);    

        expect(buf.lines[0].contents).toBe("a");
        expect(buf.lines[1].contents).toBe("b");
    });

    describe("update", () => {
        it("handles a buffer update that clears current buffer", () => {
            let buf = Buffer.create(["a", "b"], 1);    

            buf = Buffer.update(buf, {
                id: 0,
                version: 2,
                lines: ["c", "d"],
                startLine: 0,
                endLine: -1,
            })

            expect(buf.lines[0].contents).toBe("c");
            expect(buf.lines[1].contents).toBe("d");
        });

        it("handles a buffer update inserts before the buffer", () => {
            let buf = Buffer.create(["a", "b"], 1);    

            buf = Buffer.update(buf, {
                id: 0,
                version: 2,
                lines: ["c", "d"],
                startLine: 0,
                endLine: 0,
            })

            expect(buf.lines[0].contents).toBe("c");
            expect(buf.lines[1].contents).toBe("d");
            expect(buf.lines[2].contents).toBe("a");
            expect(buf.lines[3].contents).toBe("b");
        });

        it("handles a buffer update inserts after the buffer", () => {
            let buf = Buffer.create(["a", "b"], 1);    

            buf = Buffer.update(buf, {
                id: 0,
                version: 2,
                lines: ["c", "d"],
                startLine: 2,
                endLine: 3,
            })

            expect(buf.lines[0].contents).toBe("a");
            expect(buf.lines[1].contents).toBe("b");
            expect(buf.lines[2].contents).toBe("c");
            expect(buf.lines[3].contents).toBe("d");
        });

        it("handles a buffer update inserts in the middle of the buffer", () => {
            let buf = Buffer.create(["a", "b"], 1);    

            buf = Buffer.update(buf, {
                id: 0,
                version: 2,
                lines: ["c", "d"],
                startLine: 1,
                endLine: 1,
            })

            expect(buf.lines[0].contents).toBe("a");
            expect(buf.lines[1].contents).toBe("c");
            expect(buf.lines[2].contents).toBe("d");
            expect(buf.lines[3].contents).toBe("b");
        });
    });
})


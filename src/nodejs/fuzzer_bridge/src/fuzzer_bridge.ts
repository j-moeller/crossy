// @ts-ignore
import * as fb from "./build/Release/fuzzer_bridge_native";
import { readBuffer } from "./util";

type UserCallback = (data: Buffer) => Promise<string>;

async function run(data: Buffer, target: UserCallback) {
    // fb.collect();
    try {
        console.error("OUTPUT: ", await target(data));
    } catch (e) {
        // Do something with exception?
    }
    // fb.collect();
}

function edgesToArray(n_edges: number) {
    return new Uint8Array([
        (n_edges & 0x000000ff) >> 0,
        (n_edges & 0x0000ff00) >> 8,
        (n_edges & 0x00ff0000) >> 16,
        (n_edges & 0xff000000) >> 24,
    ]);
}

export async function fuzz(target: UserCallback) {
    setInterval(() => { }, 1 << 30);

    const prefix = process.env.FUZZER_BRIDGE_PREFIX;
    const n_edges = 0x00001000;

    fb.openSharedMemory(prefix + "-edges", n_edges);
    fb.openSemaphores(prefix);

    fb.waitForParent();
    fb.writeToSharedMemory(edgesToArray(n_edges));
    fb.notifyParent();

    while (true) {
        fb.waitForParent();

        const buffer = await readBuffer(process.stdin, 8);
        const size = buffer.readUInt32LE(0) + (buffer.readUInt32LE(4) << 32);
        const data = await readBuffer(process.stdin, size);

        console.error(data, size);

        await run(data, target);
        fb.notifyParent();
    }
}
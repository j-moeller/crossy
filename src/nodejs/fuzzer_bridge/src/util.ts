// TODO: Currently, we register a new listener on each read. Maybe we could
// create a wrapper around the stdin-stream that continuously reads data as soon
// as it is ready and provides a "read(n_bytes): Promise<Buffer>" function that
// returns the appropriate amount of data

export async function readBuffer(stream: NodeJS.ReadStream, nbytes: number) {
    return new Promise<Buffer>((resolve, reject) => {
        const chunks: Buffer[] = [];
        let remaining = nbytes;

        if (nbytes === 0) {
            resolve(Buffer.concat(chunks));
            return;
        }

        function cleanUp() {
            stream.off("readable", read);
            stream.off("end", end);
        }

        function read() {
            const buffer = stream.read(remaining) as Buffer | null;
            if (buffer === null) {
                return;
            }

            remaining -= buffer.length;
            chunks.push(buffer);

            if (remaining === 0) {
                cleanUp();
                resolve(Buffer.concat(chunks));
                return;
            }
        }

        function end() {
            cleanUp();

            if (remaining == 0) {
                resolve(Buffer.concat(chunks));
            } else {
                reject("Stream ended with " + remaining + " bytes unread.");
            }
        }

        stream.on("readable", read);
        stream.on("end", end);
    })
}
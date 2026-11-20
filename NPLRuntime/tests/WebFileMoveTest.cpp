// Run with em++ WebFileMoveTest.cpp -lidbfs.js -o move-test.js && node move-test.js.
#include "../ParaEngine/Framework/FileSystem/WebFileMove.h"
#include <cassert>
#include <emscripten.h>

int main()
{
	EM_ASM({
		FS.mkdir('/worlds');
		FS.mount(IDBFS, {}, '/worlds');
		FS.mkdir('/worlds/directory');
		FS.writeFile('/worlds/existing', 'old');
		var bytes = new Uint8Array(20003);
		for (var i = 0; i < bytes.length; ++i) bytes[i] = i % 251;
		FS.writeFile('/tmp/region', bytes);
		FS.writeFile('/tmp/empty', new Uint8Array(0));
	});

	using ParaEngine::MoveWebFile;
	assert(MoveWebFile("/tmp/region", "/worlds/existing"));
	assert(MoveWebFile("/tmp/empty", "/worlds/empty"));
	assert(MoveWebFile("/worlds/empty", "/worlds/renamed"));
	assert(!MoveWebFile("/worlds/existing", "/tmp/missing-parent/file"));
	assert(!MoveWebFile("/worlds/directory", "/tmp/directory"));
	assert(!MoveWebFile("/worlds/renamed", "/tmp"));
	assert(!MoveWebFile("/tmp/missing", "/worlds/existing"));

	EM_ASM({
		var noSpace = $0;
		FS.writeFile('/tmp/retry', 'new data');
		Module.originalWrite = FS.write;
		FS.write = function(stream, ...args) {
			if (stream.node.name.startsWith('.npl-move-'))
				throw new FS.ErrnoError(noSpace);
			return Module.originalWrite.call(FS, stream, ...args);
		};
	}, ENOSPC);
	assert(!MoveWebFile("/tmp/retry", "/worlds/existing"));
	EM_ASM({
		FS.write = Module.originalWrite;
		if (FS.readFile('/tmp/retry', {encoding:'utf8'}) !== 'new data')
			throw new Error('failed move lost its source');
		var bytes = FS.readFile('/worlds/existing');
		if (bytes.length !== 20003 || !bytes.every((v, i) => v === i % 251))
			throw new Error('region contents changed');
		var temp = FS.readdir('/tmp');
		if (temp.includes('region') || temp.includes('empty') ||
			FS.readdir('/worlds').includes('empty') || FS.stat('/worlds/renamed').size !== 0)
			throw new Error('unexpected source or destination');
		if (FS.readdir('/').concat(FS.readdir('/worlds')).some(name => name.startsWith('.npl-move-')))
			throw new Error('failed move left a staging file');
	});
}

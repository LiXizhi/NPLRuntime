# BlockWorld height and rendering checks

The buildable range is −256 through 9,000. The native regression suite exercises both endpoints, the two adjacent out-of-range layers, picking, signed section encoding, legacy region loading, extended saves, block metadata and materials.

## Lighting

The test places a build at Y −256, changes a roof, drains the two-channel lighting queue and counts work. Counts include nearby air needed for propagation.

| Roof Y | Empty blocks above the build | Queued seeds | Queue visits | Stored sections in the column |
| ---: | ---: | ---: | ---: | ---: |
| 1 | 256 | 19 | 58 | 6 |
| 8 | 263 | 26 | 65 | 6 |
| 8,745 | 9,000 | 27 | 66 | 6 |
| 9,000 | 9,255 | 26 | 65 | 6 |

The two roofs with the same offset inside a section produced identical counts. All four cases left the intervening gap unallocated. The test covers an unloaded roof, glass, water, overlapping light sources across a section boundary, zero queued cells on a clean cached load and a save taken with pending light changes.

## Flight replay

Run `NPLRuntime/tests/test_block_height_flight.lua` as the bootstrapper against a disposable copy of Project 530 named `worlds/DesignHouse/_block_height_*`. Each view distance gets one 180-frame orbit to warm up and an identical 180-frame orbit to measure. The replay uses the game setting that adjusts the camera far plane, fog, terrain generation and block renderer together. It checks the effective distance and camera range. The dense fixture contains 16,384 blocks. The sparse fixture contains 1,089 blocks spaced one section apart. The replay verifies every placed block.

The original executable places the sparse fixture at Y 225, its highest comparable position. The revised executable places it at Y 8,960. The player's altitude relative to the fixture and the camera route match. Project 530 terrain lies closer to the original sparse route.

Use `heightSafetyOnly=true` to run the vertical teleport, physical jump, landing and floor fall cases. The fall checks intercept the death callback so the replay can continue across game modes. The test runs those cases with the real-terrain option enabled and disabled.

For the two-peer test, follow the launch instructions in `NPLRuntime/tests/test_block_height_network.lua`. It checks the login decision for legacy and height-capable peers. The server sends the bottom section first. The receiver verifies that it knows the height of the roof while its voxels are absent, then receives the roof, edits both endpoints and waits for the server to acknowledge both edits.

## Windows results

The original and revised release clients ran sequentially on an NVIDIA GeForce RTX 3070 Laptop GPU, using D3D12 at 1280 × 720. The Windows replay uses a high-resolution performance counter. Values are p95 wall-clock milliseconds per frame, original → revised.

| Route | 96 blocks | 256 blocks | 512 blocks |
| --- | ---: | ---: | ---: |
| Project 530 | 20.0 → 20.1 | 11.7 → 14.4 | 21.2 → 13.7 |
| Dense | 22.4 → 21.3 | 20.5 → 20.9 | 25.9 → 18.7 |
| Sparse | 14.9 → 20.6 | 20.1 → 18.9 | 32.1 → 20.6 |

The 512-block sparse route improved by 35.8%. The 96-block dense route improved by 4.9%. Peak process working set across the replay went from 666.3 to 386.4 MiB. At the end of the 512-block sparse route, renderable chunks went from 71,824 to 1,531.

## WebAssembly results

The original and revised WebAssembly builds ran in a visible Chromium WebGL2 window at 1280 × 720. The table reports p95 elapsed milliseconds between frames, including browser presentation pacing. Each build read back all 16,384 dense and 1,089 sparse blocks. Separate visual inspection confirmed the high-altitude geometry.

| Route | 96 blocks | 256 blocks | 512 blocks |
| --- | ---: | ---: | ---: |
| Project 530 | 19 → 19 | 18 → 18 | 33 → 25 |
| Dense | 17 → 19 | 19 → 17 | 41 → 42 |
| Sparse | 19 → 17 | 20 → 17 | 44 → 23 |

The 512-block sparse route improved by 47.7%. The 96-block dense route regressed by 11.8%. The peak WebAssembly linear heap went from 819.7 to 723.8 MiB. The 512-block sparse route finished with 71,824 original renderable chunks and 1,758 revised renderable chunks.

These results describe one machine and one set of routes. Browser frame pacing places a floor near 17 ms on this display. Windows working-set and WebAssembly heap figures measure different kinds of memory.

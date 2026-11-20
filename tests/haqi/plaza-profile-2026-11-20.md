# Haqi plaza, uncapped before and after

The optimized build rendered 239.1 FPS, up from 192.2 FPS (24.4% higher throughput). Average wall time per rendered frame fell from 5.20 to 4.18 ms. These are scene frame-counter deltas divided by monotonic elapsed time with both profilers stopped.

## Matched workload

Apple M3 Pro, arm64 Release, OpenGL, 1280 × 720 logical. Baseline engine fad3eb61a has the same uncapped-mode patch from 5158cfe1c. The comparison includes 6be2d9210 and f8dcbf476. Both clients use LuaJIT 2.1 revision 63c8f1a9 and identical compiler and linker flags. The scripts differ at the music-region change (da09fde6f versus 87f507ee8). Each runtime started with its own copy of the same cached assets and account data.

All timing polls reported 61HaqiTown and the same eye [20071.76672, 12.3473, 19707.21737] and look-at [20058.60138, 3.57404, 19694.982]. Instancing, JIT, the visible HUD and unlimited FPS were on. The character budget culled zero characters. Native draw and triangle counters returned zero on this backend, so there is no measured draw-count comparison. Players, effects and network traffic can change in a live plaza.

| Run order | Seconds | Counted frames | Rendered FPS |
|---|---:|---:|---:|
| before0 | 12.46 | 2434 | 195.4 |
| after0 | 12.46 | 2982 | 239.4 |
| after1 | 12.46 | 2975 | 238.8 |
| before1 | 12.50 | 2363 | 189.1 |

## Native main-thread profile

Apple sample, two 12-second windows per build, requested interval 1 ms. 18,157 baseline and 18,519 optimized samples. Each estimate multiplies inclusive sample share by elapsed wall time per frame within those sampling windows. Includes blocking and callees. Rows overlap. GPU execution time requires a GPU trace.

| Function | Before share | After share | Before ms/frame | After ms/frame |
|---|---:|---:|---:|---:|
| OpenGL present | 36.76% | 20.61% | 2.076 | 0.923 |
| CPU ParaX draw pass | 9.13% | 5.33% | 0.516 | 0.239 |
| Shader animation draw | 0.00% | 5.78% | 0.000 | 0.259 |
| Pose evaluation | 2.79% | 5.87% | 0.157 | 0.263 |
| Sprite quad submission | 6.79% | 8.39% | 0.383 | 0.376 |
| Static mesh draw | 9.06% | 15.02% | 0.512 | 0.673 |
| Ocean occlusion tests | 3.87% | 0.80% | 0.218 | 0.036 |
| Text layout | 1.88% | 1.12% | 0.106 | 0.050 |
| Particle draw | 1.79% | 3.00% | 0.101 | 0.134 |
| NPL dispatch | 1.69% | 1.77% | 0.095 | 0.079 |

## NPL main-state profile

LuaJIT statistical profiler at a requested 1 ms interval. For source attribution, a separate 35-second window per build collected VM states and up to eleven stack frames, with no simultaneous native sampling. Lua VM C-state includes time spent in native calls and time outside active script. Named script callers can appear even in C-state.

| Measurement | Before | After |
|---|---:|---:|
| All samples | 17,262 | 22,674 |
| C state | 17,116 | 22,587 |
| Compiled Lua | 75 | 52 |
| JIT compiler | 63 | 31 |
| Interpreted Lua | 8 | 4 |
| Samples with a named script | 162 | 110 |
| WriteToLogFile, innermost named frame | 33 | 21 |
| timer.lua in stack | 92 | 62 |
| HaqiBgmPlaylist.lua in stack | 35 | 27 |

The separate 104-location mask regression returned identical region keys on all six runs. Each baseline scan called tile preparation 712 times, each optimized scan 356 times. This fixture measures lookup work, independent of how often stationary plaza gameplay requests it.

## Next experiments in priority order

1. Trace OpenGL HUD batch breaks by texture set, clipping and custom font shader. The optimized sprite flush still costs about 0.38 ms/frame. An additional live HUD visibility test on the ported build measured 211.5 FPS visible, 238.5 FPS hidden, 209.5 FPS visible again. Hiding the entire HUD affects more than the sprite renderer. Use the same mixed-script HUD screenshot as the correctness check.
2. Count CPU fallback draws by mesh, material, and rejection reason. Time pose evaluation separately from vertex skinning. calcBones marks the whole rig dirty before evaluation. Count constant tracks and parent traversals on the expensive rigs before changing interpolation or caching poses. Preserve identity, billboard and script-driven transforms.
3. Capture one GPU frame and tag static-mesh material passes. Present is the largest measured wait. Compare repeated sampler and texture changes at CMeshObject::DrawInner, with a counter for draws actually merged. Main-thread sampling alone cannot distinguish GPU execution, driver submission and window-server waits.
4. On the NPL side, count repeated verbose network logging per call site and check log levels before building strings. WriteToLogFile is the most frequent named leaf in this capture. Retain error logs and verify on a longer network-active run. Music-region polling is the next script candidate for a stationary-player test. The small number of named Lua samples limits precision.

## Frame-rate switch verification

The macOS toggle test launched each backend with default settings, enabled profiling, changed the requested interval to 30 FPS, disabled profiling, and finally requested 60 FPS again. Median FPS at those five steps was 58, 275.5, 278.5, 29 and 58 on OpenGL, and 58, 357, 352, 29 and 58.5 on Metal. These checks used separate sessions from the plaza comparison.

The macOS Release build passed all eight CTest cases. A Chrome/WASM scheduler test restored its original requestAnimationFrame divisor after toggling profiling, pausing rendering and entering the simulated XR state. The affected Windows and Linux host sources cross-compiled. The Android JNI and EGL sources passed an ARM64 syntax check and the renderer Java class compiled against Android API 34. The iOS display-link change passed a Catalyst SDK syntax check. Full Haqi device runs on Windows, Linux, Android and iOS remain unverified. HarmonyOS has no active graphical host loop in this checkout.

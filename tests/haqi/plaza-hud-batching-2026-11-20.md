# Haqi plaza HUD batching and follow-up optimizations

## Uncapped plaza comparison

Apple M3 Pro, macOS arm64 Release, OpenGL, LuaJIT 2.1 at `63c8f1a9`, 1280 × 720 logical (2560 × 1440 screenshots). The baseline was engine `674931ad8` and scripts `87f507ee8`. The candidate includes this engine change and scripts `7817963c2`. All timing windows ran with `ProfileUnlimitedFPS=true`, JIT on, debug hooks off, and both profilers stopped. The ordinary frame-rate setting is independent of these changes.

Four foreground launches used the same account, cache, plaza coordinates and camera orientation. Each frame-counter window lasted roughly 12.5 seconds. A fixed crop of the open daily-reward window matched the reference before and after every window. Every timing poll confirmed foreground focus, the same viewport, eye and look-at point, and zero character-budget culls. Windows with mixed focus or reward-window visibility are excluded.

| Measure | Before | After |
|---|---:|---:|
| FPS by launch, in order within each build | 224.4, 225.8 | 228.5, 234.6 |
| Frames divided by elapsed time across two launches | 225.11 FPS | 231.58 FPS |
| Wall time per frame | 4.442 ms | 4.318 ms |

The foreground run order was candidate, baseline, baseline, candidate. Aggregate throughput changed by +2.87%. Both candidate runs exceeded both baseline runs in the foreground session. Player activity, effects and system load varied across launches.

A separate four-launch comparison while the desktop was locked kept both builds out of focus. Baseline runs reached 265.1 and 264.4 FPS. Candidate runs reached 265.2 and 246.0 FPS. These windows are excluded from the foreground figures.

## HUD submission and visual equivalence

A separate diagnostic build switched HUD paths in the same live client while keeping bone evaluation, shaders outside the GUI, and scripts fixed. Six 3.2-second timing windows per mode used a balanced order. Draw and CPU counters were collected inside nonempty sprite flushes. The totals below omit the initial screenshot exercises and the partial counter blocks straddling a switch.

| HUD path | Draws per 1,000 quads | Submission µs per 1,000 quads | Median rendered FPS |
|---|---:|---:|---:|
| Four slots and general transforms | 33.37 | 99.75 | 122.18 |
| Eight slots and axis-aligned transform shortcut | 26.91 | 86.22 | 122.82 |
| Eight slots, binding only used units on native GL | 27.16 | 85.64 | 123.71 |

The shipping path reduced HUD draws per quad by 18.6% and CPU submission time per quad by 14.1%. Both four-slot and eight-slot shader variants compiled for desktop GLSL 330 and ESSL 300. WebGL retains complete bindings across all declared slots.

RGB pixel comparisons found zero differences in the 968 × 428 mixed Chinese/Latin text and sprite fixture, the 1224 × 322 fixture with eleven overlapping patterned textures, rotated geometry and clipped text, and the 297 × 310 static reward-panel crop. The widened fixture also matched across all three live HUD modes. Animated portions of the online scene naturally differed between launches.

## Native and NPL work

Four separate eight-second main-thread sampling windows gave the following inclusive wall-time estimates. A function can include callees and driver waits.

| Native function | Before, ms/frame | After, ms/frame |
|---|---:|---:|
| Sprite quad flush | 0.397 | 0.322 |
| Bone pose evaluation | 0.296 | 0.276 |
| Static model rendering | 0.368 | 0.370 |
| Shader pass uniform synchronization | 0.260 | 0.195 |

- Animation key selection validates a per-thread interval hint against each track before use. A deterministic native regression produced byte-identical pose data for a 72-bone rig in eight combinations of layered animation, bone storage order and scripted overrides. All interpolated outputs also matched in a separate key-lookup test. In the 129-key fixture, sequential lookups went from 11.47 to 4.82 ns and randomized frame lookups from 12.29 to 5.12 ns. That benchmark shares frame times across 128 tracks. A second benchmark randomized the time independently for each track. Its 129-key trials went from 24.88–25.32 ns to 19.97–20.27 ns, with matching checksums.
- A one-frame GL capture recorded 149 static-mesh draws. Nine adjacent passes had contiguous indices. Zero also matched the primary and secondary textures, material flags and replaceable texture ID. The implementation keeps those draws separate. It reuses the linked uniform binding and skips scans when the pass has seen every effect parameter revision. Across 1,703,936 diagnostic synchronization calls on the plaza renderer, 43.6% skipped a scan, avoiding 3,103,716 binding visits. The renderer also reused 3,182,476 resolved uniform entries. After the plaza chair texture finished loading, the 384 × 384 offscreen renders matched pixel-for-pixel at two camera angles and on returning to the first.
- Short NPL log records without callbacks or memory logging used one native write per record, down from three. Formatting, embedded NULs, long records, warnings, errors, callback fragments and memory-log bytes were covered by a LuaJIT output-equivalence test. In the live client, 450 formatted short debug records took a median of 2.21 ms before and 0.92 ms after across nine repetitions per launch. A separate 20-second LuaJIT sample per build recorded 12,202 and 12,415 total samples, including 51 and 51 with named scripts. `WriteToLogFile` was the innermost named frame in 12 and 9 samples. The controlled logging test provides the more precise comparison.

The macOS Release client built successfully and all eight CTest cases passed. The local standalone graphics-test target omitted the macOS input constructors at link time. The two affected test binaries ran after linking their test objects with the client host objects. Haqi runtime measurements in this report cover macOS OpenGL.

## Follow-up on OpenGL uniform updates and mesh culling

Baseline engine `a3112ff0b`, Haqi scripts `aebd64fb4`, Apple M3 Pro, macOS Release OpenGL, 1280 × 720 logical, uncapped. In the first eight-second plaza trace, presentation occupied 29.9% of main-thread samples, almost entirely inside the system buffer swap. Static mesh rendering occupied 8.9%, sprite flushing 8.0%, and shader uniform synchronization 3.8%. These inclusive shares overlap.

The shipped change reuses storage for same-sized uniform updates. Deferred commits visit only the range from the earliest to the latest changed entry in cache insertion order. This preserves the ordering of writes to overlapping array locations. Across 5,767,168 nonempty commits in the live diagnostic, the full scan would visit 96,683,603 entries. The bounded scan visits 32,004,590, a 66.9% reduction.

| Matched plaza measurement | Baseline | Uniform change | Uniform change plus experimental bounds cache |
|---|---:|---:|---:|
| Main render-thread CPU, ms/frame | 5.161 | 5.099 | 5.139 |
| Wall time, ms/frame | 8.320 | 8.300 | 8.335 |
| Rendered FPS | 120.19 | 120.48 | 119.98 |

All three paths ran in one foreground client at the same plaza position, camera and HUD. A diagnostic build switched paths at presentation boundaries in all six possible orders, repeated 12 times. The table covers 216 windows of 39 frame intervals each, after warm-up at each switch. Thread CPU and wall clocks were read on the render thread; the diagnostic includes bookkeeping for the comparison. Resampling the 12 complete cycles gives an approximate 95% interval of 0.019–0.111 ms/frame saved on thread CPU by the uniform change. Its wall-time interval spans zero. The run establishes a CPU saving and leaves the FPS effect unresolved.

Slower comparisons were noisier. Separate foreground launches ranged from 181–300 FPS on the baseline. The uniform-only foreground launch reached 270 FPS. In a separate diagnostic session with 18 windows of roughly three seconds each, median FPS was 174, 156 and 160 respectively. Those windows varied substantially even within one mode.

A driver-independent replay through the production uniform setters and commit path produced the same 7,950,099 upload calls and matching ordered payload digest on both builds, including deferred writes, direct writes, array resizing and reset. The CPU timings below are medians of five runs with 48 cached locations.

| Changed locations per commit | Baseline | Uniform change |
|---|---:|---:|
| 1 | 72 ns | 22 ns |
| 4 | 168 ns | 64 ns |
| 48 | 1,834 ns | 867 ns |

The second experiment cached the eight transformed mesh bounds until the bounds, transform or render origin changed. In 1,200 comparisons with moving origins, changing bounds and rotations, every output vertex matched the baseline byte-for-byte. A stationary lookup fell from 13.0 to 4.3 ns. Changing the origin on every lookup raised the cost from 14.3 to 19.1 ns. The incremental whole-frame CPU benefit was inconclusive even with 1,376,256 cache hits in 1,376,256 counted stationary-plaza lookups. The cache was dropped.

The static HUD crop matched at both ends of the timed launches. The 384 × 384 reference mesh matched pixel-for-pixel across the three builds at two camera angles and on returning to the first. All eight macOS CTest cases passed with the shipped renderer, using the client host objects to link the two standalone graphics tests as above.

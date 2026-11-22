---
name: paracraft-model-builder
description: Build Paracraft scenes that combine editable native blocks with small world-local 3D props. Search Lucas Martinic’s Polyfork catalog, customize its parametric Three.js models, and place small GLB exports at block scale using desktop paracraft-cli.
---

# Build with blocks and catalog models

Use native blocks for walkable floors, stairs, walls, and other geometry the builder wants to edit. Use model blocks for details such as park benches, trees, birds, or equipment. One world-local GLB can serve many instances with different positions, scales, and headings.

## Select the world

Use the selected desktop client's local `/ajax/paracraft_cli` endpoint. The examples below use Bash, Python 3, and paths relative to this skill directory. Confirm `health` identifies the intended entered world. Capture the creation identity **before scouting coordinates**. Refresh it after a world switch or reopen and scout again.

```sh
cli=http://127.0.0.1:8099/ajax/paracraft_cli
identity=$(mktemp)
curl -fsS "$cli?action=get_creation_capabilities" > "$identity"
world=$(python3 -c 'import json,sys; print(json.load(open(sys.argv[1]))["result"]["identity"]["worldPath"])' "$identity")
```

Use the absolute world directory visible to this desktop engine. When the engine reports a relative path, resolve it against its writable files directory. Read the world's `AGENTS.md` with `world_files` and use the client's `creation.md` guide for the block layout. Enter edit mode with `run_command` and `{"command":"/mode edit"}` before writing.

## Choose and stage a prop

Choose the lightest model that preserves the needed silhouette, animation, and interaction at gameplay distance. Polyfork search prioritizes matching titles and ranks them by triangle count. Read the model's parameter schema and reduce segments, facets, or optional parts where they add little visible detail. For repeated scenery, use one shared GLB and vary instance transforms. Use native blocks when they can express the shape just as well. [Browse Polyfork](https://polyfork.dev/assets) · [API](https://polyfork.dev/llms.txt)

Use `measure` to compare plausible candidates or detail settings in memory before staging the winner. It reports actual GLB bytes, triangles, and mesh primitives, counting each mesh definition once. Primitives are a clue to rendering cost. For repeated props, consider how many copies will be visible together. When frame time matters, profile the assembled scene on the intended device.

```sh
python3 scripts/models.py search polyfork planter
python3 scripts/models.py info polyfork planter-f0dab0
python3 scripts/models.py measure polyfork planter-f0dab0 \
    --param tubTallness=1.4 --param shrubFacets=chunky
python3 scripts/models.py fetch polyfork planter-f0dab0 --world "$world" \
    --param tubTallness=1.4 --param shrubFacets=chunky
```

`measure` and `fetch` use a **64 KiB default ceiling**. Aim below it whenever a lighter variant fits. If a model exceeds it, try simpler settings or another matching asset first. For a close-up or animated asset that needs more detail, set a deliberate per-call `--max-bytes`. `fetch` reports the same geometry measurements plus the world-relative filename and SHA-256.

Each selected mesh lands under `blocktemplates/catalog/` with a `source.json` recording the parameter values, source page, and creator's credit line. Distribute the receipt with the mesh and include the credit and changes in world credits. The free low-poly meshes and baked variants are available under CC BY 4.0. The Three.js source has separate terms. Once staged, the GLB works offline. [Polyfork licensing](https://polyfork.dev/licensing)

## Fit the layout and place instances

1. Probe the imported bounds. For example, with `asset` set to the returned filename, run `python3 scripts/place.py "$identity" "$asset"`. To fit an object to a width in blocks, use `scale = desiredBlockWidth * blockSize / (max_x - min_x)` at zero yaw. Inspect the vertical minimum and footprint as well. Model origins and import units differ between assets.
2. Build the architecture with `run_code`, passing the captured `expectedIdentity`, a unique `requestId`, and Lua source. Poll `code_job` for completion. Return absolute carrier coordinates from the same scene that built the supports. This example yields a seven-block platform and a planter position centered above it.

   ```lua
   local s = createScene({name="planter_platform", dimensions={9,5,7}})
   s:group("deck")
   s:box({position={1,0,1}, dimensions={7,1,5}, blockId=68}) -- StoneBrick
   return {planter=s:position({4,1,3}), eye=s:cameraPoint({9,5,-5}),
           target=s:cameraPoint({4,1.5,3})}
   ```

3. Pass the three coordinates returned in `planter` as `x y z`. Use `--port` for a different client port and `--authoring-session` for the block job's named authoring session. The helper uses the native BlockModel carrier (254) through `run_npl_code` to place a world-relative GLB on a GLB-capable desktop engine. This route works on clients whose `scene:model` helper accepts only BMax/ParaX.

   ```sh
   python3 scripts/place.py "$identity" "$asset" --at "$x" "$y" "$z" --scale 1 --facing 0
   ```

4. Read the returned filename, transform, and loaded bounds. Capture the assembled scene with `camera_capture`, passing the scene's `eye`, `lookat=target`, and `expectedIdentity`. Check that the prop rests on the intended surface, faces the right way, and leaves routes clear. Record the asset paths and placements in the world's change record. A native world save is a separate `run_command` world operation when saving is in scope.

The placement helper polls cold assets, checks the world session and footprint, and recognizes an already-matching carrier. If a request times out after a write, inspect or retry the same file, transform, coordinates, and identity. Each placement is independent. Keep successful instances if a later one fails. Use the local engine's native collision blocks where players need a reliably walkable surface.

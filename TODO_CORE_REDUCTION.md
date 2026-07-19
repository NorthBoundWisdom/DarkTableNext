# Core reduction TODO

## Completed

- [x] Hard-remove the Lua runtime, bindings, scripts, build wiring, and tests.
- [x] Remove the `USE_LUA` option and all Lua compatibility paths.
- [x] Verify a no-Lua full build, build graph, dynamic links, and version output.
- [x] Commit the result as `e4f52d5 refactor: remove Lua runtime and integration`.
- [x] Remove slideshow, print, map, and tethering views.
- [x] Remove their camera, live-view, print-settings, map-settings, map-locations,
      location-search, GPX, and active geotagging workflows.
- [x] Remove MIDI and gamepad input modules.
- [x] Remove email and Piwigo storage backends; retain disk storage.
- [x] Reduce the supported image formats to RAW, JPEG, PNG, TIFF, RGBE/HDR, QOI, and copy.
- [x] Remove GraphicsMagick, ImageMagick, G'MIC compressed-LUT, and Colord integrations.
- [x] Remove the cltest, cmstest, chart, and generate-cache executables; retain the optional CLI.

## Core definition

For the next reduction passes, treat the core product as:

- local photo catalog and import;
- RAW decoding and the essential development pipeline;
- lighttable and darkroom views;
- local disk export, including JPEG, PNG, TIFF, and original-file copy.

## High-priority removable extensions

- [x] Remove peripheral views: slideshow, printing, map, and tethering.
- [x] Remove their companion modules: print settings, camera, live view, map settings,
      map locations, location search, and the GPS/GPX geotagging workflow.
- [x] Remove MIDI and gamepad input modules.
- [x] Remove remote export destinations: email and Piwigo; retain disk storage.

Removed build switches: `USE_MAP`, `BUILD_PRINT`, `USE_CAMERA_SUPPORT`,
`USE_PORTMIDI`, and `USE_SDL2`.

The remaining GPS scope is deliberately passive: read, preserve, and display existing image
coordinates. There is no map UI, location lookup, GPX import/matching, or active geotagging
workflow. Local disk export remains the only storage backend.

## Medium-priority removable extensions

- [x] Reduce format support to the core set. Removed PDF, PPM/PNM, PFM,
      OpenEXR, WebP, XCF, JPEG 2000, AVIF, HEIF/HEIC, and JPEG XL.
- [x] Remove GraphicsMagick/ImageMagick fallback import support; broad raster import is
      not part of the product scope.
- [x] Disable G'MIC compressed-LUT support. The `lut3d` module remains without G'MIC;
      only `.gmz` compressed LUT support is lost.
- [x] Remove non-product executables: `darktable-cltest`, `darktable-cmstest`,
      `darktable-chart`, and `darktable-generate-cache`. The CLI remains optional for a
      GUI-only product.
- [x] Remove Colord display-profile integration; it is optional integration, not editing
      semantics.

Removed build switches: `USE_OPENJPEG`, `USE_JXL`, `USE_WEBP`, `USE_AVIF`, `USE_HEIF`,
`USE_XCF`, `USE_OPENEXR`, `USE_GRAPHICSMAGICK`, `USE_IMAGEMAGICK`, `USE_GMIC`,
`USE_COLORD`, and `BUILD_CMSTEST`. `USE_OPENCL` remains in scope for the GPU strategy below.

## GPU acceleration strategy

- [ ] Treat the current OpenCL implementation as a legacy compatibility backend, not as a
      long-term required dependency. OpenCL can target the Apple Silicon GPU, but arm64 apps
      have no OpenCL CPU device; Apple has deprecated OpenCL and may remove it from a future
      macOS release.
- [ ] Replace it with a mandatory Metal compute backend for the macOS product rather than
      permanently enabling `USE_OPENCL`. Keep the CPU implementation as the correctness
      baseline and fallback while Metal coverage is incomplete.
- [ ] Before porting, benchmark representative RAW editing and export workflows to identify
      the few modules responsible for most pixel-pipeline time. Port those first; do not
      translate all OpenCL kernels mechanically.
- [ ] Build a backend-neutral GPU resource/dispatch boundary, then implement the hot kernels
      as precompiled Metal shaders. Chain compatible operations in command buffers and avoid
      needless CPU/GPU synchronization so Apple Silicon unified memory is actually useful.
- [ ] Validate output against the CPU path and measure end-to-end latency, throughput, memory,
      and energy on Apple Silicon. Remove the OpenCL runtime, `.cl` kernels, test target, and
      `#ifdef HAVE_OPENCL` branches only after the Metal path covers the chosen core pipeline.

## Last: optional image-operation modules

- [ ] Assess and remove creative or specialized processing modules separately, beginning
      with bloom, soften, overlay, velvia, vignette, split-toning, grain, borders,
      liquify, retouch, watermark, censorize, negadoctor, and agx.

Do not remove image-operation modules blindly: stored edits, styles, and presets reference
their operation names. Define a migration or explicit incompatibility policy before removing
them.

## Next batch

Continue with the GPU strategy and the optional image-operation module review.

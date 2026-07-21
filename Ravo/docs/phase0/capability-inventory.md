# Ravo Capability Inventory

## Purpose and status

This is a Phase 0 implementation inventory, not a promise to reproduce every
legacy IOP. [`TODO_REWRITE.md`](../../../TODO_REWRITE.md) retains the
product-boundary authority; several creative and specialised modules are still
explicit candidates for a separate keep/remove decision. The table identifies
work that must not be silently implied by a successful Ravo build.

The source inventory was collected from the 76 non-comment `add_iop(...)`
registrations in [`src/iop/CMakeLists.txt`](../../../src/iop/CMakeLists.txt).
The generated fixture manifest records the 68 operation names currently
represented by XMP regression assets.  A `yes` in the fixture column means
only that one or more legacy XMP files name that operation; it is not a
validated Ravo compatibility result.

## Phase 1 reserved descriptors

Phase 1 defines metadata and validation only.  It does not execute these
operations or claim old parameter compatibility.  These IDs are reserved so
the recipe model, registry, CLI, and legacy adapter can be tested before a
CPU pixel executor exists.

| Ravo operation ID | Legacy source operation | Planned first use |
| --- | --- | --- |
| `ravo.core.identity` | none | synthetic recipe and render-contract testing |
| `ravo.raw.prepare` | `rawprepare` | first RAW vertical-slice planning |
| `ravo.raw.demosaic` | `demosaic` | first RAW vertical-slice planning |
| `ravo.color.input` | `colorin` | first colour-chain planning |
| `ravo.core.exposure` | `exposure` | visible first-operation candidate |
| `ravo.color.output` | `colorout` | first colour-chain planning |
| `ravo.output.scale` | `finalscale` | output request negotiation |

The legacy adapter can map only a proven one-to-one operation with explicitly
handled parameters.  All other operation names must produce a structured
`unsupported_legacy_operation` diagnostic.  It must not retain opaque module
struct bytes or call the old dynamic module ABI.

## Legacy registry census

| Legacy IOP | Fixture | Phase 0 Ravo disposition |
| --- | --- | --- |
| `agx` | yes | product decision pending; no Phase 1 implementation |
| `ashift` | yes | defer until geometry/ROI contract exists |
| `atrous` | yes | defer until shared denoise facilities exist |
| `basecurve` | yes | product decision pending; no Phase 1 implementation |
| `bilat` | yes | defer until shared denoise facilities exist |
| `bilateral` | yes | defer until shared denoise facilities exist |
| `bloom` | yes | product decision pending; no Phase 1 implementation |
| `blurs` | yes | defer until shared blur facilities exist |
| `borders` | yes | product decision pending; no Phase 1 implementation |
| `cacorrect` | yes | defer with RAW geometry capability |
| `cacorrectrgb` | no | defer with RAW geometry capability |
| `censorize` | yes | product decision pending; no Phase 1 implementation |
| `channelmixerrgb` | yes | defer until colour operation policy exists |
| `colorbalance` | no | defer until colour operation policy exists |
| `colorbalancergb` | yes | defer until colour operation policy exists |
| `colorchecker` | yes | defer until colour operation policy exists |
| `colorcontrast` | yes | defer until colour operation policy exists |
| `colorcorrection` | yes | defer until colour operation policy exists |
| `colorequal` | yes | defer until colour operation policy exists |
| `colorharmonizer` | yes | product decision pending; no Phase 1 implementation |
| `colorin` | yes | reserved `ravo.color.input`; execution deferred to Phase 2 |
| `colorize` | yes | defer until colour operation policy exists |
| `colormapping` | yes | defer until colour operation policy exists |
| `colorout` | yes | reserved `ravo.color.output`; execution deferred to Phase 2 |
| `colorreconstruct` | yes | defer with RAW reconstruction capability |
| `colorzones` | yes | defer until colour operation policy exists |
| `crop` | no | defer until geometry/ROI contract exists |
| `demosaic` | yes | reserved `ravo.raw.demosaic`; execution deferred to Phase 2 |
| `denoiseprofile` | yes | defer until denoise model and fixtures exist |
| `diffuse` | yes | defer until shared denoise facilities exist |
| `dither` | yes | defer until output quantisation contract exists |
| `enlargecanvas` | yes | defer until geometry/ROI contract exists |
| `exposure` | yes | reserved `ravo.core.exposure`; execution deferred to Phase 2 |
| `filmicrgb` | yes | defer until colour operation policy exists |
| `finalscale` | no | reserved `ravo.output.scale`; execution deferred to Phase 2 |
| `flip` | no | defer until geometry/ROI contract exists |
| `gamma` | no | defer until colour operation policy exists |
| `graduatednd` | yes | defer until mask/blend contract exists |
| `grain` | yes | product decision pending; no Phase 1 implementation |
| `hazeremoval` | yes | defer until shared dehaze facilities exist |
| `highlights` | yes | defer with RAW reconstruction capability |
| `highpass` | yes | defer until shared blur facilities exist |
| `hotpixels` | no | defer with RAW decode capability |
| `lens` | yes | defer until lens database adapter is designed |
| `liquify` | yes | product decision pending; no Phase 1 implementation |
| `lowlight` | yes | defer until colour operation policy exists |
| `lowpass` | yes | defer until shared blur facilities exist |
| `lut3d` | yes | defer until LUT adapter and colour policy exist |
| `mask_manager` | yes | defer until canonical mask graph exists |
| `monochrome` | yes | defer until colour operation policy exists |
| `negadoctor` | yes | product decision pending; no Phase 1 implementation |
| `nlmeans` | yes | defer until shared denoise facilities exist |
| `overexposed` | no | diagnostics-only legacy behaviour; no Phase 1 implementation |
| `overlay` | yes | product decision pending; no Phase 1 implementation |
| `primaries` | yes | defer until colour operation policy exists |
| `profile_gamma` | no | defer until colour operation policy exists |
| `rasterfile` | yes | defer until raster adapter and mask contract exist |
| `rawdenoise` | yes | defer with RAW decode capability |
| `rawoverexposed` | no | diagnostics-only legacy behaviour; no Phase 1 implementation |
| `rawprepare` | yes | reserved `ravo.raw.prepare`; execution deferred to Phase 2 |
| `retouch` | yes | product decision pending; no Phase 1 implementation |
| `rgbcurve` | yes | defer until curve schema and colour policy exist |
| `rgblevels` | yes | defer until colour operation policy exists |
| `rotatepixels` | no | defer until geometry/ROI contract exists |
| `scalepixels` | no | defer until geometry/ROI contract exists |
| `shadhi` | yes | defer until shared denoise facilities exist |
| `sharpen` | no | defer until shared convolution facilities exist |
| `sigmoid` | yes | defer until colour operation policy exists |
| `soften` | yes | product decision pending; no Phase 1 implementation |
| `splittoning` | yes | product decision pending; no Phase 1 implementation |
| `temperature` | no | defer with RAW colour capability |
| `tonecurve` | yes | defer until curve schema and colour policy exist |
| `toneequal` | yes | defer until colour operation policy exists |
| `velvia` | yes | product decision pending; no Phase 1 implementation |
| `vignette` | yes | product decision pending; no Phase 1 implementation |
| `watermark` | yes | product decision pending; no Phase 1 implementation |

## Data and export inventory

| Capability | Phase 1 decision | Rationale |
| --- | --- | --- |
| Legacy XMP input | Parse only a bounded XML subset; return structured incompatibility outside proven mappings | Prevent old module ABI or opaque bytes from crossing the boundary |
| Canonical recipe | Version 1 JSON, immutable snapshots, explicit schema upgrades | Required by every future client |
| RAW/JPEG/PNG/TIFF decode | Not implemented | Requires codec adapters and fixture-backed behaviour in Phase 2 |
| JPEG/PNG/TIFF/original export | Not implemented | Requires atomic file and metadata contracts in Phase 2 |
| Masks and blending | Modelled as versioned data only | Pixel semantics require dedicated CPU tests and ROI rules |
| Catalog, history, styles | Explicitly out of Phase 1 | No database or desktop target before the headless exit |
| GPU/OpenCL/Metal | Explicitly out of Phase 1 | CPU-only reference work precedes any backend adapter |

The next inventory update must cite the Ravo test, legacy mapping, fixture, and
product decision that changes a row.  A compile-only change is not sufficient
evidence to move any disposition to "Ravo accepted".

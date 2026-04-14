# CST Implementation Plan

Implementation plan for adding Constant Strain Triangle (CST) continuum elements to Instabeam. Derivation reference: [`new-element-derivations/cst_continuum_element.py`](new-element-derivations/cst_continuum_element.py).

## 1. Architectural prerequisite: generalize the "Rod-centric" core

The current calculator, coincidence table, GSM assembly, and boundary conditions are hard-coded around `QList<Rod*>` with 6 DOFs = (y1, m1, y2, m2, x1, x2) across 2 nodes. CST has **3 nodes × 2 translational DOFs = 6 total**, no rotations. This is the biggest refactor and must come first.

- Introduce an abstract `FemElement` interface (or add virtuals to `TrussElement`):
  - `int nodeCount()`, `Node* getNode(int i)`
  - `int dofCount()` (always 6 for Rod/CurvedBeam/CST, but future-proof)
  - `Eigen::MatrixXd computeLocalESM()` / `computeTransformation()`
  - `DofKind dofKind(int localId)` → {TransX, TransY, RotZ} for connectivity rules
  - `setDOF / getDOF / setU / getU` with variable count
- Rewrite `Calculator::parseAndNumberElements`: iterate generic `FemElement*`, assign DOFs per-node-per-kind. Gerber-joint logic applies only to RotZ; CST contributes only TransX/TransY so it merges cleanly at any node regardless of node type.
- Rewrite `assembleCoincidenceTable` / `assembleGSM` with `MatrixXd` (not `Matrix6d`) so future elements with other sizes work.
- Update `applyConstraints` and `applyResults` to iterate over nodes of each element rather than the hardcoded `{0,1,4}/{2,3,5}` local-DOF pattern.

## 2. New element class: `elements/triangle.{h,cpp}`

- `Triangle : public TrussElement` (not derived from Rod).
- Holds `Node *node1, *node2, *node3` (weak ptrs), like Rod does.
- Material: `E`, `nu`, `t` (thickness), `planeStress` (bool). Default: steel, t=0.01 m, plane stress.
- `int dof[6]`, `double u[6]`.
- Result fields: `double sigma_xx, sigma_yy, sigma_xy` (constant per element).
- `ElementType::Triangle = 7` in `trusselement.h`.
- z-value constant between Rod and Node so it renders behind rods.

## 3. ESM computation — use standard B-matrix form, not the right-triangle symbolic output

The `cst_continuum_element.py` derivation assumes axis-aligned `a×b` right triangle. Generalizing that to arbitrary triangles requires a rotation + shear matching the triangle — error-prone. Instead use the textbook equivalent (same strain energy, arbitrary geometry):

```
B = (1/2A) * [[y23, 0, y31, 0, y12, 0],
              [0, x32, 0, x13, 0, x21],
              [x32, y23, x13, y31, x21, y12]]
D = (E/(1-ν²)) * [[1, ν, 0], [ν, 1, 0], [0, 0, (1-ν)/2]]      # plane stress
k_local = t * A * Bᵀ D B
```
No rotation matrix needed — `B` already uses global coords, so `T_e = I`.

Add a commented reference in the cpp pointing to `new-element-derivations/cst_continuum_element.py` and noting the equivalence.

## 4. Factory + UX wiring

- `factories/triangleadder.{h,cpp}`: click three nodes to create element. Preview with a virtual third node that follows the cursor, mirroring `CurvedBeamAdder`.
- Toolbar + `mainwindow.cpp`: new action "Add triangles" with icon (reuse a simple polygon SVG in `resources/`).
- `graphicsscene.cpp`: mode handling, same pattern as rods/beams.
- `elementselectiondialog`: register button text "Triangle (ID: …)".

## 5. Easy-change dialog: `widgets/easychange/triangledialog.{h,cpp,ui}`

Fields: E, ν, thickness, plane-stress/plane-strain radio, node IDs (readonly), computed area, stress results (σxx, σyy, σxy, von Mises) after solve.

## 6. Rendering

- `paint()`: filled polygon (3 nodes). Use color-map over **von Mises stress** after solve (analogous to rod inner-force coloring). Before solve, light translucent fill.
- `boundingRect()` / `shape()`: QPolygonF path; `shape()` uses full polygon for clicking.
- Deformed-system overlay: draw a polygon offset by node displacements (CST has linear displacement field — just translate each vertex).

## 7. Calculator integration

- In `determineESM`, add `if (auto tri = dynamic_cast<Triangle*>(el))` branch producing the 6×6 CST matrix; `T_e = Identity`.
- In `applyConstraints`: triangle nodes with no bearing use the same force-boundary logic as rod nodes — already generic once DOF-kind refactor (step 1) lands. Pure CST nodes have no rotation DOF, so the moment row simply doesn't exist for them.
- In `applyResults`: compute element strain `ε = B·u`, stress `σ = D·ε`, store on element; track max von Mises for colormap scaling.

## 8. Serialization

- `jsonkeys.h`: add `triangleArray`, `node1Id/node2Id/node3Id` (or reuse and add node3Id), `thickness`, `poisson`, `planeStress`.
- `saveAsJson` / `loadFromJson` on `Triangle`.
- `graphicsscene.cpp` save/load paths: add triangle array handling next to rod/curvedBeam arrays.

## 9. System-definition dialog

- New "Triangles" tab (`systemdefinitiondialog.{cpp,ui}`) listing elements with columns: ID, Node1, Node2, Node3, E, ν, t, σ_vM.

## 10. Minor chores

- About dialog: extend credits ("CST continuum elements: derived & implemented by Wolfgang Flachberger").
- README: move CST out of Todo into Features, link to derivation.
- Add a test model in `resources/` (e.g., cantilever plate under tip load) as a loadable `.json` sanity check.

## Suggested commit sequence

1. Refactor to `FemElement` abstraction + `MatrixXd` assembly (no behavior change; all rod tests still pass).
2. Add `Triangle` element + ESM + solver wiring (headless; constructable only via JSON).
3. Add factory + toolbar + rendering.
4. Add easy-change dialog + system-definition tab.
5. Stress visualization + deformed overlay.
6. Sample model + README update.

## Key risks / decisions to confirm before coding

- **Refactor scope**: step 1 touches `calculator.cpp`, every element's DOF handling, and JSON load. Alternative: keep rod-based calculator and add a parallel CST path — faster but creates two assembly codepaths. Recommend the refactor.
- **Plane stress vs plane strain default**: propose plane stress with a per-element toggle.
- **Mesh generation**: out of scope — user places triangles manually, same interaction model as rods.

## Fixed-shape constraint

CST elements must have a pre-defined shape (as defined by the interpolation functions) — otherwise the derived ESMs will not work. This is not very versatile; however, the speed of the calculation will be maintained and it fits in with the scheme of the other elements that have predefined ESMs.

# shared/ — WHAT IS AUTHORITATIVE AND WHAT IS NOT

**Three files, and they do not have the same standing.** A folder where one file is a source and
another is an illustration, both looking like documents, is the arrangement that produced this
round's divergence in the first place — so the standing is stated per file rather than implied by
being in here together.

| File | Standing | Read it for |
|---|---|---|
| `FONTS.md` | **AUTHORITATIVE, and the register for one distinction this bundle cannot afford to lose** | every face, its licence, and which of three states it is in — ships, **absent by decision** (letterforms ship as artwork), **absent by omission** (a file someone must copy). Each short `fonts/` directory carries an `ABSENT.md` saying which kind it is |
| `HEADER-PART.md` | **AUTHORITATIVE.** The contract for the header, revision 2 plus §10. Where it and a casting spec disagree on header geometry, this file wins | the block, the band, the LCD cell with its 49-character budget and 47-character cap, the Program-button construction and five-state matrix, the meter wells, the descriptor anchor at y 78, and §10's propagation table |
| `BRAND-AMENDMENT-BYPASS.md` | **PROPOSAL. Binding on nothing.** | before implementing bypass — it argues the disengaged treatment should cover only what is disengaged, since two of six castings already depart from the full-bleed rule the same way. Until adopted, the current BRAND.md rule stands |

## Inventory — authoritative, derived, deliberately absent

Every file in this delivery is one of three things, and the difference decides what you may read a
figure off. **Derived files are outputs: correct when cut, and stale the moment their source
moves.** A figure taken off a derived file is a figure with its base left out, which is how three
of this round's wrong numbers happened.

| Kind | Files | What you may read off it |
|---|---|---|
| **Authoritative** | `shared/HEADER-PART.md`, each casting's `GUI-SPEC.md`, each `RECUT.md`, `MANIFEST.md` | every figure. Where two disagree on the header, `HEADER-PART.md` wins; on a casting's own material, ink or strings, that casting's spec wins |
| **Derived** | `fifth-member/plate/fifth-member-plate-3x.png`, `chorus-60/assets/*`, `elmer/assets/*`, `taperot/assets/taperot-wordmark.png`, every `icons/` file | nothing. They are cuts of the panels at a stated scale. Check them **against** the two dimension columns in the sheet beside them; do not measure a new figure on one |
| **Deliberately absent** | `Header Part — Six Materials` (the part's showcase render) · Impact Label Reversed · Permanent Marker | nothing, and that is the point. Their absence is a decision, restated below so it cannot be read as an oversight |
| **Was missing, now landed** | Share Tech Mono and Barlow Condensed SemiBold in `reflect-84/fonts/`, `taperot/fonts/`, `elmer/fonts/` | every figure measured on them. **These were never the absent-by-decision kind** — both are OFL and the omission was packaging. All six castings now hold the faces they render, so the §11 type-adoption gate is satisfied rather than waived |
| **Derived from a superseded source** | icons copied forward this call to restore the sets — chorus-60 16/32/64/128/512 + light 512, reflect-84 32/64/128, taperot 32 | nothing. **Correct at their size and verified after the copy**, but cut before this round's ink pass; treat as provisional until re-cut. **A missing size below 64 is not a resample** — Gatecrasher's ladder draws each size at its size |
| **Source, and the exception among icons** | `taperot/icons/taperot-icon.svg` (1024 viewBox) | **figures.** It is the only artefact that regenerates a size below 64 as a drawn cut rather than a resample, which is why dropping it would have made that ladder unrepeatable. The only icon file in the delivery you may measure on |

**Design-side sources, not shipped, named so a re-cut is not a re-invention:** the panels
themselves (`<Casting> Panel.dc.html`) are where every derived file is cut from, and
`Artwork Cutting Sheet.dc.html` holds the four pieces of this bundle's artwork that had no source
at all — Elmer's meter face and needle, Chorus-60's two switch states.

**Deliberately absent: `Header Part — Six Materials`.** The part's showcase render is **not
shipped**, because it is not citable: in one pass it was found carrying a wrong model number, a
wrong descriptor, a string lifted from the wrong row, an ink worse than the body it was meant to
have fixed, and two block materials that do not ship. **No figure is read off it, no ratio measured
on it, no string taken from it — the strip is deliberately absent, so there is nothing to read a
figure off.** Its geometry demonstration is a design-side tool.

**Deliberately absent: two font binaries.** Impact Label Reversed (TapeRot's wordmark) and
Permanent Marker (Fifth Member's nameplate and tape) are **absent by licensing, not missing**.
Neither can be embedded in a shipping binary, so both faces' letterforms ship as artwork and the
fonts do not. Both absences are declared in their own `GUI-SPEC.md`; an absent font that is not
declared looks like a delivery defect and gets "fixed" by substituting a face, which moves every
measurement taken from the nameplate.

**What is authoritative in `HEADER-PART.md`, and what is not:** it owns **geometry and
behaviour** — coordinates, sizes, budgets, state matrices. It does **not** own **material, ink or
strings**; those are the casting's, per its own §1, and each `GUI-SPEC.md` states them. A figure
in this folder that describes a colour is describing the part's construction, not a casting's
palette.

**Revision 3 is open**, and §10 records which of its four items is which: three are figures
waiting on build answers (the meter's display clamp, its number format at both ends, the sign
convention), and the fourth — how a change to this part reaches six bodies — is already written,
because it is a process question that outlives the round.

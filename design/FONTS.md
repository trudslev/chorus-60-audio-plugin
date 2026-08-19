# FONTS — WHAT SHIPS, WHAT IS ABSENT BY DECISION, WHAT IS ABSENT BY OMISSION

**Three states, and the middle two must never be read as each other.** This bundle contains both a
font that is deliberately not here and fonts that are accidentally not here. **An absence by decision
is a fact about the licence; an absence by omission is a fact about the packaging** — and the second
kind is the only kind anyone should try to fix by adding a file.

| Face | Used by | Licence | State |
|---|---|---|---|
| Barlow Condensed SemiBold | all six — panel lettering (call 7) | OFL | **ships in all six** — reflect-84 and taperot landed this call |
| Share Tech Mono | all six — the shared LCD face, meter values | OFL | **ships in all six** — reflect-84 and elmer landed this call |
| IBM Plex Mono | elmer, reflect-84 — numerals, units, model line | OFL | ships |
| Jost | reflect-84 — wordmark | OFL | ships |
| **Archivo Expanded Bold** (+ the variable file) | elmer — wordmark | **OFL 1.1** | **ships** — landed this call, `elmer/fonts/` |
| Librestile Extended | chorus-60 — wordmark | licensed, embeddable | ships |
| Tudor Victors | gatecrasher — wordmark | licensed, embeddable | ships |
| **Impact Label Reversed** | taperot — wordmark | **donationware, not embeddable** | **ABSENT BY DECISION.** Letterforms ship as artwork: `taperot/assets/taperot-wordmark.png` |
| **Permanent Marker** | fifth-member — nameplate, tape strings | **not embeddable** | **ABSENT BY DECISION.** Letterforms baked into `fifth-member/plate/fifth-member-plate-3x.png` |

## The omissions — **closed**

| Casting | Was missing | Now at |
|---|---|---|
| reflect-84 | Share Tech Mono, Barlow Condensed SemiBold | `reflect-84/fonts/ShareTechMono-Regular.ttf` · `reflect-84/fonts/BarlowCondensed-SemiBold.ttf` |
| taperot | Barlow Condensed SemiBold | `taperot/fonts/BarlowCondensed-SemiBold.ttf` |
| elmer | Share Tech Mono | `elmer/fonts/ShareTechMono-Regular.ttf` |

**No absence by omission remains in this delivery.** Every face any casting renders is either in that
casting's `fonts/` or is one of the two by-decision absences below, whose letterforms ship as artwork.

**The `ABSENT.md` markers are retired where the omission closed** — reflect-84's and elmer's are gone,
because a marker naming a file that is now present is worse than no marker. **TapeRot's stays**, and
that is deliberate: its directory is still short one face on purpose, and without the marker that
shortness reads as the omission that just got fixed. Fifth Member's stays for the same reason.

## The gate, and why it was a gate

**No casting adopts the shared LCD budget of 49 or the cap of 47 until its own `fonts/` holds Share
Tech Mono.** Binding: `HEADER-PART.md` §11. **All six now hold it, so the gate is satisfied rather
than waived** — the rule stays in force for a seventh casting.

Both figures are measured on that face against the 538.00 name area. **A cap may never shrink**,
because it limits how long a saved Program name may be — lowering it orphans names users have already
saved. So a cap adopted against a face that turns out absent is not a re-export but a data migration.
**The one irreversible figure in the part is the one most easily asserted.**

**Elmer was the named instance:** cap **22 → 47**, the largest rise in the suite, in a casting whose
folder held no Share Tech Mono. **The face is now in it, so the rise is measured against a face that
ships** and Elmer's LCD no longer has to be deferred. Reflect-84's type pass is unblocked — its body
was already finished, so it is the casting the gate was costing most. TapeRot, the cheapest remaining
panel at +4 px, has its last dependency closed.

## Elmer's wordmark: Archivo, not Archivo Black

**The exact face:**

| Field | Value |
|---|---|
| Family | **Archivo** |
| Designer / foundry | Héctor Gatti · **Omnibus Type** |
| Licence | **SIL Open Font License 1.1** — embeddable |
| Kind | **variable**, two axes: `wdth` and `wght` (Archivo became variable in 2021; current release 2.001) |
| Google Fonts filename | **`Archivo[wdth,wght].ttf`** — the axes-in-brackets convention, no `VF` suffix |
| Repository path | `ofl/archivo/` in `google/fonts` |
| **Face of record** | **`elmer/fonts/Archivo_Expanded-Bold.ttf`** — `wdth` 125 · `wght` 700, 121136 bytes |
| Also shipped | **`elmer/fonts/Archivo-VariableFont_wdth_wght.ttf`** — 652084 bytes; the same design as a variable file |
| Not these | `Archivo-Bold.ttf` (normal width) · `Archivo_SemiExpanded-*` (`wdth` ~112) · `Archivo-Black.ttf` (`wght` 900) · `Archivo_Expanded-SemiBold.ttf` (unused — see below) · any `*Italic` |
| CSS as the panels declare it | `font-family:'Archivo'; font-weight:700; font-stretch:125%` |
| Equivalent low-level form | `font-variation-settings:'wdth' 125,'wght' 700` |
| Request URL the panels load | `https://fonts.googleapis.com/css2?family=Archivo:wdth,wght@125,600;125,700` |

**Both files ship, and they carry different jobs.** The **static** is the face of record — the build
embeds statics, and `Archivo_Expanded-Bold.ttf` is exactly `wdth 125 / wght 700` with no axis to set.
The **variable** file is included because the design sources declare the stretched form
(`'Archivo'` + `font-stretch:125%`), which only a variable file can satisfy; a build that prefers it
can use it unchanged.

**Only weight 700 is used, anywhere.** `Archivo_Expanded-SemiBold.ttf` is **not needed** and should
not be shipped. The panels' request URL asks for `125,600;125,700`, but **no element in any of the six
panels or in the header part renders weight 600 in Archivo** — the 600 is an over-request left in the
URL, now removed from it. An earlier revision of this section said the header part needed SemiBold;
that was read off the request URL rather than off the markup, which is the same class of error as
trusting a ratio without its base.

**Two corrections to earlier revisions of this section, kept because both were confidently wrong:**
the static family *does* ship an Expanded width (no instance needs cutting — Archivo's statics cover
six widths at every weight), and SemiBold was never required.

**With static faces, drop `font-stretch`.** The width is already in the file, so
`font-stretch: 125%` on top of an Expanded face either does nothing or synthesises a second stretch,
depending on the renderer:

```css
@font-face { font-family: 'Archivo Expanded'; src: url('fonts/Archivo_Expanded-Bold.ttf') format('truetype');
             font-weight: 700; font-display: swap; }
@font-face { font-family: 'Archivo Expanded'; src: url('fonts/Archivo_Expanded-SemiBold.ttf') format('truetype');
             font-weight: 600; font-display: swap; }
```

then `font-family:'Archivo Expanded'; font-weight:700` and **no `font-stretch`**. The panels' current
declaration (`'Archivo'` + `font-stretch:125%`) is correct for the *variable* file only; a build
installing statics must change the declaration with them, or the wordmark's width comes from the
wrong place.

§6's **31 px** is measured on `wdth 125 / wght 700` and is correct for it.

**`ArchivoBlack-Regular.ttf` is a different typeface, not a heavier setting of the same one.** Archivo
Black is a **separate release within the Archivo superfamily** — alongside Archivo Narrow — shipped as
a single static face at weight 900 with **no axes at all**. It can reproduce neither the stretch nor
the weight, and it is a different drawing: heavier and squarer, not the 700 master widened. The panel's stack reads
`font-family:'Archivo','Archivo Black',sans-serif; font-weight:700; font-stretch:125%` — **Archivo
Black is the fallback that appears when the variable face is missing**, which is exactly what the
build is seeing. It was never the specified face.

**So `wordmarkSize = 53` and §6's `31` are not the same quantity measured differently, and the build
is right to refuse the conversion.** They are em sizes of two unrelated faces. Adopting 31 against
Black would render the wordmark narrower *and* heavier — two changes at once, from a table that only
appears to disagree by a number.

**This was an omission in this file, not a disagreement in the spec.** Archivo had no row here at
all, so the one face whose absence would silently substitute was the one face the table did not
declare — the failure the table exists to prevent. It is declared now.

**Until the binary lands:** keep the panel's current wordmark rather than adopting 31 against Black.
A size for Black is deliberately *not* given here — it would have to be measured on Black, and
publishing a converted figure is how a substituted face gets legitimised.

## What a substituted face costs, stated once

An undeclared absent font gets "fixed" by substituting a metrically different one, **which moves
every measurement taken from the nameplate** — that is why both by-decision absences are declared in
their casting's own `GUI-SPEC.md` §9 / §10 and restated here. Nobody has substituted a face and
nobody will, but the two kinds of absence sitting in one table is what keeps that true.


---

## Where the prototypes look for these faces

Three of the six `designs/*.dc.html` panels declare a per-casting face with `url('fonts/…')` — a path
relative to the prototype, not to its casting folder. **So `designs/fonts/` carries those binaries as
well**, duplicating two files that also ship under `gatecrasher/fonts/` and chorus-60's. The
duplication is deliberate: a missing `@font-face` source is silent, the browser substitutes and lays
the page out on the fallback's metrics, and every width measured afterwards belongs to a face nobody
chose. Two extra binaries are cheaper than one silent substitution.

| Prototype declares | Resolves at `designs/fonts/` |
|---|---|
| `TudorVictors.ttf` | yes |
| `LibrestileExtBold.ttf` | yes |
| `ImpactLabelReversed.ttf` | **no — absent by decision, see `designs/ABSENT.md`** |

**Librestile Extended's "ships" row above was accurate as an intent and wrong as a fact** in export 3:
the face was in no location in that bundle, so Chorus-60's runtime nameplate — Librestile Extended
28 / 32 — could not be checked against its own prototype. It ships at `designs/fonts/` from export 4.

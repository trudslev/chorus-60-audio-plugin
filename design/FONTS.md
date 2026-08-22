# FONTS — WHAT SHIPS, WHAT IS ABSENT BY DECISION, WHAT IS ABSENT BY OMISSION

**Three states, and the middle two must never be read as each other.** This bundle contains both a
font that is deliberately not here and fonts that are accidentally not here. **An absence by decision
is a fact about the licence; an absence by omission is a fact about the packaging** — and the second
kind is the only kind anyone should try to fix by adding a file.

| Face | Used by | Licence | State |
|---|---|---|---|
| Barlow Condensed SemiBold | all six — panel lettering (call 7) | OFL | **ships in all six** — reflect-84 and taperot landed this call |
| Barlow Condensed **Medium (500)** | **gatecrasher (4 §8 rows), taperot (1), fifth-member (1)** — counted against the tables, not by grep | OFL 1.408 | **ships in all three** — closed 2026-08-22, see below |
| Share Tech Mono | all six — the shared LCD face, meter values | OFL | **ships in all six** — reflect-84 and elmer landed this call |
| IBM Plex Mono | elmer, reflect-84 — numerals, units, model line | OFL | ships |
| Jost | reflect-84 — wordmark | OFL | ships |
| **Archivo Expanded Bold** (+ the variable file) | elmer — wordmark | **OFL 1.1** | **ships** — landed this call, `elmer/fonts/` |
| **Librestile Extended** | chorus-60 — wordmark | **SIL Open Font License** — *ocelothe2k1, 2024*, read from the delivered file's own name table | **ships**, and now on evidence rather than assertion |
| **Tudor Victors** | gatecrasher — wordmark | **© Chequered Ink 2020, All Rights Reserved** — read from the delivered file's own name table. No licence was bought, and the available licences grant use of the face to make things, not the right to redistribute the file | **ABSENT BY LICENSING.** Letterforms ship as artwork: `gatecrasher/assets/gatecrasher-wordmark.png` |
| **Impact Label Reversed** | taperot — wordmark | **donationware, not embeddable** | **ABSENT BY DECISION.** Letterforms ship as artwork: `taperot/assets/taperot-wordmark.png` |
| **Permanent Marker** | fifth-member — nameplate, tape strings | **not embeddable** | **ABSENT BY DECISION.** Letterforms baked into `fifth-member/plate/fifth-member-plate-3x.png` |

## "Licensed, embeddable" was a claim nobody could check

**Two rows carried those words identically and the two faces are not alike.** The phrase named a
conclusion without naming its evidence, and that is what kept the question invisible for nine
exports: a row asserting a licence reads exactly like a row recording one.

**Both were answered from the files themselves** (2026-08-22), by reading the `name` table of the
`.ttf` each casting delivers, which is the one piece of evidence that travels with the binary:

| Face | nameID 0 / 13 as delivered | Verdict |
|---|---|---|
| `LibrestileExtBold.ttf` | *"SIL Open Font License. Made by ocelothe2k1, 2024"* | **OFL. Embeddable, ships.** Nothing was bought and nothing needed to be |
| `TudorVictors.ttf` | *"Typeface by Chequered Ink. © 2020. All Rights Reserved"*, vendor URL `chequered.ink` | **Not redistributable.** Cannot ship in a binary at either price |

**They differ, and the answer for one was not the answer for the other** — which is the reason to
check a sibling on its own evidence rather than by inheritance from the row above it. Librestile's
outcome is unchanged; only its justification is, and that is not a small difference: it is now a
fact in the register instead of a hope.

**A licence column states where the claim comes from, from here on.** OFL and the embedded name
record are checkable; "licensed" is not. Two rows still say only "OFL" — Barlow Condensed and Share
Tech Mono, both Google Fonts and both carrying their OFL text in-file; **worth reading their name
tables the same way** rather than trusting the pattern that just failed twice.

## Gatecrasher's wordmark: artwork, not a face

**It takes TapeRot's treatment**, which this suite has already run once. Cut at **3×** with the face
**checked loaded at cut time** — `document.fonts.check("36px TudorVictors")` returned true against a
single loaded face before the raster was taken, so the letterforms are Tudor Victors and not the
`'Barlow Condensed'` fallback sitting behind it in the stack. That check is the whole point of the
procedure: a fallback cut looks like a successful cut.

| | |
|---|---|
| Path | `gatecrasher/assets/gatecrasher-wordmark.png` (and `designs/assets/gatecrasher/`) |
| Raster | **699 × 120** at 3× |
| Drawn | **233 × 40** |
| Ground | transparent — the header's own gradient shows through, so no plate colour is baked |
| Ink | `#1b1e21`, per-letter rotation ±2.4° and vertical drift ±1.2 px, all eleven glyphs as §8 sets them |

**This is a standalone cut, not a plate.** Gatecrasher has no plate to bake into — the rewrite
deleted it and the panel is entirely code-drawn — which makes it unlike Fifth Member, whose
Permanent Marker letterforms live inside `fifth-member-plate-3x.png` because there was a plate
already going out. **Same reason, two different artefacts**, and §2's "one binary ships" line on
Gatecrasher inverts: the font stops shipping and a PNG starts.

The cut is trimmed to its ink plus a stated **3 px** margin on all four sides, and placed at
**left −4.67, top +2.33** inside the header's 303 × 84 text box — the offsets that put the ink
exactly where the code-drawn glyphs were, since the rotated bbox overhung its own line box. The
descriptor line below it holds position by an explicit `margin-top: 48px`, the 8 + 38 + 2 it used
to inherit from the flow.

## The 500 weight — **CLOSED**

**Ruled 2026-08-21: 500 was intended and the file is what moves.** Deliver the same OFL
`BarlowCondensed-Medium.ttf` already cut for Fifth Member to **`gatecrasher/fonts/`** and
**`taperot/fonts/`**; both builds pick it up with one line. The §8 rows are not being changed to
600 — eight roles across two panels ask for 500 deliberately (units, scale numerals, scope
legends and header data all sit one step below their labels, and that step is the hierarchy).

**Both directories now hold it** — `gatecrasher/fonts/` and `taperot/fonts/`, delivered 2026-08-22.
No casting that asks for 500 now draws it in 600, and no substitution remains to declare.

**The delivered file, from its own tables:** `BarlowCondensed-Medium.ttf`, **680 glyphs**, `usWeightClass` **500**, 1000 upem, typo ascender/descender 1000 / −200, version **1.408**, *Barlow Project Authors* / Tribby Type, **OFL**. This is TapeRot's cut rather than Fifth Member's 694-glyph one — the same cut now sits in both castings, which is a better outcome than the ruling asked for: one file, one metric set, two directories.

**It went through three states in three days and only the last is a delivery:** ruled (the weight
stands), reached TapeRot's build directly, then reached both bundle directories. **The middle state
was the dangerous one** — from the build side a half delivery reads exactly like a whole one, and
this register briefly said both "ships in fifth-member only" and "ships in taperot" while neither
was quite true of the bundle.

**TapeRot's copy is a different cut from Fifth Member's, and that was checked rather than assumed:
680 glyphs against 694, same release.** It is identical on every metric the type scale depends on
and carries the same 525 codepoints, so it is fit for purpose. Recorded because **the ruling said
"the same OFL file" and this is not byte-identical to it** — a ruling naming a file and a delivery
satisfying it by equivalence are different things, and the second needs saying out loud or the next
reader assumes the first.

**Gatecrasher’s four roles** — unit, scale numeral, scope legend (`500/600`), scope header data —
draw at 500 from this export. Its `labelFont` substitution note comes out with the next build
change; **the prototype never substituted**, having drawn `font-weight: 500` at all four roles
throughout, so no artwork moves and there is nothing to re-check on the panel.

**`tools/check_font_sets.py` is the closure condition** — the missing arm described below, now
written. Its last complaint was:

    ** Barlow Condensed 500   asked by 3: has ['fifth-member', 'taperot'] · LACKS ['gatecrasher']

**It should now be quiet on this face.** Worth running rather than assumed — the whole point of the
arm is that nobody’s reading closed it.

### This register's own figures were wrong, and by the mechanism it exists to catch

It said gatecrasher asked in **5** rows and taperot in **2**. Counted against the tables instead of
by mentions: **gatecrasher asks in 4 §8 rows** — unit, scale numeral, scope legend (`500/600`) and
scope header data — and **taperot in 1**, the scale numeral. The extras were §3.2's restatement of
the numeral-ring role in each casting, counted twice because a grep over mentions cannot tell a role
from a reference to it. **Seven roles was five.**

**This register inherited both numbers from the ask that raised them rather than checking them**,
which is the same act it was written to prevent one level down: a figure republished is a figure
claimed. Row counts here are now counted against §8's table and §3's restatements are named as
restatements.

**The check that missed it is the finding.** A bundle delivering *fewer* files than a casting
needs passes every arm: forward finds every path CMake names, reverse finds every delivered file
read, and the set comparison diffs a directory against **its own** previous state. Nothing asks
whether two castings with the same §8 row got the same font set. **Add that arm** — group §8 rows
by (face, weight) across castings and assert the delivered sets match. **Written — `tools/check_font_sets.py`**, and it is the closure condition
above rather than a report. Seventh instance of a fix not travelling between castings.

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

Two of the six `designs/*.dc.html` panels declare a per-casting face with `url('fonts/…')` — a path
relative to the prototype, not to its casting folder. **So `designs/fonts/` carries those binaries as
well**, duplicating files that also ship under `gatecrasher/fonts/` and chorus-60's. The
duplication is deliberate: a missing `@font-face` source is silent, the browser substitutes and lays
the page out on the fallback's metrics, and every width measured afterwards belongs to a face nobody
chose. Two extra binaries are cheaper than one silent substitution.

| Prototype declares | Resolves at `designs/fonts/` |
|---|---|
| *(Gatecrasher declared `TudorVictors.ttf`)* | **no longer declared** — the prototype places the wordmark artwork instead, so nothing asks for a face that cannot ship. Export 11 |
| `LibrestileExtBold.ttf` | yes — and it may, being OFL |
| *(TapeRot declared `ImpactLabelReversed.ttf`)* | **no longer declared** — the prototype places the wordmark artwork instead, so nothing asks for a face that cannot ship. See `designs/ABSENT.md` |

**Librestile Extended's "ships" row above was accurate as an intent and wrong as a fact** in export 3:
the face was in no location in that bundle, so Chorus-60's runtime nameplate — Librestile Extended
28 / 32 — could not be checked against its own prototype. It ships at `designs/fonts/` from export 4.

**`TudorVictors.ttf` is gone from `designs/fonts/` too, and one more file had to move with it** —
`designs/Header Part - Six Materials.dc.html`, the six-casting materials study, was the only other
file declaring the face. Withdrawing a font that cannot be redistributed is not optional, so
deleting it orphaned that declaration; the study takes **the same artwork at the same offsets**
rather than being left to fall back to Barlow Condensed and quietly compare the wrong letterforms.
**A withdrawal is a two-file change on this casting, not one** — worth knowing before the next
licence answer comes back.


**TapeRot was the last measurement-unsafe artefact in the delivery and is closed in export 9.** Its
prototype declared the one face absent by decision, so the nameplate silently laid out on Barlow
Condensed — the substitution `taperot/fonts/ABSENT.md` warns about, performed by a file shipped
beside it. The nameplate now places `assets/taperot/taperot-wordmark.png` (694 × 150 delivered, drawn
231.2 × 50), which is what the build embeds. **Absence by decision no longer implies a prototype that
cannot be measured** — where the letterforms ship as artwork, the prototype uses the artwork.

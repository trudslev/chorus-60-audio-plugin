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
| **Librestile Extended** | chorus-60 — wordmark | **OFL — and the only face here whose licence cannot be settled from the file.** Name: file, nameID 0, *"SIL Open Font License. Made by ocelothe2k1, 2024"*. **Version 1.1: author's repo only** — the file carries **no nameID 13 and no nameID 14**, so it names no version, alone among these faces. **Year 2024: the file**, agreeing with that repo's `LICENSE` — while the same repo's `SIL Open Font License.txt` says **2023**. Reserved Font Name `"Librestile"` and the verbatim notice: repo `LICENSE`, read 2026-08-23 | **ships.** The 2023/2024 disagreement is deliberately left visible — **2024 governs, because that is what the redistributed file declares** |
| **Tudor Victors** | gatecrasher — wordmark | **© Chequered Ink 2020, All Rights Reserved** — read from the delivered file's own name table. No licence was bought, and the available licences grant use of the face to make things, not the right to redistribute the file | **ABSENT BY LICENSING.** Letterforms ship as artwork: `gatecrasher/assets/gatecrasher-wordmark.png` |
| **Impact Label Reversed** | taperot — wordmark | **donationware, not embeddable** | **ABSENT BY DECISION.** Letterforms ship as artwork: `taperot/assets/taperot-wordmark.png` |
| **Permanent Marker** | **elmer AND fifth-member** — nameplate, tape strings, scribble strips | **Apache 2.0** — nameID 13 *"Licensed under the Apache License, Version 2.0"*, nameID 14 `apache.org/licenses/LICENSE-2.0`, © 2010 Font Diner, Inc. Redistribution permitted outright | **SHIPS in both**, via `juce_add_binary_data`, and always has. Fifth Member's letterforms are **also** baked into `fifth-member-plate-3x.png` — both are true |
| **Special Elite** | **nothing draws it** — two roles in README prose, implemented in no artefact; one use in a **rejected** icon concept | **Apache 2.0** — © 2010 Brian J. Bonislawsky DBA Astigmatic (AOETI) | **ORPHAN. The removal was correct and stands.** This register's reopening is retracted — see below |

## Barlow Condensed: **1.408 is canonical, 1.101 is the outlier** — ruled 2026-08-24

**Chorus-60's build embeds `BarlowCondensed-SemiBold.ttf` version 1.101 (89 916 bytes); everything
else in the suite is 1.408.** The two cuts are not the same face under two numbers: 391 codepoints
against 525, **19 of 95 ASCII advances differ**, and `·` U+00B7 is **148 against 215 — +45.3 %**. All
three 1.408 copies in the tree are metrically identical, so the target is unambiguous.

**1.408 is what the designers deliver, what four castings carry, and what every prototype now uses.**
The two castings on 1.101 are on it by age, not by decision. **Every measurement taken from
Chorus-60's prototype is currently against a face its build does not have**, which is the class this
project has already been bitten by.

**Held, not cancelled.** Adopting it is one line per casting and a re-read of two panels, and it moves
labels on two shipping panels — `MODEL CH-60 · STEREO` moves **+0.830 px**, left-aligned, so its right
end takes the full amount. **Doing it inside the About round would confound each panel's
render-and-read with a change that has nothing to do with the About part.** It lands as its own change
set once the About round closes. Nothing is missing meanwhile: U+00B7 and U+2212 are the only
non-ASCII codepoints either casting draws and **both are present in 1.101**.

**And it is measured out of Gatecrasher's `REVERB TANK` residue** — build 95.5 px against prototype
89.5. The cut accounts for **0.336 px** of it and `ENVELOPE` is identical in both. Type size stays the
named next candidate.

## Two delivered faces have no licence beside them

`designs/fonts/` delivers **eight faces and four licence files**. **Archivo Expanded Bold** and
**Share Tech Mono Regular** ship without one. Both are OFL 1.1 by their own `name` tables and both are
credited as OFL in §9.3, so this is a missing file rather than a wrong claim — but this register's own
rule is that a licence claim is carried by a file in the tree, not by a row. **`ShareTechMono-OFL.txt`
and `ArchivoExpanded-OFL.txt` are owed**, and one of them is the plausible identity of change set 40's
missing 117th file.

## Who owns which column — ruled **(a)**, 2026-08-23

**The *Used by*, *Licence* and *State* columns become the build's, generated and delivered each
round. This register cites that table and does not restate it.** What stays here is what only a
designer can state: **which face a panel is designed in, what is baked and why, why a face was
chosen, and what a substitution costs.**

**The reasoning is the argument the ask makes: the columns that go wrong have a machine-checkable
source, and the columns that are valuable do not.** Ground truth for the first three is each
casting's `juce_add_binary_data` block, which is not in this bundle — so an authored copy of it here
is a transcription with no way to be checked, which is the definition of the thing this file exists
to catch.

**This is the mechanism that closed the export-stamp divergence**, and it is the second time it has
been the answer: `MANIFEST.md` reads *"Identity: see `BUNDLE.md`"* with no restated figures, and the
stamp has not diverged since. **A figure with two homes has two values eventually.** `FONTS.md` has
**seven landings** — the root and all six castings — so an authored fact here has seven chances to
drift per export.

**Until the generated table arrives, the rows below stand corrected and are the interim record.**
When it lands, they are replaced by a citation, not edited again.

## The failure mode: a retraction that does not reach the sentence it retracts

**Two delivered files disagreed with themselves this week, the same way.** `FONTS.md`'s Special Elite
row read *ships* while a section further down said *CLOSED: it was the unused embed*.
`CONTRAST-CEILING.md` §2.3 opened with *Confirmed* while §2.2, immediately above, retracted the case.
Both corrections were written; **neither was carried to the line that asserted the opposite.**

**A summary row and its body are two homes for one fact**, which this register already has a rule for
— it is why *Used by* / *Licence* / *State* moved to the build, and why `MANIFEST.md` reads
*"Identity: see `BUNDLE.md`"*. **The rule was applied to figures across files and not to a heading
three paragraphs from its own correction.**

**Practice: a retraction is not filed until the thing it retracts says so where it stands.** Strike
the row, strike the heading, then add the reasoning. A correction placed elsewhere in the same
document is invisible to a reader who stops at the row — and the row is what a reader stops at.

## The failure mode: a true clause carrying a false one

**Permanent Marker's row was false in both halves on the day it was written and survived nine
exports.** Not stale — stale means true once. It read *"not embeddable, therefore baked into the
plate, and here is the plate"*, and the plate clause **is true**: the letterforms genuinely are baked
into `fifth-member-plate-3x.png`. Both things are true at once, which the corrected row now says
explicitly.

**So the true clause supplied exactly the evidence a reader would check before doubting the false
one.** Nobody was careless; the row answered its own obvious follow-up question. That is what makes
this family expensive:

- the plate enumeration that came out **thirteen rows short**, because every row in it was ink and
  every missing one was material;
- *"Wordmark — stays baked, it is the CHORUS badge"* — true about the badge, false about the
  nameplate;
- `CLAUDE.md`'s paragraph where a true clause about JUCE carried a false one, and six castings wrote
  to the wrong directory for a day;
- and, in this suite's own documents, **three contrast exposures computed to two decimals against
  grounds nothing stood on** — the arithmetic true throughout, the pairing invented.

**The tell: you can name evidence for the clause's neighbour but not for the clause.** Worth applying
when writing a row, which is where it is cheap — the checking is the expensive end, and by then the
row already reads as coherent.

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

**The licences were not shipping at all until 2026-08-23**, and that is a separate fault from any row
above. Embedding a font in a binary is redistribution, and OFL 1.1 and Apache 2.0 both require the
notice and licence to travel with it. The `OFL.txt` files sat under `design/` — a **build-input**
directory — so **the fonts went in as bytes and the licences went nowhere.** Each casting now
generates `THIRD-PARTY-LICENCES.txt` into `Contents/Resources/`. **A licence file in a build-input
directory looks exactly like a licence file that ships**, which is why `ABOUT-PART.md` §8 can now
honestly point a reader at it.

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
already going out. **Note the difference in kind: Fifth Member bakes an Apache-2.0 face it also
embeds, for plate-export reasons; Gatecrasher bakes a face it may not embed at all. Baked and
unlicensed are independent**, and reading them as one sentence is what kept Permanent Marker's row
wrong for nine exports. **Same reason, two different artefacts**, and §2's "one binary ships" line on
Gatecrasher inverts: the font stops shipping and a PNG starts.

The cut is trimmed to its ink plus a stated **3 px** margin on all four sides, and placed at
**left −4.67, top +2.33** inside the header's 303 × 84 text box — the offsets that put the ink
exactly where the code-drawn glyphs were, since the rotated bbox overhung its own line box. The
descriptor line below it holds position by an explicit `margin-top: 48px`, the 8 + 38 + 2 it used
to inherit from the flow.

## Reflect-84's Jost and IBM Plex Mono (2026-08-23, export 14)

**§8 declared five faces; `reflect-84/fonts/` held two.** `Jost-Medium.ttf` (500) and
`IBMPlexMono-Regular.ttf` (400) delivered with their own licences, and the Barlow `OFL.txt` renamed
**`BarlowCondensed-OFL.txt`** — three families, three genuinely different licence documents (4471 /
4478 / 4455 chars), so a bare `OFL.txt` in a shared directory is a filename waiting to be overwritten.

**The prototype had been pulling both from the Google Fonts CDN.** That is the mechanism worth adding
to this register: **a CDN link renders correctly and delivers nothing**, so it fails in the most
comfortable direction — the panel is right on every machine with a network while the bundle ships two
families short. **Sweep the other five castings' prototypes for CDN-only families**; this one was
found by reading §8 against a directory listing, not by any arm.

**`reflect-84/fonts/` had no `ABSENT.md` and now has one saying the set is complete.** Six exports of
a missing marker reading as nothing-to-report is the same failure as an unchecked claim: **a directory
that makes no claim cannot be caught being wrong.** Every fonts directory in this bundle should carry
one, whether it reports an absence or its absence of absences.

## Reflect-84's 500 and 700 (2026-08-23)

**Barlow Condensed Medium and Bold delivered to `reflect-84/fonts/` with `OFL.txt`.** Fetched from
`google/fonts` `ofl/barlowcondensed/` per that casting's own provenance rule — the build owning its
font sources rather than inheriting a sibling's bundle.

**Read from the files on arrival, they are the 680-glyph cut, not the 694 the ask described — and the
Medium is byte-identical to export 12's** (97960 bytes, `27d58d15…`). So there is **no mixed cut on
this casting** and, better, **the suite now runs one cut of Barlow Condensed across five castings and
three weights**: 1000 upem, typo 1000 / −200 / 0, win 1075 / 274, cap 700 on all three, x-height the
only thing that moves. **Nobody chose that; it is worth writing down before it drifts.**

**A ruling accepting the mixture was written and is retracted** — it reasoned carefully about a state
that did not exist, from a figure this register took from the ask and did not read off the files. Same
mechanism as the row counts in the section below, one export later.

**One thing could not be reproduced:** the ask's byte-equality of the Bold against Fifth Member's
delivered copy. `fifth-member/fonts/` holds only `ABSENT.md` — **no Barlow Condensed at any weight is
in that directory**, which is its own question, and the fifth-member column of the 500-weight table
below rests on the same absent evidence. Carried on the build's word and marked as such.

### And the arm written in export 12 passed this casting twice

`tools/check_font_sets.py` reported reflect-84 clean while it was two weights short, for two
unrelated reasons — **both of which are properties of checkers generally and not of this casting:**

1. **It matched weights by substring.** `"bold" in "semibold"` is true, so a SemiBold satisfied a
   Bold ask; **the same collision sits under ExtraBold/Bold and ExtraLight/Light.** Fixed to
   longest-match-then-equality. Every other name-matched check in this suite is worth reading for
   the same shape.
2. **It reads §8's type table, and the 500 was only in §2.2's prose.** §8's ALGORITHM row is now
   split — caption 600, corners 500 / 700 — so the requirement sits where the tool looks. **A
   checker that reads one table makes that table the contract**; anything stated only in prose is
   outside every arm this suite has, which is worth a sweep rather than a fix.

**Export 12's closing line was that a checked set had closed the last font debt. It had not** — a
third casting was two weights short at the time, and the arm said otherwise. The arm is better now;
what it is not is finished.

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

## The prototypes' font links over-declare, on all six — and that is where strays come from

**A `fonts.googleapis.com` link is a weight declaration that nobody owns.** Every casting requested
`Barlow+Condensed:wght@400;500;600;700` while drawing two or three of those weights, and Elmer also
still requested **`Archivo Black`** — the face *Elmer's wordmark: Archivo, not Archivo Black* below
corrected away from. The correction landed in the spec and in the drawing, and the link kept asking.

**Trimmed to what each casting draws** (root prototypes and `designs/`):

| Casting | Was | Now |
|---|---|---|
| chorus-60 | `@400;500;600;700` | `@600;700` |
| elmer | `@400;500;600;700` + `Archivo Black` | `@500;600`, Archivo Black dropped |
| gatecrasher | `@400;500;600;700` | `@500;600` |
| taperot | `@400;500;600;700` | `@500;600` |
| fifth-member | `@400;500;600;700` | `@500;600` |
| reflect-84 | `@500;600;700` | **unchanged** — it sets weight through template holes, so a source scan cannot enumerate it; §2.2's selected corner label is a live 700 |

**This is the likeliest source of the embedded strays** — Gatecrasher's Bold and Regular against a §8
asking only 500 and 600. An over-declaring link is a plausible-looking list of weights that no role
requires, sitting in the artefact a build reads for guidance. **It over-declares silently and in the
direction that makes a stray look intended.**

**Elmer's Barlow 700 was a stray too, and only the DOM said so.** The source shows a 700 site; the
rendered panel reports `Barlow Condensed:700` **unloaded** with exactly one element at weight 700 —
the wordmark, in **Archivo** 700. Trimmed to `@500;600`. **Reflect-84 is left alone for the mirror
reason:** its weights arrive through holes, and the same scan that found Elmer's stray reported
Reflect-84 as not drawing 700 when §2.2 demonstrably does.

**Both halves of that are the same lesson at different ends: a source scan under-counts what holes
supply and over-counts what a link merely requests.** Neither number is the drawing.

*(One flagged and withdrawn before reporting: a DOM sweep of Elmer reported eight elements rendering
in Times. They are its `<style>` and `<script>` nodes — the query counted leaf nodes with text
content and those qualify. Noted because the previous three retractions in this project all reached a
document first.)*

## Fifth Member's Permanent Marker sites: **four, and all four bake**

**Measured in the rendered DOM 2026-08-23**, by walking each marker element up to its nearest
`[data-plate]` ancestor rather than by proximity in the source:

| String | Size | Plate flag on the nearest ancestor |
|---|---|---|
| `FIFTH MEMBER` | 27 px | **none — renders into the plate** |
| `CH 4 — GTR / STAGE LEFT` | 15 px | **none — renders into the plate** |
| `DLY 4` | 12 px | **none — renders into the plate** |
| `HALDEN HALL · LOAD-IN 06` | 14 px | **none — renders into the plate** |

**Neither of the two readings offered below was right, and neither was the build's first count.**
Settled 2026-08-23: `Font::marker` is **one definition inside a tape lambda called twice**, drawing
`DLY 4` at 12 px and `HALDEN HALL · LOAD-IN 06` at 14 px — exactly two of the four above, at exactly
those sizes.

**They are drawn because the plate cannot reach them.** The plate cut is 1340 × 1012 blitted at
`frameX = 52`, spanning canvas x **52…1392**; both tape boxes sit at **1396…1440**, on the right ear,
`earWidth = 52` either side of a 1444 canvas. **The plate stops 4 px short of them.** So no
double-print is possible and no role is missing.

**Final reconciliation: four bake in the prototype's continuous canvas; two of those fall outside the
delivered plate cut and must be drawn; two fall inside and correctly are not.**

**The error in this file's own reading was a missing premise, not a missing measurement.** It said
*baked and drawn-on-top look identical from the design side* — true **only if the plate reaches the
element**, and the plate's extent was never checked against the elements' coordinates. A conditional
stated as a symmetry. The DOM ancestry measurement was correct and the count of four was correct;
what was wrong was treating “renders into the plate” as “is contained by the plate”.

**That leaves one of two things, and the build side can tell which:**

- the runtime consumer draws a string the plate already carries, in which case it is **drawn twice**
  and the plate's copy is underneath it — invisible while the two agree, and a mismatch the moment
  either moves;
- or it draws a fifth string the prototype does not have, in which case the **design** is short a
  role.

**“Baked” and “missing” look identical from the build side, and “baked” and “drawn on top of the
bake” look identical from the design side.** Neither side can close this alone, which is why the
count disagreeing 4-to-1 is more useful than either number was.

**The first reconciliation offered — “the plate bakes the other three” — was the natural one and it
was wrong by one.** It is the tell again: the baked clause was true, and being true it made the
arithmetic look finished at three.

## Special Elite — **ORPHAN after all. This register's reopening is RETRACTED**

**The original removal was correct. The reopening below was wrong, and it caused a face to be restored
to five binaries and a waiver to be added to a working tool.** Three checks would have prevented it,
all available in the file the reopening was reading:

1. **`IconStencil.dc.html` is a rejected concept.** The README's own artefact table says
   *"Rejected icon concept 1d (rack-ear stencil). Context only."* The chosen icon is **1b**,
   `IconPulse.dc.html`, the phosphor repeat train. **The reopening cited a rejected design as a live
   consumer**, with the row saying so in the same document.
2. **Icons never carry a font dependency at all.** The README: *"Outline this type on export — never
   ship the icon with a live font dependency."* Deliverables are an SVG with outlined type and PNGs.
   So even the chosen icon needs no embedded face.
3. **The two panel roles are in README prose and in no artefact.** Not in the original
   `Fifth Member.dc.html` (zero Special Elite sites), not in its `GUI-SPEC.md` (zero), not in
   `BUILD-HANDOFF.md` (zero), not in the delivered prototype, not in the build. **Six artefacts
   including the casting's own specification, none of which implements it.**

**So nothing draws Special Elite anywhere, and nothing ever did.** ORPHAN is the correct verdict on
the correct evidence, and the tool's waiver should come out with the face.

### The error, because it is the eighth instance and the first one to move a binary the wrong way

**A README is not a specification, and presence in prose is not implementation.** The reopening found
two role descriptions and treated them as the spec — while `GUI-SPEC.md`, which *is* the spec, does
not mention the face. That is this project's own rule inverted: it had already written that **absence
from a document that does not cover the thing is not evidence**, and then read **presence in a
document that is not authoritative** as proof.

**And it was made one turn after writing the practice note about retractions**, which is the part
worth keeping. The tell held exactly as documented — true clauses, false conclusion — and knowing the
tell by name did not catch it. **What would have caught it is the mechanical step: before asserting a
role exists, open the artefact that would implement it.** The reopening asserted from three quotations
and opened nothing.

### What is genuinely open, and it is small

**The README describes two panel elements that no artefact has ever implemented** — the right-ear
stencil (11 px / .22 em / `rgba(38,35,31,.55)`) and the recessed foot-label window in Special Elite
(11 px / .10 em / `#a09883`, `CH 4 — GTR / STAGE LEFT`, where the panel draws Permanent Marker 15 px).

**That is a design question, not a licensing or build one:** either the prose is stale and should be
struck, or two elements were specified and never built. **Neither answer embeds a font today**, because
until something draws them the face has no reader. If they are wanted, the face returns with them and
§8 gains two rows at the same time.

### The reopening, kept for the record

**— retracted, see above —** Found 2026-08-23, after the removal shipped, in
`uploads/Fifth Member Audio Plugin/README.md`, **which is a handoff README rather than the casting's
specification**; Special Elite has **two specified roles**, and a third use in the
plugin icon:

| Role | Specification |
|---|---|
| **Right-ear stencil** | Special Elite **11 px**, letter-spacing **.22 em**, `rgba(38,35,31,.55)` |
| **Recessed foot-label window** | Special Elite **11 px**, letter-spacing **.10 em**, `#a09883`, text `CH 4 — GTR / STAGE LEFT` |
| `IconStencil.dc.html` | Special Elite **76 px**, `#191713` at .86 opacity |

That README also states the casting's typographic rule outright: *"anything applied by a human with a
marker or a stencil is in Permanent Marker or Special Elite. Three voices, no exceptions."​* **Special
Elite is one of Fifth Member's three voices, not a stray file.**

### The evidence that closed it was about a different element

The closure rested on §1's **RACK 4 ear mark** being `Font::label(11.0f)` at tracking **3.74f**,
matching the prototype's `font-weight: 600; font-size: 11px; letter-spacing: .34em` with no
`font-family` — 0.34 × 11 = 3.74 exactly. **That arithmetic is correct and it proves the rotated ear
text is Barlow.**

**But the README specifies two separate elements on the ears**, one sentence apart:

> *Rotated text on ears: 11 px Barlow Condensed 600, letter-spacing **.34em**, rotated −90°.*
> … *Right ear stencil: Special Elite 11 px, letter-spacing **.22em**.*

**Same ear, same size, different tracking, different face.** The 3.74 match confirmed the neighbour
and was read as ruling on the stencil. **Seventh instance of the tell, and the first to cost a
deletion rather than a wrong row** — and the exactness of the match is what made it conclusive.

### The prototype is short both roles, which is why the build looked clean

The delivered prototype has **zero** Special Elite sites. It has the `.34em` rotated ear text
(`RACK 4 · MON WORLD`, Barlow, correct) and no ear stencil. **So build and design agreed — both were
missing the same two roles**, and agreement between two artefacts that lost the same thing is
indistinguishable from correctness. **`check_font_sets.py` reading from the binary cannot find this
either:** the face had no reader because nothing draws it in either tree.

**And one prototype string is in the wrong voice.** `CH 4 — GTR / STAGE LEFT` is drawn in **Permanent
Marker 15 px**, where the README puts it in **Special Elite 11 px / .10 em / `#a09883`** inside the
recessed window. That also revises the marker count: **one of Fifth Member's four Permanent Marker
sites is a Special Elite site wearing the wrong face.**

### What was asked — **withdrawn**

1. ~~**Do not delete the face.** Revert the removal.~~ **Wrong.** Re-remove it; the waiver comes out
   with it. And the count in this line was wrong twice over — six was stated against a tree that had
   already dropped Barlow Bold on a separate ruling, so the correct figure was five either way.
2. **The two roles are design work and are not being re-cut unasked.** The prototype needs the ear
   stencil restored and the foot-label window re-voiced; both are visible changes to a delivered
   panel and want a decision, not a silent edit.
3. **`fifth-member/GUI-SPEC.md` §8 needs both rows** — they are absent, which is why every arm on
   both sides read the face as unwanted.

### What the original closure got right

`Font::stencil` genuinely had zero consumers, and the RACK 4 mark genuinely is Barlow. **The build was
short a role, exactly as `Font::labelMedium` is** — the same WIRE case, one classification apart, and
the tool's own two headings had the right pair of names for it all along.

**Settled from the artefact.** `Font::stencil` had zero consumers and its only caller was itself.
The role its name implied — §1's RACK 4 ear mark — is drawn with `Font::label(11.0f)` at tracking
**3.74f**, and the delivered prototype sets that element `font-weight: 600; font-size: 11px;
letter-spacing: .34em` with **no `font-family`**, so it inherits the panel's Barlow. **0.34 em × 11 px
= 3.74 px exactly**, so build and design already agreed and the face was never the one that mark is
set in.

**Removed** from `juce_add_binary_data`, the theme, BinaryData, the licence file and the shipped
bundle; Fifth Member now embeds five faces, and the licence file is 19,767 bytes with zero occurrences
of the name, byte-identical in the installed `.vst3`. A comment stands where the builder was.

**And the name was not misleading after all.** `Font::stencil` was named for the **ear stencil**, a
role the README specifies and both trees had lost — so the identifier was the last surviving record of
it. Read as a calibration-constant coincidence, it was in fact the correct name for a missing element;
**the name outlived the thing it named, and was deleted for looking like a mistake.**

**Two more with no consumer, reported and not acted on: `Font::labelMedium` and `Font::labelBold`.**
Both at zero in Fifth Member while Barlow Medium and Bold are embedded — **the opposite case**, since
§8 asks for Medium, so the build is short a role rather than carrying a stray. Corroborated from this
side: the prototype has **one** `font-weight: 500` site (the model line, 12 px / .30 em) and **zero**
at 700. **So Medium has one design consumer and no build consumer, and Bold has neither.** Bold is a
candidate for removal on the Special Elite reasoning; Medium is a missing wire, not a stray file.

### The original query, for the record

**Verified on this side:** `Permanent Marker` is declared in **two** prototypes — Elmer once, at
28.5 px for `CH 24 — MIX BUS / GLUE`, and Fifth Member four times including the `FIFTH MEMBER`
nameplate at 27 px. That corroborates the correction from the drawings as well as from
`PanelBackground.cpp`.

**`Special Elite` is declared by none of the six.** It is embedded in fifth-member's binary and no
delivered prototype names it. Two possibilities and they want different answers:

- **the build draws something the prototype does not** — then the prototype is short a role and
  `fifth-member/GUI-SPEC.md` §8 needs the row;
- **it is an unused embed** — then it is bytes in a shipped binary for nothing, and its Apache notice
  is a licence obligation carried for no benefit.

**This is the inverse of every font question so far** — all seven prior ones were a casting asking
for a face it did not have. A face present and unasked-for is new, and the arm that finds it is not
the one that finds the others: **`check_font_sets.py` compares §8's rows against delivered files and
would report this as clean, because §8 has no row to be short.**

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

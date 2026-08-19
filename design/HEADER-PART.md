# NEON FOUNDRY — THE HEADER PART

**Suite-wide contract. One part, six castings.** This file lives in `shared/` and is never copied
into a casting folder — a figure duplicated six times disagrees with itself within a round.

**Revision 4** · parts catalogue checkpoint 3 · every figure below is either **measured** off the
rendered part (`Header Part — Six Materials`, 1×) or **computed** from a stated law, and says which.

**Asset format: nothing in the header is baked.** Every element here is drawn at runtime — the LCD
strings, the four Program-button legends, the two meter values, the captions, the block and the
wells. There is no header bitmap in any casting. The nameplate block is the exception and is
per-casting: see §8.

---

## 1 · What is shared and what is not

**Shared, and identical in all six:** canvas width, the header block's box, the nameplate zone's
box, the caption row's baseline, the descriptor line's y, the five-part band — LCD, SAVE, DELETE,
IN, OUT — with its height, widths, gaps and baseline, the Program-button construction and state
matrix, the LCD's face, size, tracking, character budget and user-name cap, and the dropdown
construction.

**Not shared:** fascia material and colour, the header block's own material, ink, phosphor,
nameplate metaphor, the mono face used for the model line and printed scale numerals, and the
accent colour. Canvas *height* is per casting.

**If harmonising something would make two panels indistinguishable at a glance, it is the wrong
thing to harmonise.** The band is the part; the material around it is the casting.

---

## 2 · Geometry

Canvas width **1340** in every casting. All coordinates are canvas-local, origin top-left.

| Element | x | y | w | h | Notes |
|---|---|---|---|---|---|
| Header block | 16 | 16 | 1308 | 104 | Border-box. **Inset ring, not a border — see §3** |
| Nameplate zone | 38 | 30 | 303 | 84 | Wordmark + descriptor + model line. **84 is measured**; the stack runs 79–81 across the six metaphors |
| Function descriptor | 38 | **78** | — | 17 | **The anchor line — see §4** |
| Model line | 38 | 95 | — | 14 | Casting's own mono face |
| PROGRAM caption | 357 | 41 | — | 13 | `NAME PROGRAM` during name entry |
| LCD | 357 | 61 | 641 | 34 | **Inset ring, not a border — see §3** |
| SAVE / STORE | 1006 | 61 | 62 | 34 | 8 px gap from the LCD |
| DELETE / CANCEL | 1076 | 61 | 70 | 34 | 8 px gap from SAVE |
| IN caption | 1164 | 41 | 64 | 13 | Centred on the well |
| IN well | 1164 | 61 | 64 | 34 | **18 px gap from DELETE** — wider than the meters' own 10, so they read as a pair |
| OUT caption | 1238 | 41 | 64 | 13 | Centred on the well |
| OUT well | 1238 | 61 | 64 | 34 | Right edge **1302** = 1324 − 22, closing the block's own left padding |
| Body origin | 16 | **120** | 1308 | varies | 16 + 104 = 120, flush to the block's bottom edge |

**Band height is 34 in every casting.** Not a proportion of the panel — a fixed figure, because the
castings are differently-sized units rather than scales of one design. 34 is set by the Program
buttons: two 11 px legends with leading and padding need about 27, and a height that only just fits
does not read.

**The meter wells are 64 and the row ends at 1302.** An earlier revision had them 76 wide ending
flush at 1324, which gave the block 22 px of left padding and none on the right. 64 holds a
four-character meter value — 43.52 px of glyph run at the LCD face, measured — inside 16 px of
padding, and it is even, so 0.5× lands on whole pixels.

---

## 3 · The two inset rings, and what happens if they become borders

**Instruction: the header block's edge and the LCD's edge are inset rings drawn inside the box, not
borders added to it.** In CSS terms an inner stroke on the padding box; in JUCE, draw the outline
one pixel inside `getLocalBounds()` and lay the contents out against the **full** box.

This is a CSS construct with two valid translations into JUCE, and **the wrong one is silent** — the
panel looks correct and two things are quietly wrong:

- **The block.** A 1 px border adds to the outside of a 1308 × 104 border-box, so every element in
  §2's table lands **one pixel right and one pixel low** of its stated coordinate. Nothing looks
  broken; the band simply no longer sits where the spec says, and the body origin at 120 no longer
  meets the block's bottom edge.
- **The LCD.** A 1 px border on each side reduces the name area from **538.00** to **536.00** px.

**What that costs, measured rather than asserted.** 49 glyphs and 48 gaps need
`49 × 9.180 + 48 × 1.700 = 531.42` px. Against 538.00 that leaves **6.58 px spare**; against 536.00
it leaves **4.58**. So a border pair does *not* by itself drop the budget — the count survives at
49 — and any spec claiming it costs two characters does not reproduce against these numbers. The
catalogue's §3 says the same thing in the same terms; if the two ever disagree, this file is the
contract.

**The real consequence is that there is less than one character of headroom either way.** 50 glyphs
need 542.30, which is 4.30 px more than the cell has. Every construct that quietly eats a pixel —
a border pair here, a rounded padding there, a chevron trim measured to the glyph instead of the
box — spends part of a 6.58 px margin that has no room for a second mistake. Hold the ring
construction because the budget has no slack to absorb the alternative, not because the alternative
costs exactly two characters.

---

## 4 · The descriptor line is the anchor, not the wordmark

**All six function descriptors sit on the same y: 78 canvas.** The wordmark above it does not align
across the six and must not be made to — each nameplate metaphor is a different physical object and
a different height. A label-maker strip, a spray stencil, a silkscreen with a rule under it, an
engraved plate, a length of gaffer tape and a moulded relief are 38 to 46 px tall as artwork.

**So the leading beneath each wordmark is tuned per casting to land the descriptor on 78.** That is
the rule, and it is the one thing in this file a seventh casting could not derive from the
coordinate table.

| Casting | Top offset in zone | Nameplate box | Leading under it | Stack from 30 | Descriptor y | Artwork box |
|---|---|---|---|---|---|---|
| TapeRot | **0**, measured | **44** = 2 + 40 line box + 2, the plate's padding | 4 | 30 + 44 + 4 | **78** | **50** × 231.2 at −1.5° |
| Gatecrasher | **8**, measured | 38 stencil line box | 2 | 30 + 8 + 38 + 2 | **78** | 46 |
| Chorus-60 | **0**, measured | 42 = 32 + 5 rule + 5 | 6 | 30 + 42 + 6 | **78** | 42 |
| Reflect-84 | **0**, measured | 40 engraved plate | **8** | 30 + 40 + 8 | **78** | 40 |
| Fifth Member | **0**, measured | **40** = 5 + 28 line box + 7, the tape's padding | **8** | 30 + 40 + 8 | **78** | **45.6** × 268 at −1.2° |
| Elmer | **0**, measured | 39 relief plinth | 9 | 30 + 39 + 9 | **78** | 39 |

**Every row now closes on 78, and three figures changed to make that true — none of them an offset.**
The offsets are genuinely zero and are now stated as measured zeros, because **a measured zero and an
assumed zero read identically in a table.** Gatecrasher's 8 was always real. What was wrong:

- **TapeRot 38 → 44.** The old figure was the wordmark's line box; the nameplate is a Dymo plate with
  2 px of padding above and below the type. Its stated leading of 4 was right all along.
- **Fifth Member 34 → 40**, and **leading 9 → 8.** Same defect plus a leading off by one: the tape
  carries 5 px above and 7 px below its 28 px line box.
- **Reflect-84 leading 9 → 8.** Its height of 40 was correct; only the gap was off by one.

**One cause, and it is why the three misses had no arithmetic in common: the heights were taken from
the type rather than from the object.** A glyph line box is not a nameplate — the two rows that missed
hardest are the two whose metaphors carry padding, and Gatecrasher's `+ 8` is the same class of thing
noticed once. **The two off-by-one leadings were read off a render as a visual gap** rather than
computed from the box.

**The last column is a different height and has to be labelled as one.** TapeRot's plate and Fifth
Member's tape are rotated, so their **artwork** box is taller than their **layout** box — 50 against 44,
45.6 against 40. §4's sentence about metaphors running 38 to 46 px tall describes the artwork box; the
anchor stack uses the layout box. **A column of heights that mixes the two cannot close, whatever the
figures are.**

**The stack is a check, not a generator, and every panel already treats it that way.** All six pin the
descriptor absolutely — `top: 62` inside the block, canvas 78 — so no leading has ever produced the
anchor in any casting. That is why three rows could sit wrong for two revisions without a panel
looking wrong: **nothing read them.** Asserting them as a check is correct, and the build's
`landsOnDescriptorAnchor(nameplateTop, height, leading)` is the right shape for this table — the three
rows above should now pass it rather than being pinned at their misses.

A seventh casting's instruction is therefore: draw the nameplate however the metaphor requires
inside the 303 × 84 zone, then set the leading so the descriptor's line box starts at 78. If a
metaphor cannot fit the descriptor on 78 inside the zone, the metaphor is too tall — the zone and
the anchor do not move.

---

## 5 · The LCD

**Face: Share Tech Mono, 17 px em, tracking 1.700 px (.10 em).** Shared across all six, per catalogue
call 2. This is the em size, meaning what a CSS `font-size` means — `withPointHeight()` in JUCE 8,
never a calibrated ratio onto `withHeight()`.

| Figure | Value | Provenance |
|---|---|---|
| Cell | 641 × 34 | Shared, §2 |
| Bank cell (`FACT` / `USER` / `NAME`) | **72.00** | Measured |
| Divider | 1.00 | Measured |
| Chevron trim | **30.00** | Measured |
| **Name area** | **538.00** | **Measured** — 641 − 72 − 1 − 30 |
| Glyph advance | 9.180 | Measured off the face at 17 px |
| Tracking | 1.700 | Stated, .10 em |
| Per character | 10.880 | Computed |
| Run for 49 glyphs + 48 gaps | 531.42 | Computed — fits, 6.58 spare |
| Run for 50 | 542.30 | Computed — 4.30 over |
| **Budget** | **49 characters** | Measured by exact fit |
| **User-name cap** | **47 characters** | Computed — 49 − max(dirty marker 2, caret 1.47) |

**The naming caret is drawn, not a glyph.** It was `▮` — a character the LCD face does not carry, so
every casting rendered it at whatever the fallback font gave: short of the type's own height and
narrower than a character cell, which read as a stray mark rather than a cursor. It is now a
**14 × 17 block with a 2 px left margin**, filled `currentColor` with a `0 0 6px` bloom of the same,
so it inherits each casting's readout ink and needs no per-casting value.

- **17 px is the type size**, so the caret is exactly as tall as the glyphs it sits beside.
- **14 px is the glyph advance (9.18) plus tracking (1.7) plus 3**, i.e. wider than a character cell
  rather than narrower — the failure being corrected.
- **Budget unaffected.** 16 px with its margin is **1.47 characters**, and the cap is set by
  `max(dirty marker 2, caret)`, which the 2-character dirty marker still wins. **47 stands.** A caret
  wider than 21.76 px would take the cap to 46 and orphan saved names.

**The cap rises everywhere and falls nowhere.** Previous caps: Reflect-84 39, Chorus-60 31,
Fifth Member 26, Gatecrasher 27, TapeRot 25, Elmer 22. The floor for this round was 39 — the
highest current cap — because lowering a cap orphans names already saved: they load, then fail to
save back under their own name. 47 clears it by eight.

**The chevron's inset, and why the budget did not move.** The chevron sat about 6 px off the LCD's
inner right edge, against the 14–16 px insets the same cell uses for its text — the tightest
clearance in the part. Raising it to a 16 px box inset (14.1 px to the rotated ink, since the glyph
is a rotated square whose ink reaches past its box) needs a wider trim, and the trim is a term in the
budget: every pixel taken comes out of the name area at 10.880 px per character.

**So the pixels came from the bank cell instead of from the name area.** `FACT` / `USER` / `NAME`
are four characters — a 41.82 px glyph run — so a 77 px cell carried 17.6 px of padding per side
against a cell whose own text insets are 14–16. At **72** the padding is 15.09 per side, in the same
family as every other inset in the part, and the 5 px funds the trim: **72 + 1 + 30 = 103**, so the
name area stays **538.00**, the budget stays **49** and the cap stays **47**.

The alternative was to widen the trim and accept 48 / 46 — legal, since 46 still clears the floor of
39 by seven. It was not taken because the cap is the figure that was approved and this was a
clearance problem with a cheaper source of pixels. **The bank cell is now spent as reserve, though:**
it has about 2 px of padding left before it reaches the 14 px inset, so the next request for glyph
room in this cell does come out of the budget.

**A correction, attached because a figure with its own correction is the one that survives review.**
An earlier pass stated the budget as **48**. It measured the name area at 533.00 — a bank cell of 77
against this revision's 30 px chevron trim, a pairing no revision ever shipped — and 533.00 still
holds 49 by 1.58 px, so the 48 was conservative rather than measured. Against the current pair, bank
cell **72** and trim **30**, the measured area is **538.00** and the measured count is **49**. The
correction was to the area; the count moved because the count was never measured the first time.

**Behaviour.** The LCD doubles as the live value readout: a control being moved shows its value
here in this face, reverting to the Program name shortly after release — direct user manipulation
only, never host automation. An edited Program shows a trailing ` *` in the same type and colour as
the name; the marker and SAVE's live state read the same flag, so they cannot disagree. The bank tag
reads `NAME` while a name is being typed, not `USER` — the Program is not in the user bank until the
name is committed. Factory display names are stored upper-cased, so case has exactly one source.

---

## 6 · The Program buttons

**Two legends each, permanently printed, on a dark cap. The legend itself lights.** SAVE above
STORE, DELETE above CANCEL — resting function on top, what the button becomes during naming beneath
it.

- Cap: `#23282C` → `#14181B`, radius 4, inset ring `rgba(255,255,255,.10)` top and
  `rgba(0,0,0,.55)` bottom, drop `0 1px 2px rgba(0,0,0,.45)`.
- Legends: **11 px / line box 13 / .12 em**, Barlow Condensed 600, both permanently printed. Neither
  moves, changes weight or changes size — the backlight is the only variable.
- Lit: `#F4F8FA` with a 7 px bloom. Idle: `#9AA1A6`, flat and matte, no bloom.
- **Measured against the cap's two stops:** lit 15.9:1 on `#23282C` and 13.1:1 on `#14181B`; idle
  5.2:1 and 4.3:1. Both clear the 3:1 state floor at both ends.
- **The cap is dark on every fascia, whatever the fascia is.** Forced by the mechanism: the live
  legend is the bright one, and a pale field gives it nowhere brighter to go.
- **No disabled face.** Cap, ring and highlight are identical in all five states. Both legends
  stepped back reads as "nothing to do here", never as a blank button. No lamp, lens or bezel is
  drawn anywhere on these buttons — the glow is the legend glowing.

| Panel state | SAVE | STORE | DELETE | CANCEL |
|---|---|---|---|---|
| Factory Program, unmodified | idle | idle | idle | idle |
| Factory Program, edited | **lit** | idle | idle | idle |
| User Program, unmodified | idle | idle | **lit** | idle |
| User Program, edited | **lit** | idle | **lit** | idle |
| Naming a Program | idle | **lit** | idle | **lit** |

**Escape out of naming leaves the Program still edited, because nothing was stored.** SAVE always
creates a *new* named Program and never overwrites, even with a User Program loaded. DELETE works
only on User Programs.

---

## 7 · Meter wells and captions

Wells 64 × 34, dark glass in the casting's own LCD material, value in the shared LCD face at
**17 px / line box 22 / .10 em**, centred — the same figure §5 states for the LCD itself, stated here
because the wells are the shared part's rather than any casting's. One convention per role: the LCD
and both wells are the same face at the same size in the same header, so a prototype drawing the
wells with tracking unset differs from the part and the part governs. Captions `IN` / `OUT` at 10 px / line box 13 / .28 em, Barlow Condensed 600, centred on
their wells at y 41. `PROGRAM` at the same size and weight at .24 em, left-aligned to the LCD at
x 357.

---

## 8 · Type, and the one thing that is per casting

**Panel lettering is Barlow Condensed across all six** — control labels, section captions, function
descriptors, Program-button legends (catalogue call 7). **The LCD face is Share Tech Mono across all
six** (call 2). **The casting's own mono face stays its own** for the model line and printed scale
numerals: those are ink on the fascia rather than parts of the header, they carry no shared budget,
and they are where a casting's character reads.

**State the metric convention with every size.** Every figure in this file is a CSS pixel em size
with its line box stated beside it, measured off the rendered part. "11 px" alone is ambiguous
between a CSS pixel and a JUCE font height and the two differ by about 11 % on these faces.

**Fonts ship as binaries under `fonts/`**, per the delivery convention. **Two castings carry a
licensing exception to that, and each states it in its own `GUI-SPEC.md` with the artwork path** —
TapeRot's wordmark face and Fifth Member's marker face are not distributable, so those wordmarks
ship as artwork and the font binaries are deliberately absent. An absent font that is not declared
looks like a delivery defect and gets "fixed" by substituting a face, which moves every measurement
taken from the nameplate.

---

## 9 · Changelog

**Revision 4.** §12 added — the Program list's 553 inset stated as a contract rule with its
derivation, the list foot landing on the chassis inset. §4's nameplate table rewritten with six
measured rows, closing the three that sat wrong: TapeRot height 38 → 44, Fifth Member 34 → 40 with
leading 9 → 8, Reflect-84 leading 9 → 8. No figure in §§1–3 or §§5–8 moved.

**Revision 3.** §11 added — the type-adoption gate, with the LCD-face register and the Elmer cap
rise 22 → 47 measured against a shipping face. Disposition cited as `design/CALL-3-DISPOSITION.md`
§3. No figure moved.

**Revision 2.** The chevron trim 25 → 30 and the bank cell 77 → 72, so the glyph gains a 16 px box
inset matching the cell's text and the name area, budget and cap are unchanged at 538.00 / 49 / 47.

**Revision 1.** First issue as a shared contract. Contains: the 1340 canvas width; the 1308 × 104
block; the 34 band with LCD 641, buttons 62 and 70, wells 64, row ending 1302; the 303 × 84
nameplate zone; the descriptor anchor at y 78 with per-casting leading; the shared LCD face at
17 px / 1.700 tracking with name area 538.00, budget 49 and cap 47; the Program-button construction,
five-state matrix and measured contrast; the inset-ring instruction with its consequences.

Corrected in this revision against the checkpoint documents: meter wells 76 → 64 with the row
ending 1324 → 1302; nameplate zone 76 → 80 → 84; LCD budget 48 → 49 and cap 46 → 47 on the measured
538.00 name area; body origin 119 → 120.

---

## 10 · How a change to this part reaches six bodies

**Revision 3, item 4 — landed ahead of the other three.** The clamp, the format, the button
sizing rule and the chevron glyph all wait on build answers. This one does not: it is a process
question rather than a figure, and it will still be true after the handoff ships.

### The defect class, argued from three instances rather than one

Every figure in this file is held **seven times** — once here, once in each of the six panels,
and again in `Header Part — Six Materials`. Nothing carries a change from one copy to the
others. Three instances in a single round, all the same mechanism:

| Change | Landed in | Did not reach |
|---|---|---|
| **The chevron glyph** | one casting, which had thought about it | five castings and all six cells of the strip — **eleven sites**, and six shipping builds behind them, drawing a 9 × 9 rotated box at 84.5° against the drawn path's 77°. **Published as "nine": listed from memory, not enumerated — see the enumeration's own defect below** |
| **The model-line ink** | the strip for four castings, the bodies for two | the other artefact in every case; and the strip's own TapeRot value was **6.03** against the block that actually ships, so a copy was not merely stale but wrong |
| **The 1340 canvas** | the panels | **both exported plates** — Chorus-60's is 2× of the superseded 1282 × 776, Fifth Member's is 3× of the superseded 1240 × 931. Both were *correct ratios* of a canvas the round had already changed, and one of them was on the completed list |

A process item argued from one instance reads as a tidy-up. Three, in one round, in three
different asset classes — markup, ink, exported bitmap — is a defect class.

### What does not fix it

**An authority table.** Naming which artefact wins per class of figure — geometry to this part,
material and ink and strings to the casting — settles an argument and changes nothing about
propagation. Seven hand-populated copies drift whatever the table says. That is the lesson the
code side already paid for: the answer to six copies was never a rule about which copy wins.

### What does

**One implementation, six materials.** The header is extracted to a single component that every
panel mounts, taking its material, ink, strings and phosphor as props — the casting's own,
per §1 — while the geometry, the band, the LCD cell, the Program buttons and the meter wells
exist once. A change to the part is then one edit, and it is not possible for a body to miss it.
`Header Part — Six Materials` becomes six mounts of that component rather than a
hand-populated illustration, which is also what stops it drifting into a wrong model number.

**Until that exists, a change to this part is not done when this file changes.** It is done
when every dependant carries it — and the revision **enumerates them by name with a state**
each: *applied*, *not yet*, or *does not apply*. A list of six entries reads as complete, which
is how the chevron reached one casting while eleven sites went unexamined. Same shape as each
casting’s conformance section: a call in neither list is a gap by construction, and here it is a
**site in neither state**.

**`nf::HeaderPart` landed 2026-08-16, so most of that interim rule has expired** — and the enumeration
it produced was under-populated in the way the section is about. Both facts are recorded below before
the table is re-derived, because **the second is the argument for the first.**

### The enumeration's own defect, stated rather than quietly corrected

**Row 4 read `Reflect-84 RF-84 Panel · chevron glyph: applied`. True of the design prototype, false of
the shipping build**, which drew the old 9 × 9 box rotated 45° at two sites until 2026-08-16 — under a
comment calling the up chevron the caret rotated, a construction Reflect-84's own §10 item 12 had
already retired as describing something that existed on neither side.

**The eight dependants were six prototypes, the strip and the artwork. No code repository was among
them.** §9 says nothing in the header is baked, so the chevron is code in all six castings and there
was nowhere else it could live. **The rule "done when every dependant carries it" was therefore
satisfiable in full while six builds drew the old glyph** — the failure the section exists to prevent,
occurring inside the section's own enumeration.

**And the count carries the same tell independently: "five castings and all six cells of the strip —
nine sites" is eleven.** Answering the cheap question the way the nameplate offsets were answered:
**the nine was listed from what was known at the time, not enumerated by search.** Both defects are one
defect — a number written from memory in a document whose subject is numbers written from memory. The
figure stands corrected at **eleven** in the instances table above.

**The rule this yields: a state must name the artefact class it is true of.** *Applied* against a
casting is not a fact, because a casting is a prototype, a build, and sometimes a plate. A row per
casting can only ever be true of whichever artefact its author had open.

### The table re-derived: which artefacts hold a copy of something still per-casting

**Not six more rows for the builds — that grows the table in the wrong direction.** The extraction took
the geometry, the band, the LCD cell with its budget and cap, the Program-button construction and state
matrix, the meter wells and the chevron path: they exist **once**, in a library each casting pins by
tag. **A body cannot miss a change to a figure it holds no copy of**, so for everything the extraction
took there is no propagation left to track and no row to keep.

**What still propagates by hand is exactly what stayed a prop** — §1's *not shared* list. The artefact
classes that hold a copy are the six panels, the six builds, the strip, and the two plates:

| Prop (§1) | Panels | Builds | Strip | Plates |
|---|---|---|---|---|
| Fascia material and colour | copy | copy | copy | **baked** — both plates |
| Header block's own material | copy | copy | copy | **baked in Fifth Member's**; Chorus-60's plate stops at the fascia, badge and box frames |
| **Ink** | copy | copy | copy | **baked in Fifth Member's** — it carries the model line |
| Phosphor | copy | copy | copy | no — glass is drawn at runtime in both |
| Nameplate metaphor | copy | copy | copy | **baked in Fifth Member's**; TapeRot's ships as its own artwork |
| Casting's mono face | copy | copy | copy | **baked in Fifth Member's** model line |
| Accent colour | copy | copy | copy | no |
| Canvas height | copy | copy | no | **a plate is a bitmap cut at a ratio of it, and no library re-cuts one** |

**Retired for the taken figures, continuing for the props** — which is the answer this closes on. Two
consequences worth stating:

- **The chevron cannot propagate wrongly again**, and the artwork row already recorded that no plate
  bakes it. Structurally solved rather than tracked.
- **The 1340 canvas is solved in code and still live in artwork.** Core owns the width; a plate is a
  cut at a ratio of it. **The artwork row stays, and it is the only survivor of the geometry side.**

**The ink row is the argument for all of it.** It is the one of §10's three tracked changes the
extraction did *not* take, and it is the one that went wrong **again, inside this round** — two
independent fixes of one contrast defect landing on `#B3BFD3` and `#B7C2D8`, after the table had already
recorded the ink as a known propagation failure. **The table did not prevent the repeat.** An
enumeration is a snapshot; **what stops a figure drifting is having one copy of it**, and props are the
figures that will not get one.

| # | Dependant class | Still tracked for | Retired for |
|---|---|---|---|
| 1 | The six panels | every prop above | geometry, band, LCD cell and budget, button matrix, wells, chevron — pinned from core |
| 2 | The six builds | every prop above — **and they were never rows before** | same |
| 3 | `Header Part — Six Materials` | every prop it displays, and it is **not citable** either way (see below) | same |
| 4 | **Exported artwork** — `design/MANIFEST.md` §1 | **canvas height, and every prop a plate bakes** | nothing. A bitmap holds a copy of whatever was baked into it |

**Naming artwork as a class of its own is what surfaced that Fifth Member’s plate is a dependant of the
ink change as well as the canvas change** — it bakes the model line, so the corrected `#cfc6b4` has to be
in the 3× cut or the panel and its plate will disagree on the one figure this round fixed twice.
That is not visible from the panels alone. **It read *not yet* across two bundles and is now delivered:**
Chorus-60 **4020 × 2436** (3× of 1340 × 812) and Fifth Member **4020 × 3036** (3× of 1340 × 1012), the
latter carrying §10.8's two ink changes; the superseded 2564 × 1552 and 3720 × 2793 files are not in the
bundle. **A row that only ever reads "not yet" is doing its job** — this one carried the single asset the
round had been treating as complete.

**A canvas or geometry change invalidates every plate cut against the old figure, whatever
ratio it was cut at.** `design/MANIFEST.md` states delivered *and* drawn dimensions on every
row for that reason: a ratio is a statement about a base, and a ratio with its base left out
reads as done.

### 11 · The type-adoption gate — no casting adopts the budget before its face lands

**Rule: a casting does not adopt the shared LCD budget of 49 or the cap of 47 until its own
`fonts/` holds Share Tech Mono.** Both figures are measured on that face and on the 538.00 name
area; adopting them against a face that is not in the folder is adopting a measurement nobody can
reproduce.

**Why this is a gate and not a preference: a cap may never shrink.** It limits the names already
saved as Programs, so a cap raised against an absent face is not a re-export — it is a data
migration. **The one irreversible figure in this part is the one most easily asserted.**

Three castings were short (reflect-84 both faces, taperot Barlow Condensed SemiBold, elmer Share
Tech Mono). **All three landed, so every casting now holds the measured face and this gate is
satisfied rather than waived** — it stays in force for a seventh casting, which is the case it was
written for. Elmer was the named instance: its cap rise of **22 → 47**, the largest in the suite, is
now measured against a face that ships. Register: `FONTS.md`; disposition:
`design/CALL-3-DISPOSITION.md` §3.

### 12 · The Program list runs to the panel bottom — and a withdrawn contract change

**The rule is unchanged: the list runs from the display's bottom edge to the panel's bottom edge and
never outgrows it.** Height = canvas − the LCD's bottom edge. Reflect-84 is **553 = 648 − 95**. No
casting carries a gap under its list.

**A contract change was proposed here and is withdrawn.** It read: the list runs to the fascia inset,
16 px above the panel bottom, height = canvas − LCD bottom − 16 — offered because 537 was the
delivered figure and 16 is genuinely the only inset this part uses (block at x 16 / y 16, w 1308 =
1340 − 2 × 16, body origin x 16). **The derivation was sound and the conclusion was wrong**, for two
reasons worth keeping:

- **The contract already decided it.** A 16 px gap under one casting's list while the other five reach
  their panel bottoms is precisely the drift the shared part exists to prevent, and **changing five
  panels to justify one figure is the wrong direction of fit.**
- **Nobody ever wrote 16.** It appears in no spec, no panel and no changelog as a list margin — it was
  found by looking for something 537 could be derived *from*. **A derivation constructed after the
  fact to explain an unexplained figure is a reconstruction, not a base**, and this round has already
  spent three candidates that fitted their gap and were wrong. That a figure *can* be derived is not
  evidence it *was*.

**537 was a transcription with no base, and the honest correction is the figure, not the rule.**
Reflect-84's §4 and §10 now carry 553 with `648 − 95` beside it. **The one thing a wrong figure must
not buy is a change to the part.**

### The strip's status, stated here because it is part of this file's surface

**`Header Part — Six Materials` is not citable.** No figure is read off it, no ratio measured
on it, no string taken from it. In one pass it was found carrying a wrong model number, a wrong
descriptor, a string lifted from the wrong row, an ink worse than the body it was meant to have
fixed, and two block materials that do not ship. Its geometry demonstration — one part in six
materials, at a glance — is what it is for, and that survives intact.

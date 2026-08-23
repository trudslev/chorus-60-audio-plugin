# NEON FOUNDRY — THE ABOUT PART

**Suite-wide contract. One part, six castings.** This file lives in `shared/` and is never copied
into a casting folder — a figure duplicated six times disagrees with itself within a round.

**Revision 1** · geometry and behaviour only. **The six materials come after** and are §9. Every
figure below is either **measured** off the rendered part (`About Part — Shared Geometry`, 1×) or
**computed** from a stated law, and says which.

**Asset format: nothing in the About part is baked.** Every element is drawn at runtime. It carries
no artwork on any casting, including the two whose wordmarks are artwork — see §8.

---

## 1 · What is shared and what is not

**Shared, and identical in all six:** the affordance and where it sits, the veil, the box's size and
position law, every internal coordinate in §4, every type size and tracking, the row order, the
dismissal set, the link treatment, the character budgets, and the content of every line except the
five strings named below.

**Not shared:** the box's surface material and its inset-ring colour, the ink triple (body, dim,
accent), the mono face, and five strings — casting name, model code, plugin version, repository
slug, typeface credits.

**Suite release and plugin version are separate version strings and are printed on separate lines.**
**Both are semver**, and the suite's is a version rather than a counter:

| A round that is | gives the suite a |
|---|---|
| patch-only across every plugin | **patch** bump |
| any one plugin's minor | **minor** bump — and patch resets to **0** |
| a breaking change anywhere | **major** bump |

So the field is a string like **1.2.1**, never an integer, and the plugin line matches: **1.0.0**, not
1.0. Revision 1 printed `SUITE RELEASE 3` and treated the field as a counter — wrong against the
decided scheme, and wrong in a way that would have hard-coded an integer into six string fields.

**The two numbers do not agree and neither derives from the other.** The suite's minor moves when any
single plugin's minor moves, so a casting sitting at 1.0.0 inside suite 1.2.1 is the normal case, not
a mismatch. Never print them on one line, where a reader could take the second as a build tag of the
first.

---

## 2 · The affordance: the version stamp, promoted

**Instruction: the About part opens from the version stamp, which becomes a recessed tab.** No new
element is added to any panel.

**Why not the wordmark.** It is the nameplate metaphor and it is **artwork on two castings** —
Gatecrasher's `gatecrasher-wordmark.png` and TapeRot's `taperot-wordmark.png`. An affordance on the
wordmark would be a hit region over a bitmap on two panels and over live text on four, which is
six answers to one question. **The affordance has to sit on something all six draw the same way.**

**Why not a corner screw.** A screw reads inert, and BRAND.md forbids drawing anything on these
panels inert or decorative. Making one clickable does not fix that — it makes an inert-looking
element load-bearing, which is worse than a decorative one.

**Why the version stamp.** All six already carry it, bottom-right, in the casting's own mono at
10 / 13 with .10–.18 em tracking, right-aligned. It is the only string on any panel whose entire job
is identifying this build — so an About box is what it already implies, and clicking it is not a
control action. It is also the one place a reader already looks for a version number and finds one
figure where there are two.

| | Value | Source |
|---|---|---|
| Right edge | **1302** | Computed — §2 of `HEADER-PART.md`: the meter row's right edge, closing the block's own 22 px left padding |
| Bottom edge | **canvas height − 20** | Law |
| Height | **24** | Computed — 13 px line box + 5.5 padding each side. Even, so 0.5× lands on whole pixels |
| Width | **shrink-to-fit**, padding 10 each side | The strings differ in length by casting: `v1.0` to `GL-87 · SN 0042 · v1.0` |
| Face | casting's own mono, **10 / 13**, tracking as the casting already sets it | Unchanged from the stamp today |
| Construction | **inset ring, not a border** | `HEADER-PART.md` §3 applies unchanged |

**The stamp's ink moves from flavour class to functional class.** Today it is the dimmest text on
several panels — Reflect-84's is `#5e5440` at **4.71**, below the 7:1 functional floor, and that was
correct while it was a stamp nobody had to read. **A tab that opens something is functional.** Every
casting re-inks it to **7:1 or better against the well's own face** — not against the fascia, which
no ink can achieve on two of the six. Figures in §9.2. This is the only change the About part makes
to a panel in its resting state, and it is not optional.

**Hover** lightens the well one step and takes the ink to the casting's accent. **The recess is the
resting affordance** — a shallow etched plate reads pressable on hardware at rest, which a
hover-only underline does not.

---

## 3 · The veil, and not colliding with bypass

| | About | Bypass (`§7.5` where it applies) |
|---|---|---|
| Colour | the casting's **darkest ink at 0.72** | `#808080` at 0.50 |
| Direction | **darkens** | **greys** |
| Carries | the box | nothing |

**Instruction: the About veil is a darkening scrim, not a grey multiply.** The two treatments are
different colours in different directions, so a reader can tell which is which with both on screen.

**They stack and neither suppresses the other.** Bypass is host-driven and About is user-driven;
suppressing either from the other's state would put panel logic in charge of a host decision.
Stacked, the panel beneath is unreadable and that is correct — both are transient overlays and
neither is a control surface. **TapeRot has no bypass at all** (§7.5 there is marked *does not
apply*), so on that casting the question cannot arise.

**The veil is the click target for dismissal and takes keyboard focus on open**, which is what makes
Escape work. Without that the key goes to the host.

---

## 4 · The box

**880 × 540**, radius 3, **inset ring not a border** (`HEADER-PART.md` §3).

| | Value | Source |
|---|---|---|
| x | **230** | Computed — (1340 − 880) / 2. Canvas width is 1340 in every casting |
| y | **(canvas height − 540) / 2** | Law. 54 at Reflect-84's 648, 136 at Chorus-60's 812 |
| Minimum canvas height | **620** | Computed — 540 + 40 top and bottom. The shortest casting is 648 |

**Fixed size, not a proportion.** The castings are differently-sized units rather than scales of one
design, and the box carries the same words in all six, so a box that changed size would change the
line breaks and the legibility with it.

**Internal coordinates**, box-local, origin top-left. Label column x **40** width **150**, value
column x **200** width **640** — 40 + 150 + 10 gap, and 200 + 640 = 840 = 880 − 40.

| y | Element | Face | Size / line box | Tracking |
|---|---|---|---|---|
| 40 | Casting name | Barlow Condensed 600 | **34 / 40** | .10 em |
| 84 | `MODEL <code> · VERSION <major.minor.patch>` | casting's mono | **18 / 24** | .14 em |
| 120 | rule, x 40 → 840, 1 px, body ink at 18 % | | | |
| 144 | `NEON FOUNDRY` | Barlow Condensed 600 | **20 / 26** | .20 em |
| 174 | `SUITE RELEASE <major.minor.patch>` | casting's mono | **18 / 24** | .14 em |
| 210 | rule | | | |
| 234 | `LICENCE` · `AGPLv3 · SOURCE AVAILABLE` | Barlow 600 / mono | **18 / 24** | .22 / .10 em |
| 270 | `REPOSITORY` · link | Barlow 600 / mono | **18 / 24** | .22 / .10 em |
| 306 | `FOUNDRY` · `neonfoundry.io` | Barlow 600 / mono | **18 / 24** | .22 / .10 em |
| 342 | rule | | | |
| 366 | `TYPEFACES` · credits, wraps | Barlow 600 / Barlow 500 | **18 / 24** | .22 / .04 em |
| 426 | Resources note, wraps | Barlow Condensed 500 | **16 / 21** | .04 em |
| 470 | `CLOSE`, 96 × 30 at x **744** | Barlow Condensed 600 | **16 / 20** | .22 em |

**Character budget for the repository line: 45 characters.** The longest is
`github.com/trudslev/fifth-member-audio-plugin` at 45, which measures **486 px** at 18 px mono with
.10 em — inside the 640 px value column with 154 to spare. **A slug longer than 45 characters
breaks the row**, and the row must not be allowed to wrap: a wrapped URL invites a reader to copy
half of it.

---

## 5 · Type floors at scale, which is the hard part

The panels hold functional text at **7:1** and nothing functional below about **10 px**. The About
part carries more text than any panel and must survive **0.5× to 2.0×**.

**Instruction: nothing in the box is set below 16 px — 1.6× the panel's own functional floor.**

**Including CLOSE.** Revision 1 set it at 14 / 18 while stating this rule two sections later, which
would have shipped the part contradicting its own hard constraint on one of its three interactive
elements. It is **16 / 20**: at .22 em that is a 47 px glyph run inside a 96 px shoe, so the size was
available for nothing. **A constraint with one exemption in the same document is not a constraint** —
if a legend ever genuinely cannot make 16, the shoe grows.

That is the whole answer, and it works because **the box degrades later than the panel it sits on**:

| At 0.5× | renders at |
|---|---|
| the box's smallest text (16) | **8 px** |
| the box's data lines (18) | **9 px** |
| the panel's own functional text (10) | **5 px** |
| the panel's printed numerals (10–11) | **5–5.5 px** |

**So if the panel is usable at a given scale, the box is comfortably usable** — the box is never the
thing that fails first. That is a stronger guarantee than an absolute floor, because an absolute
floor at 0.5× would force 20 px minimums and a box too tall for the shortest casting.

**Every size in §4 is a whole number and every line box is even**, so 0.5× lands on whole pixels and
2.0× introduces no half-pixel rules. The three rules are 1 px and become 0.5 px at 0.5×; they are
**structure, not functional text**, and are exempt from the floor.

**All box text is functional and all of it clears 7:1** against the box's own surface. §9 states the
measured ratio per casting — this is the one place where a casting can fail the part.

---

## 6 · Dismissal — all three, all six

**Escape**, **click anywhere on the veil**, and the **CLOSE** legend. Not a choice between them:
each covers a reader the others miss — the keyboard user, the reader who expects a modal to be
dismissible by its background, and the reader who wants to be told where the exit is.

**CLOSE is a legend on a shoe, not a new part** — the two-position shoe of `PARTS-CATALOGUE` §4B at
96 × 30, drawn in its idle state with a permanent legend. **Nothing else in the box is a control.**

**Opening and closing touch no parameter and persist nothing.** The box is not a state of the plugin;
it does not enter the parameter tree, does not serialise, and is closed on every load.

---

## 7 · How a link reads as a link with no web idiom on the panel

**Instruction: the underscore marks the link. The accent marks the phosphor class.** Two marks doing
two jobs, and the part needs both because the box holds two kinds of accent text.

**Accent** is what these panels already say for phosphor — a lit lamp, an LCD string, a scope trace.
The plugin line and the suite line are accent because they **are** phosphor-class data: numbers read
off the build, the same class as the figure in a meter well. **The underscore** is what marks a
destination.

**So underscoring the plugin and suite lines would break the rule rather than clarify it** — they are
data, not destinations, and an underscore would say they lead somewhere. Nothing is ambiguous once
the two marks are read as two statements: *accent* says this is instrument data, *accent underscored*
says this goes somewhere. Revision 1 recorded this as an unresolved ambiguity; it was a misreading of
the part's own vocabulary.

Carried by three marks together, because one is not enough on a surface with no web idiom:

1. **Accent ink**, against body ink everywhere else.
2. **A 1 px underscore** in the accent at 50 %, going solid on hover.
3. **The cursor**, and on hover the ink brightens one step.

**The two data lines that are not links — the plugin line and the suite line — are also accent**,
because they are phosphor-class data rather than prose. **That is deliberate and it is the one
ambiguity in this part:** accent alone does not mark a link here; accent **plus an underscore** does.
Underscoring the two data lines would fix the ambiguity and introduce a worse one, since an
underscored non-link is the more misleading of the two. Flagged rather than solved.

---

## 8 · Typeface credits

**Acknowledgement, not a legal document.** The full text ships as `THIRD-PARTY-LICENCES.txt` in each
bundle's `Resources`, and the box says so on its last line. **The box must not paraphrase a licence**
— it names faces and licence families and points at the file.

**Instruction: credit the faces the casting embeds.** Not the faces it draws with.

Those differ on two castings and the difference is the point. **Gatecrasher's Tudor Victors is not
embedded** — it is © Chequered Ink 2020, All Rights Reserved, no licence was bought, and its
letterforms ship as artwork (`gatecrasher/fonts/ABSENT.md`). **It must not appear in Gatecrasher's
credits**, which would assert an embedding that licence forbids. It is acknowledged in
`THIRD-PARTY-LICENCES.txt` as artwork instead. **TapeRot's nameplate is the same case.**

Per-casting lists belong to §9. What is shared is the row, its position, and this rule.

---

## 9 · The six materials

**The box is the casting's own display glass, not its fascia.** That is the material decision the
rest of this section follows from: on these panels glass is the surface that already carries dense
small text at high contrast, every casting has one, and a reader already knows text on glass is meant
to be read. **The About box is a screen, not a plate.** It also settles §3 for free — a grey bypass
multiply over a dark screen still reads as a grey wash, where over a plate it would compete.

**Every ratio below is computed** from the stated hex pairs by the WCAG relative-luminance formula,
against the **worst-case stop** of each gradient rather than its mean.

### 9.1 The box

| Casting | Glass | Body | Dim | Accent | Mono |
|---|---|---|---|---|---|
| chorus-60 | `#0b0d0f` | `#e6ebee` **16.21** | `#9aa2a6` **7.50** | `#e5a021` **8.70** | Share Tech Mono |
| elmer | `#0e0d08` | `#efeae1` **16.23** | `#a8a291` **7.63** | `#e6dcae` **14.09** | IBM Plex Mono |
| gatecrasher | `#0d0f11` | `#f2f5f6` **17.53** | `#9fa9ad` **8.00** | `#e8c96a` **11.88** | Share Tech Mono |
| taperot | `#100e0b` | `#f2ebd8` **16.20** | `#a89c85` **7.12** | `#f2b25c` **10.37** | Share Tech Mono |
| fifth-member | `#121210` | `#f5f0e5` **16.50** | `#a9a291` **7.38** | `#e2bd7c` **10.54** | Share Tech Mono |
| reflect-84 | `#060a11` | `#f2e6c2` **15.93** | `#a89f86` **7.52** | `#5ce07a` **11.70** | IBM Plex Mono |

**All eighteen inks clear 7:1.** The dim column is the tight one — TapeRot's 7.12 is the suite's
narrowest margin, and it is the label column, so **a casting may not darken its dim ink to taste**.

**Inset ring** is the glass lightened to roughly 18 % against the fascia's hue: chorus-60 `#242a2d`,
elmer `#2a2820`, gatecrasher `#2a3134`, taperot `#2e281f`, fifth-member `#2b2823`, reflect-84
`#1f2b44`.

**Chorus-60 is the one casting whose fascia is dark**, so its box separates from the panel by the
veil, the ring and the drop shadow rather than by value. That is sufficient and it is worth naming:
**the box is not required to contrast with the fascia, only with its own contents.**

### 9.2 The tab, and the correction §2 needed

§2 says the promoted stamp must clear **7:1 against its own fascia**. **That instruction is
impossible on two castings and is replaced.**

Measured against the darkest stop of each fascia gradient, the best a dark ink can do is:

| Casting | Darkest fascia stop | Best achievable |
|---|---|---|
| gatecrasher | `#8e959a` | **6.45** at near-black |
| fifth-member | `#63605a` | **3.16** at near-black |

**No ink reaches 7:1 on a mid-value fascia**, because the ceiling is set by the background, not by the
ink. Elmer only just clears it at 7.19, and only by going almost black.

**The tab is a recess with its own face** (§2: inset ring, not a border), so **the ink is measured
against the well, which the casting chooses — not against the fascia, which it does not.** On a
light-fascia casting the well is a dark recess, which is what a recess looks like anyway, and the ink
goes light:

| Casting | Well face | Ink | Ratio |
|---|---|---|---|
| chorus-60 | `#191c1e` → `#212527` | `#cdd4d8` | **10.31** |
| elmer | `#1f1e17` → `#2a281f` | `#e2dcc9` | **10.77** |
| gatecrasher | `#2b2f32` → `#33383b` | `#c3cace` | **7.15** |
| taperot | `#241f18` → `#2d2720` | `#e6dcc4` | **10.82** |
| fifth-member | `#1f1d18` → `#282520` | `#e4dfd0` | **11.46** |
| reflect-84 | `#1b2334` → `#232c40` | `#dfe6ef` | **11.09** |

**Gatecrasher's 7.15 is the suite's narrowest tab margin** and its well may not be lightened.

**This is the same shape as the meter-numeral case:** an instruction was written against the surface
the element sits *on* rather than the surface it sits *in*, and it read as satisfiable until it was
measured. **A contrast requirement has to name the two colours it compares**, not one of them and a
region.

**The finding generalised past this part** and is now suite-wide in `CONTRAST-CEILING.md`: a 7:1
floor is unreachable on any ground of relative luminance **0.100 to 0.300**, whatever the ink. **The
law stands and the sweep found no live case in any casting** — its first pass claimed three and all
three were its own pairing errors. This part's own §9.2 finding is the one real instance in the suite,
and it was found by measuring the element against the surface it sits in.

**Hover** takes the ink to the casting's accent from §9.1 and lightens the well one step. Each accent
clears 7:1 on its own well face, since the wells are darker than the glass in every casting except
gatecrasher's, whose accent measures **8.66** on `#33383b`.

### 9.3 Embedded typefaces, per casting

**Embedded faces only** (§8). The wordmark faces of the two artwork castings are absent from these
lists by licence, not by oversight.

| Casting | Credit line |
|---|---|
| chorus-60 | Barlow Condensed, Share Tech Mono and **Librestile Extended**, all under the SIL Open Font License. |
| elmer | Barlow Condensed, IBM Plex Mono, Share Tech Mono and **Archivo Expanded**, all under the SIL Open Font License. |
| gatecrasher | Barlow Condensed and Share Tech Mono, both under the SIL Open Font License. |
| taperot | Barlow Condensed and Share Tech Mono, both under the SIL Open Font License. |
| fifth-member | Barlow Condensed, Share Tech Mono and **Permanent Marker**, all under the SIL Open Font License. |
| reflect-84 | Barlow Condensed, IBM Plex Mono, Share Tech Mono and **Jost**, all under the SIL Open Font License. |

**Gatecrasher's and TapeRot's lines are the short ones, and that is the licence showing through.**
Tudor Victors and TapeRot's nameplate face are **not embedded** — their letterforms ship as artwork —
so naming them here would assert an embedding their licences forbid. Both are acknowledged as artwork
in `THIRD-PARTY-LICENCES.txt`. **A reader comparing two About boxes will notice the shorter list; the
answer is that it is shorter because it is true.**

**Every face named above is OFL**, so the credit line is one sentence in every casting and the
licence family never has to be given per face.

### 9.4 What each casting changes in its resting panel

**Only the version stamp.** It gains the recessed well of §2 and §9.2, and its ink moves from flavour
class to the figures above. Everything else in this part exists only while the box is open.

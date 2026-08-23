# NEON FOUNDRY — THE CONTRAST CEILING

> ## CORRECTIONS — recorded 2026-08-23, after this document was delivered
>
> **Three of §2's cases do not exist. The §1 law and §2.4's veil finding are unaffected**, because
> both are properties of arithmetic rather than of any casting — which is exactly why neither shared
> the failure. All three retractions have one cause: **grounds and inks paired by pattern rather
> than by what stands on what.**
>
> | §2 case | Correction |
> |---|---|
> | **Gatecrasher `#8e959a`, ceiling 6.92** | Resolves in the theme's favour. `fasciaDark` `#BDC2C7` is right; `#8e959a` describes the two 16 px side rails, **which carry no text**. Nothing in the theme changes |
> | **Elmer `#34322a` at 5.24, "an ink problem"** | **There is no ink error.** The control-label ink is `#0e0d08`, 8.62 and 7.94 on the two fascia stops. `#34322a` occurs once in the casting — the footer version stamp, a *flavour* role clearing its own 4.5 floor at 5.24 |
> | **Elmer unlit KNEE `#8E8A7D`, ceiling 6.08** | **The role does not exist.** §4B deleted it, the built shoe carries no legend on either face, and `#8E8A7D` appears nowhere on the panel. 6.08 describes a construction that was replaced |
>
> **§3.5 is the rule all three broke** — *a dead-band ground is only a fault where a functional role
> stands on it* — and it is stated in this very document. Two of the three grounds carry no text at
> all and the third is not drawn.
>
> **The consequence is that the ceiling mechanism has no live case to catch today**, and that is the
> correct position for a guard rather than a reason to doubt it. `tools/check_contrast.py` computes
> the ceiling and reports *unreachable* separately from *missed*; both directions were proved by
> **causing** them, because neither could be found. The lowest ceiling among all declared grounds in
> the suite is **8.28** — no declared ground is in the 0.100–0.300 band.
>
> The document is otherwise unchanged and is kept as delivered.

**Suite-wide. Applies to every casting and every functional role.** Written 2026-08-23, from the
`ABOUT-PART.md` §9.2 finding that a 7:1 instruction can be **unsatisfiable** rather than merely
demanding.

**Every figure below is computed** by the WCAG relative-luminance formula from hexes read out of the
six delivered prototypes.

---

## 1 · The law

**A ground has a maximum contrast that no ink can exceed.** For a ground of relative luminance `L`:

- the best a **black** ink can do is `(L + 0.05) / 0.05`
- the best a **white** ink can do is `1.05 / (L + 0.05)`

The ceiling is the better of the two. Solving each for 7:

> **A 7:1 floor is unreachable on any ground whose relative luminance lies between 0.100 and 0.300.**

That band is the whole finding. **It is a property of the ground alone** — it does not depend on the
ink, the face, the size, or the role. Inside it, no ink exists that satisfies the floor, and a
specification demanding one is not strict but void.

**Above 0.300** a black-ink role is satisfiable; **below 0.100** a white-ink one is. The band between
is mid-value grey and beige — which is exactly what a metal fascia is.

**Instruction: a contrast requirement states the two colours it compares.** Not one colour and a
region. `ABOUT-PART.md` revision 1 said the version tab must clear 7:1 "against its own fascia" and
was unsatisfiable on two castings; the tab is a **recess with its own face**, and once the second
colour was named the requirement was easy. **Two of the faults in this suite's history were this
shape**, including Elmer's meter numerals.

---

## 2 · The sweep

Every ground in the six prototypes, paired against the inks actually used on it.

### 2.1 Clean — the ceiling is not in play

| Casting | Ground | L | Ceiling | Inks |
|---|---|---|---|---|
| chorus-60 | `#0a0c0d` – `#141618` | .004–.008 | 18.1–19.6 | 15.1–16.5 ✓ |
| taperot | `#e2d8bf` – `#efe6d0` | .690–.795 | 14.8–16.9 | 8.79–12.35 ✓ |
| reflect-84 | `#d8cdb0` – `#efe6d0` | .614–.795 | 13.3–16.9 | 7.62–11.23 ✓ |
| fifth-member | `#121210` body | .006 | 18.8 | `#c3bcae` **9.94**, `#b0aa9c` **8.11** ✓ |

**Chorus-60 is the only casting with a dark fascia and it is the only one with no exposure at all.**
Its whole panel sits below 0.010, so every functional role has an 18:1 ceiling. The two light-fascia
castings that pass do so because their fasciae are genuinely light — .69 and up, well clear of the
band's top.

**Fifth Member's five-stop rail gradient contains three dead-band stops** — `#7e7a73` (4.92),
`#6f6b64` (5.30), `#63605a` (6.27) — **and carries no text.** It is the 52 px side rail. Its labels
sit on the `#121210` body at 8–10:1. **Flagged and cleared:** a dead-band ground is only a fault
where a functional role stands on it, and this is the case that shows why the sweep pairs grounds
with inks rather than listing grounds alone.

### 2.2 Real exposure — two castings

| Casting | Ground | L | Ceiling | Ink in use | Verdict |
|---|---|---|---|---|---|
| **gatecrasher** | `#8e959a` (darkest fascia stop) | **.296** | **6.92** | `#16191c` → 5.81 | **7:1 unreachable** |
| gatecrasher | `#9aa1a6` (mid stop) | .351 | 8.02 | `#16191c` → 6.74 | reachable, **not met** |
| gatecrasher | `#b4babe` (lightest stop) | .485 | 10.71 | `#16191c` → 9.00 | met ✓ |
| **elmer** | `#aca596` (darker fascia stop) | .379 | 8.58 | `#34322a` → **5.24** | reachable, **not met** |
| elmer | `#b3ac9d` | .415 | 9.30 | `#34322a` → 5.69 | reachable, **not met** |
| **elmer** | `#8E8A7D` (unlit KNEE legend) | **.254** | **6.08** | ruled at 3.0 | **7:1 unreachable** |

**Gatecrasher's fascia is a 90° gradient that crosses the band's top edge.** A control label is one
ink over all three stops, so **the same role passes at one end of the panel and cannot pass at the
other** — 9.00 on the light stop, 6.92 as a ceiling on the dark one. This is not fixable by re-inking:
the label is already near-black. It is fixable only by lifting the darkest stop above L 0.300, or by
accepting a lower floor for that role and saying so.

**Elmer's labels are reachable and simply have not been cut dark enough.** `#34322a` gives 5.24; the
floor needs `#1a1913` (7.19) or darker. That is an ordinary miss, not a ceiling problem, and it is the
distinction this sweep exists to draw: **one of these two castings has a specification problem and the
other has an ink problem, and before the ceiling was computed they looked identical.**

### 2.3 Elmer's unlit KNEE legend — the functional reading was never available

**Confirmed.** `#8E8A7D` is L **.254**, mid-band, ceiling **6.08**. It was ruled at **3.0** for the
state-legend reason, which stands on its own. But had that ruling gone the other way, **7:1 could not
have been delivered** — the best any ink achieves on that ground is 6.08, and only in black.

**So the two readings were never symmetric.** One was available and one was not, and the argument was
conducted as though the choice were between two achievable outcomes. **The right conclusion is not
that the ruling was lucky** — it was correct — **but that a ceiling check belongs before a floor
argument, not after it.** An unreachable option in a two-way decision is not an option.

### 2.4 The bypass veil moves every ground

The disengaged treatment is `#808080` at 0.50, multiply. It roughly halves the luminance beneath it,
which **moves grounds into and through the band**:

| Casting | Fascia | L | → veiled | L | Ceiling veiled |
|---|---|---|---|---|---|
| taperot | `#e2d8bf` | .690 | `#716c60` | .151 | **5.23** |
| reflect-84 | `#e2d8bd` | .690 | `#716c5f` | .151 | **5.23** |
| elmer | `#aca596` | .379 | `#56534b` | .087 | 7.68 (white ink) |
| gatecrasher | `#9aa1a6` | .351 | `#4d5153` | .081 | 8.02 (white ink) |
| fifth-member | `#9d998f` | .319 | `#4f4d48` | .074 | 8.44 (white ink) |

**The two lightest fasciae land squarely in the dead band when veiled** and no functional role on them
can meet 7:1 while bypass is on. The other three fall through the band and out the bottom, where a
**white** ink would satisfy the floor — but their inks are dark, so in practice every light-fascia
casting is below the floor while bypassed.

**That is acceptable and it should be stated rather than discovered.** Bypass means the plugin is not
processing; its own rule is that pointers do not move, the scope freezes and every lamp goes out, so
nothing on the panel is being read for a value. **A contrast floor is a floor on legibility while
operating.** What must not happen is a checker asserting the floor across a veiled panel and being
right to fail it.

**TapeRot has no bypass** (`taperot/GUI-SPEC.md` §7.5, *does not apply*), so of the two castings in the
worst position, one cannot enter it.

---

## 3 · What to do with this

1. **Every new contrast requirement names two colours.** A requirement against "the fascia" or "the
   panel" is not checkable and may not be satisfiable.
2. **Compute the ceiling before arguing the floor.** \u00a72.3 is the case: a two-way ruling where one
   side was unavailable and nobody knew.
3. **Gatecrasher and Elmer are open**, for different reasons — a specification problem and an ink
   problem. Neither is a re-cut of artwork.
4. **A checker asserting 7:1 must exempt the veiled state**, or it will fail five castings for a
   reason that is correct and irrelevant.
5. **A dead-band ground is only a fault where a functional role stands on it.** Fifth Member's rails
   are the counter-example; pair grounds with inks or the sweep cries wolf six times.

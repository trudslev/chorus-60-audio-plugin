# NEON FOUNDRY — THE CONTRAST CEILING

**Suite-wide. Applies to every casting and every functional role.** Written 2026-08-23, from the
`ABOUT-PART.md` §9.2 finding that a 7:1 instruction can be **unsatisfiable** rather than merely
demanding.

**Every figure below is computed** by the WCAG relative-luminance formula. **Revision 2** — revision 1
claimed three live exposures and **all three were its own errors**; §2 now reports figures read off the
built panels, element inward. The law in §1 is unaffected and was never in question.

**The pattern in all three is one mistake:** a luminance figure is trivial to compute, so it was
computed early and often, against grounds and inks assembled by pattern-matching the source instead of
by asking what stands on what. **Cheap arithmetic applied to an unverified pairing produces confident
nonsense**, and it produced it three times in one file before anything was measured.

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

### 2.2 Real exposure — **none. The sweep found no live case at all.**

Both remaining Elmer rows are retracted. Measured off the built panel rather than off a grep:

| Casting | Ground | Ink actually on it | Ratio | Verdict |
|---|---|---|---|---|
| elmer | `#b3ac9d` (fascia, top) | `#0e0d08` control labels | **8.62** | passes ✓ |
| elmer | `#aca596` (fascia, bottom) | `#0e0d08` control labels | **7.94** | passes ✓ |
| elmer | `#aca596` | `#34322a` footer stamp | 5.24 | **flavour** role, floor 4.5 — passes ✓ |

**`#34322a` is not a control-label ink.** It occurs **once** in the whole casting, on the footer
version stamp — a flavour role clearing its own 4.5 floor. The control labels are `#0e0d08`, which
the sweep never tested because the sweep tested a list of inks it had assembled by grepping the file
rather than the ink each role actually uses. **Same defect as §2.2b, one section apart: the pairing
was fabricated on both sides.**

**And the KNEE ceiling case does not exist.** `#8E8A7D` appears nowhere in the built panel. The shoe's
real faces are `#dcd6c6 → #bdb6a4` live (L .669–.481) and `#413b31 → #2e2921` idle (L .052–.024) —
both clear of the band, in opposite directions. More to the point, **the built shoe carries no legend
on either face.** Its own source says so:

> *Both legends print permanently, one weight and one ink, on the fascia beneath their own half — so
> there is no unlit-legend role to give a floor to.*

**§4B withdrew that role.** The legends sit on the fascia at `#0e0d08`, 7.94 and up: functional and
passing. So the "unlit KNEE legend" whose ceiling this file computed to two decimal places is a role
that was **deleted before the sweep ran**, and its hex came from a spec paragraph describing the
construction §4B replaced.

**The state ruling at 3.0 stands** — it was never about this. Nothing needs re-arguing, and the note
that "the functional reading was never available" is void rather than reassuring: there was no
legend, no ground, and no reading.

### 2.2b Gatecrasher was a false positive, and it was this file's own error

**Retracted.** Revision 1 of this sweep reported Gatecrasher's fascia as a 90° gradient crossing the
band, with a control-label role passing at one end of the panel and capped at 6.92 at the other. It
offered three routes out. **All three were answers to a fault that does not exist.**

**Gatecrasher's fascia is the 2 px brushed-steel repeat its own §0 states:**
`repeating-linear-gradient(90deg, #c3c8cc 0 2px, #bdc2c7 2px 4px)`, L **.535–.556**, ceiling ~12, and
`#16191c` measures **9.84** on it. The `#8e959a` gradient is the panel's **two 16 px side rails** —
edge shading, 16 px of a 1340 px panel, carrying no text.

**That is the same construct this file had already cleared on Fifth Member one section earlier**, and
the reason the error is worth keeping: **the sweep was fed the first gradient a regex matched in each
file rather than the ground each label actually stands on.** On Fifth Member the rail was obviously a
rail; on Gatecrasher it was first in the file, so it read as the fascia. **§3's own rule — pair
grounds with inks or the sweep cries wolf — was written in the same pass that broke it.**

**A ground is identified by what stands on it, not by where it appears in the source.** A luminance
figure is cheap to compute and that is exactly what makes it dangerous: the arithmetic was right
throughout, applied to the wrong rectangle.

### 2.3 ~~Elmer's unlit KNEE legend — the functional reading was never available~~ — **SUPERSEDED by §2.2**

**The worked example below is withdrawn: there is no such legend.** `#8E8A7D` is in no built panel,
and §4B withdrew the unlit-legend role before this sweep ran — §2.2 has the measurements. **Kept
unedited because its closing argument is sound and is why the mechanism in §1 exists**: a ceiling check
belongs before a floor argument, because an unreachable option is not an option. Only the example is
gone.

**And this section is the failure it now records.** §2.2 was rewritten to retract this case while this
heading and its first word stayed as written — **a retraction added elsewhere in the document, above
stale prose that was never struck.** Two readers met a file that contradicted itself, one section
apart, and both times the correction existed and had simply not been carried to the sentence that
asserted the opposite. **A retraction is not filed until the thing it retracts says so where it
stands.**

**Withdrawn text follows.**

~~Confirmed.~~ `#8E8A7D` is L **.254**, mid-band, ceiling **6.08**. It was ruled at **3.0** for the
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
3. **No casting is open.** Every ground/ink pair in the six panels clears its own floor. The law in
   §1 is sound and its application in §2 was wrong three times out of three — **the sweep's value is
   the law and the bypass finding, not its findings.**
4. **A checker asserting 7:1 must exempt the veiled state**, or it will fail five castings for a
   reason that is correct and irrelevant.
5. **A dead-band ground is only a fault where a functional role stands on it.** Fifth Member's rails
   and Gatecrasher's rails are both counter-examples — **and this file caught the first and missed
   the second.** Identify the ground by what stands on it. Pair grounds with inks, from the element
   inward, never from the stylesheet down.

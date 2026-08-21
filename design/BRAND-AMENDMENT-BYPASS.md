# PROPOSED BRAND.md AMENDMENT — THE DISENGAGED TREATMENT COVERS WHAT IS DISENGAGED

**Three things, not two:** the header never dims, **the scope never dims**, and what dims is
the controls that have become unusable.

**Status: proposal.** Not yet folded into `BRAND.md`. Raised because **two of six castings now
depart from the current rule in the same direction**, which is evidence about the rule rather
than about the castings.

---

## 1 · What the rule says today

> A lighting change only. Applied as a **multiply over the panel**, not an alpha blend toward
> the background. Pitched dark enough to read as off — Chorus-60 lands at 0.50. Pointers stay
> exactly where they are. No desaturation. The legibility floors do not apply. No caption.

Everything in that paragraph is right except its **extent**: "over the panel" is read as
full-bleed, and full-bleed includes the header.

---

## 2 · The two departures, and why each was taken

| Casting | Treatment as built | Stated reason |
|---|---|---|
| **Elmer** | 0.50 multiply over the **body only**; header stays lit | its header is a raised sub-panel in its own material, and darkening it reads as a second unit rather than one unlit one |
| **Chorus-60** | OFF dims the **knob group and the two lower boxes**; header, engine caps, footer and scope stay lit | OFF is a page, not a bypass — the unit is powered and passing dry signal |

Different reasons, same conclusion: **the header does not dim.**

Reflect-84, TapeRot and Gatecrasher currently multiply full-bleed. Fifth Member had no
disengaged state at all until this round and now follows the full-bleed rule.

### The two castings with no recorded choice — **both now recorded** (2026-08-21)

The suite audit found four castings documenting whether they had a disengaged state and two
saying nothing. Both are ruled:

| Casting | Has a bypass? | Where it is recorded |
|---|---|---|
| **TapeRot** | **No, by decision** — this effect is a tape path, and a bypass would be the tape being out of the machine | `GUI-SPEC.md` §7.5, marked **does not apply**; `Source/PluginProcessor.h` |
| **Gatecrasher** | **Yes** — full-bleed 0.50 multiply as built, moving to the control area under §4 above, its envelope scope leaving the multiply and holding the closed-gate baseline it already draws | this table; its `GUI-SPEC.md` has **no §7.5 to mark** |

TapeRot's veil stays implemented and driven from `getBypassParameter()` — unreachable, one
override away, and stated in the code rather than left as a dead branch. **A specified state a
casting cannot enter is marked, not deleted**, the way §6 marks Elmer's absent meter; §9 wants
that gap visible. Reversing either is a processor change, not a panel one.

---

## 3 · The shared reason, which is stronger than either casting's

**The header keeps working while the effect is disengaged.** Programs still load, SAVE and
DELETE still respond, the bank tag still reads, the meters still show peak. Dimming them says
*unusable* about controls that are not — which is the one thing the disengaged treatment is
supposed to communicate, aimed at the wrong half of the panel.

That reason applies to all six castings. Neither Elmer's material argument nor Chorus-60's
page argument is needed to reach it; both are local justifications for a rule that should have
been general.

---

## 4 · Proposed replacement wording

> **The disengaged treatment covers what is disengaged.** A lighting change only: a
> **0.50 `#808080` multiply** over the processing section of the panel — every control that
> stops doing anything while the effect is bypassed.
>
> **The header is never part of it.** Program management and metering keep working when the
> effect does not, so they keep their brightness. A dimmed SAVE button claims a control is
> unavailable when it is not.
>
> **The live display is never part of it either.** A scope is a readout, not a control: it
> keeps working and keeps reporting accurately, and what it reports while the effect is
> disengaged is a flat trace. **Showing nothing is showing something** — the flat line is the
> indication. A dark scope is ambiguous between disengaged and broken; a lit scope holding a
> flat line says exactly one thing. Dimming it would say *not to be trusted* about the one
> element telling the truth.
>
> Pointers stay exactly where they are. No blur, no defocus, no desaturation, no redrawn or
> flattened controls, no caption. Lamps belonging to the disengaged section go out. The
> legibility floors do not apply inside the multiply.

---

## 5 · What adopting it costs

| Casting | Change |
|---|---|
| Elmer | **none** — stops being a departure |
| Chorus-60 | **none** — stops being a departure; its page-versus-bypass distinction remains its own |
| Reflect-84 | **two lines** — multiply moves from full-bleed to the control area, and the TANK LIVE scope leaves it (trace holds flat, LED out, glass and legends at full brightness) |
| TapeRot | **two lines** — same, and its wow/flutter scope leaves the multiply |
| Gatecrasher | **two lines** — same, and its gate-envelope scope leaves the multiply, holding the baseline it already draws when the gate is closed |
| Fifth Member | **two lines** — same, plus its rack ears leave the multiply: they are chassis, not controls |

Eight one-line changes across four castings, and the rule then describes what all six do.

**The scope question is settled, in the direction two castings already implement.** Chorus-60
leaves its scope lit under OFF and Elmer keeps its meter lit; both were ruled correct at the
time, for this reason. Reversing it for the bypass case would give one component two rules
depending on which state reached it.

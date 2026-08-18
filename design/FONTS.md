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

## What a substituted face costs, stated once

An undeclared absent font gets "fixed" by substituting a metrically different one, **which moves
every measurement taken from the nameplate** — that is why both by-decision absences are declared in
their casting's own `GUI-SPEC.md` §9 / §10 and restated here. Nobody has substituted a face and
nobody will, but the two kinds of absence sitting in one table is what keeps that true.

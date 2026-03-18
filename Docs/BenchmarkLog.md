# Benchmark Log

## Campaign 1 - 2026-03-17 - Air only (single scenario)

Scenario: air_no_input

### Raw metrics

| Preset | Duration (s) | Max planar speed | Avg planar speed | Airborne time (s) | Worst air speed loss |
|---|---:|---:|---:|---:|---:|
| Classic | 9.967 | 3180.14 | 2050.27 | 5.764 | 20.21% |
| Agile | 9.971 | 2963.24 | 1987.71 | 6.071 | 19.09% |
| Heavy | 9.972 | 3239.97 | 2022.06 | 6.019 | 17.96% |
| Wild | 9.998 | 3218.51 | 2150.87 | 5.493 | 15.53% |

### Conclusions

- Best avg speed: Wild
- Best air stability: Wild
- Best max speed peak: Heavy
- This campaign was useful but not sufficient to choose a global winner because only one scenario was tested.

## Campaign 2 - 2026-03-17 - Matrix (air scenarios)

Scenarios tested:
- air_no_input
- air_forward_hold
- air_turn_left_hold
- air_brake_pulse
- air_boost_window

### Winners per scenario

- air_no_input: Agile
- air_forward_hold: Agile
- air_turn_left_hold: Classic
- air_brake_pulse: Heavy
- air_boost_window: Wild

### Global ranking (points)

- Agile: 18
- Classic: 14
- Wild: 12
- Heavy: 11

### Conclusions

- Best global profile on this matrix: Agile
- Classic remains strongest on sustained turning behavior
- Heavy remains strongest on brake pulse behavior
- Wild remains strongest on boost burst behavior but can destabilize in sustained turn

## Next campaign target

- Add ground_cp_run on softer terrain with checkpoint-like timeline.
- Compare preset behavior on mixed ground control (turn, brake, boost).

## Balance target model - 2026-03-18

Goal:
- No universally best preset.
- Each preset must be first on at least one terrain archetype.
- Global score spread should stay narrow (target <= 8%).

### Preset identity targets

| Preset | Intended advantage | Intended drawback | Target terrain |
|---|---|---|---|
| Classic | Stability + consistency in chained turns | Lower burst acceleration | Mixed technical valley |
| Agile | Best relaunch and direction changes | More heat pressure under continuous throttle | Switchbacks and checkpoint rhythm |
| Heavy | Brake authority and slope composure | Lower avg speed on flat stretches | Long downhill control and braking zones |
| Wild | Highest burst potential and risk/reward | Lower predictability in sustained steering | Opportunistic straights and dynamic terrain |

### Metric guardrails

- Avg speed spread between presets on same scenario: target <= 10%.
- Time-to-finish spread on same scenario: target <= 8% unless scenario is explicitly niche.
- Critical overheat runs in benchmark scenarios: target = 0.
- Each preset must be top-1 on at least one scenario over a full matrix.

## Valley scenario set - same start and orientation

Common anchor:
- StartLocation: X=22156.984799, Y=-35180.367373, Z=-904.293776
- StartRotation: Pitch=-14.407581, Yaw=-110.829080, Roll=-149.458124

New scenarios added:
- ground_valley_sprint
- ground_valley_switchback

Purpose:
- Keep reproducibility from a common valley anchor.
- Differentiate profile strengths on speed flow vs rapid direction changes.

## Campaign 3 - 2026-03-18 - Full matrix (air + valley ground)

Scenarios tested:
- air_no_input
- air_forward_hold
- air_turn_left_hold
- air_brake_pulse
- air_boost_window
- ground_cp_run
- ground_valley_sprint
- ground_valley_switchback

### Winners per scenario (latest run per preset/scenario)

- air_no_input: Classic
- air_forward_hold: Agile
- air_turn_left_hold: Heavy
- air_brake_pulse: Classic
- air_boost_window: Agile
- ground_cp_run: Classic
- ground_valley_sprint: Heavy
- ground_valley_switchback: Agile

### Global ranking (points, 4-3-2-1)

- Agile: 22
- Wild: 22
- Classic: 18
- Heavy: 18

### Differentiation reading

- No single preset dominates all contexts.
- Classic leads on neutral air and checkpoint consistency.
- Agile leads on controlled speed flow and switchback handling.
- Heavy leads on turn-hold and valley sprint control.
- Wild remains competitive globally but wins fewer niche categories.

### Balance status

- Differentiation objective: partially achieved.
- Next tuning objective: give Wild and Heavy clearer niche wins while keeping global spread tight.

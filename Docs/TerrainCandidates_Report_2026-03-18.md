# Terrain Candidates Report - 2026-03-18

## Scope
Qualification GO/NO-GO des 3 maps procedurales candidates v1 pour entree benchmark.

## References
- Fun metrics: `Docs/FunTerrainMetrics_v1.md`
- QA checklist: `Docs/TerrainQA_Checklist_v2.md`
- Import runbook: `Docs/Unreal_Import_Runbook_10min.md`
- Dataset: `Tools/HeightmapProcessing/output/candidates_v1/`

## Session config baseline
- Landscape resolution: 1009 x 1009
- Scale X/Y: 100
- Scale Z baseline: 50 (ajuster 80-140 si relief trop plat)
- Target: import -> map jouable <= 15 min

## Candidate 1 - valley_flow (seed 1011)

### Source files
- PNG: `Tools/HeightmapProcessing/output/candidates_v1/valley_flow_1009_norm.png`
- R16 fallback: `Tools/HeightmapProcessing/output/candidates_v1/valley_flow_1009.r16`
- Preview: `Tools/HeightmapProcessing/output/candidates_v1/valley_flow_preview.png`

### Import notes
- Import start time:
- Import end time:
- Actual Scale X/Y:
- Actual Scale Z:
- Orientation notes:

### QA critical checks
- Start valid: [ ]
- Finish reachable: [ ]
- Checkpoints valid: [ ]
- Respawn valid: [ ]
- No critical collision blocker: [ ]

### Fun metrics scoring
- Flow (/35):
- RiskReward (/25):
- Recoverability (/25):
- Readability (/15):
- Total (/100):

### Decision
- GO benchmark: [ ]
- NO-GO: [ ]
- Top defects (max 3):
- Corrective action:

## Candidate 2 - valley_switchback (seed 2027)

### Source files
- PNG: `Tools/HeightmapProcessing/output/candidates_v1/valley_switchback_1009_norm.png`
- R16 fallback: `Tools/HeightmapProcessing/output/candidates_v1/valley_switchback_1009.r16`
- Preview: `Tools/HeightmapProcessing/output/candidates_v1/valley_switchback_preview.png`

### Import notes
- Import start time:
- Import end time:
- Actual Scale X/Y:
- Actual Scale Z:
- Orientation notes:

### QA critical checks
- Start valid: [ ]
- Finish reachable: [ ]
- Checkpoints valid: [ ]
- Respawn valid: [ ]
- No critical collision blocker: [ ]

### Fun metrics scoring
- Flow (/35):
- RiskReward (/25):
- Recoverability (/25):
- Readability (/15):
- Total (/100):

### Decision
- GO benchmark: [ ]
- NO-GO: [ ]
- Top defects (max 3):
- Corrective action:

## Candidate 3 - valley_risk_reward (seed 3091)

### Source files
- PNG: `Tools/HeightmapProcessing/output/candidates_v1/valley_risk_reward_1009_norm.png`
- R16 fallback: `Tools/HeightmapProcessing/output/candidates_v1/valley_risk_reward_1009.r16`
- Preview: `Tools/HeightmapProcessing/output/candidates_v1/valley_risk_reward_preview.png`

### Import notes
- Import start time:
- Import end time:
- Actual Scale X/Y:
- Actual Scale Z:
- Orientation notes:

### QA critical checks
- Start valid: [ ]
- Finish reachable: [ ]
- Checkpoints valid: [ ]
- Respawn valid: [ ]
- No critical collision blocker: [ ]

### Fun metrics scoring
- Flow (/35):
- RiskReward (/25):
- Recoverability (/25):
- Readability (/15):
- Total (/100):

### Decision
- GO benchmark: [ ]
- NO-GO: [ ]
- Top defects (max 3):
- Corrective action:

## Final shortlist
- Selected for benchmark:
- Backup candidate:
- Rework needed on:

## Recommendation rule
- Priorite 1: aucun blocage critique.
- Priorite 2: Score total >= 70.
- Priorite 3: meilleur combo Flow + Recoverability pour benchmark stable.
import json
from pathlib import Path

from pydantic import BaseModel

from belbin_orchestration.models import BelbinRole, TeamMember

ROLE_TRANSLATION = {
    "Investigador de Recursos": BelbinRole.RESOURCE_INVESTIGATOR,
    "Cohesionador": BelbinRole.TEAMWORKER,
    "Coordinador": BelbinRole.COORDINATOR,
    "Cerebro": BelbinRole.PLANT,
    "Monitor Evaluador": BelbinRole.MONITOR_EVALUATOR,
    "Especialista": BelbinRole.SPECIALIST,
    "Impulsor": BelbinRole.SHAPER,
    "Implementador": BelbinRole.IMPLEMENTER,
    "Finalizador": BelbinRole.COMPLETER_FINISHER,
}


class SimulationConfig(BaseModel):
    global_seed: int
    team: list[TeamMember]
    affinity_matrix: dict[BelbinRole, dict[BelbinRole, float]]


def load_synthetic_scenario(json_path: Path, txt_path: Path, seed: int = 29) -> SimulationConfig:
    with open(json_path) as f:
        raw_members = json.load(f)

    team = []
    for raw in raw_members:
        team.append(
            TeamMember(
                member_id=raw["nombre"],
                primary_role=ROLE_TRANSLATION[raw["personalidad1"]],
                secondary_role=ROLE_TRANSLATION[raw["personalidad2"]],
                primary_weight=float(raw["porcentaje1"]),
                secondary_weight=float(raw["porcentaje2"]),
            )
        )

    affinity_matrix: dict[BelbinRole, dict[BelbinRole, float]] = {role: {} for role in BelbinRole}

    with open(txt_path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            roles, weight = line.split(",")
            role_a, role_b = roles.split("-")

            role_a = ROLE_TRANSLATION[role_a.strip()]
            role_b = ROLE_TRANSLATION[role_b.strip()]
            weight = float(weight.strip())

            affinity_matrix[role_a][role_b] = weight

    return SimulationConfig(global_seed=seed, team=team, affinity_matrix=affinity_matrix)


if __name__ == "__main__":
    root_dir = Path(__file__).resolve().parents[4]

    data_dir = root_dir / "data/raw/synthetic"
    config = load_synthetic_scenario(data_dir / "test1.json", data_dir / "test1.txt")

    out_dir = root_dir / "experiments/scenario_base"
    out_dir.mkdir(parents=True, exist_ok=True)

    with open(out_dir / "config.json", "w") as f:
        f.write(config.model_dump_json(indent=2))

    print("Configuration created at 'experiments/scenario_base/config.json")

from pathlib import Path
from belbin_analysis.models import Interaction, TeamMember

def load_dataset(path: Path) -> list[Interaction]:
    # TODO: implementar contra el formato real cuando llegue el dataset.
    # Ver data/raw/DATASET_CONTRACT.md para el esquema esperado.
    raise NotImplementedError("Pendiente: acceso al dataset del paper")

def load_team_roster(path: Path) -> list[TeamMember]:
    raise NotImplementedError("Pendiente: acceso al dataset del paper")

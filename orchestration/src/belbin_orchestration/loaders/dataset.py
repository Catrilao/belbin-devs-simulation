from pathlib import Path

from belbin_orchestration.models import Interaction, TeamMember


def load_dataset(path: Path) -> list[Interaction]:
    # TODO: implementar contra el formato real cuando llegue el dataset.
    raise NotImplementedError("Pendiente: acceso al dataset del paper")


def load_team_roster(path: Path) -> list[TeamMember]:
    raise NotImplementedError("Pendiente: acceso al dataset del paper")

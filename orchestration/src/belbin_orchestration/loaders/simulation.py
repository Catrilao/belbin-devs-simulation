from pathlib import Path

from belbin_orchestration.models import Interaction


def load_simulation_output(run_dir: Path) -> list[Interaction]:
    """Reads the output of a simulation run and returns the interactions"""

    # TODO: implementar el parseo real del formato CSVLogger de Cadmium.
    # Por ahora levanta NotImplementedError para que un test que lo invoque
    # falle explícitamente, en vez de devolver datos silenciosamente incorrectos.

    raise NotImplementedError("Pendiente: parsear output_state.txt de Cadmium")

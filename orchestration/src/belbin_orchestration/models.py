from enum import StrEnum

from pydantic import BaseModel


class BelbinRole(StrEnum):
    PLANT = "plant"
    MONITOR_EVALUATOR = "monitor_evaluator"
    SPECIALIST = "specialist"
    SHAPER = "shaper"
    IMPLEMENTER = "implementer"
    COMPLETER_FINISHER = "completer_finisher"
    COORDINATOR = "coordinator"
    TEAMWORKER = "teamworker"
    RESOURCE_INVESTIGATOR = "resource_investigator"


class TeamMember(BaseModel):
    member_id: str
    primary_role: BelbinRole
    secondary_role: BelbinRole
    primary_weight: float
    secondary_weight: float


class Interaction(BaseModel):
    source: str
    target: str
    weight: float
    timestamp: float | None = None

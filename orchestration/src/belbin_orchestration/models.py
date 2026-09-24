from pydantic import BaseModel
from enum import Enum

class BelbinRole(str, Enum):
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
    secondary_role: BelbinRole | None = None

class Interaction(BaseModel):
    source: str
    target: str
    weight: float
    timestamp: float | None = None

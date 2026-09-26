from pathlib import Path

import pytest

from belbin_orchestration.loaders.dataset import load_dataset


def test_load_dataset_not_yet_implemented():
    with pytest.raises(NotImplementedError):
        load_dataset(Path("dummy_path"))

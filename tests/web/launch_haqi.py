"""Compatibility entry point for the Haqi browser test launcher."""

from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from haqi.launch_haqi import main, web_server


if __name__ == "__main__":
    main()

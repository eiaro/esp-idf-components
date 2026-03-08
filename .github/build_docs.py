#!/usr/bin/env python3

import argparse
import logging
import os
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


LOG = logging.getLogger("docs_pipeline")


@dataclass
class PipelineSettings:
    root: Path
    output_dir: Path
    docs_version: str
    stop_on_error: bool


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build documentation sites for all components in this repository.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--version", default="latest", help="Version segment for generated site URLs")
    parser.add_argument("--output-dir", default="docs_build_output", help="Where to place built books")
    parser.add_argument("--no-fail-fast", action="store_true", help="Keep processing after a component failure")
    parser.add_argument("--verbose", "-v", action="store_true", help="Enable debug logs")
    return parser.parse_args()


def discover_docs_components(root: Path) -> list[Path]:
    found: list[Path] = []
    for manifest in sorted(root.glob("*/idf_component.yml")):
        component_dir = manifest.parent
        docs_dir = component_dir / "docs"
        if (docs_dir / "book.toml").is_file():
            found.append(component_dir)
    return found


def run_checked(command: list[str], cwd: Path, env: dict[str, str] | None = None) -> subprocess.CompletedProcess:
    LOG.debug("cmd: %s (cwd=%s)", " ".join(command), cwd)
    return subprocess.run(command, cwd=str(cwd), env=env, check=True, capture_output=True, text=True)


def try_generate_api_markdown(docs_dir: Path) -> None:
    (docs_dir / "src").mkdir(exist_ok=True)
    try:
        run_checked(["esp-doxybook", "-i", "doxygen_output/xml", "-o", "src/api.md"], cwd=docs_dir)
        LOG.info("API markdown updated: %s", docs_dir / "src" / "api.md")
    except FileNotFoundError:
        LOG.warning("esp-doxybook is not installed; skipping API markdown generation for %s", docs_dir)
    except subprocess.CalledProcessError as error:
        LOG.warning("Skipping API markdown generation for %s", docs_dir)
        LOG.debug("esp-doxybook stdout:\n%s", error.stdout)
        LOG.debug("esp-doxybook stderr:\n%s", error.stderr)


def build_component_book(component_dir: Path, settings: PipelineSettings) -> None:
    component_name = component_dir.name
    docs_dir = component_dir / "docs"
    try_generate_api_markdown(docs_dir)

    mdbook_env = os.environ.copy()
    mdbook_env["MDBOOK_OUTPUT__HTML__SITE_URL"] = f"/{settings.docs_version}/{component_name}/"

    result = run_checked(["mdbook", "build", str(docs_dir)], cwd=settings.root, env=mdbook_env)
    LOG.debug("mdbook stdout:\n%s", result.stdout)
    LOG.debug("mdbook stderr:\n%s", result.stderr)

    generated_book = docs_dir / "book"
    if not generated_book.exists():
        raise FileNotFoundError(f"mdbook build finished but missing output directory: {generated_book}")

    destination = settings.output_dir / component_name
    if destination.exists():
        shutil.rmtree(destination)
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copytree(generated_book, destination)
    LOG.info("Built docs for %s", component_name)


def run_pipeline(settings: PipelineSettings) -> int:
    components = discover_docs_components(settings.root)
    if not components:
        LOG.info("No components with docs/book.toml found.")
        return 0

    LOG.info("Components with documentation: %s", ", ".join(component.name for component in components))

    if settings.output_dir.exists():
        shutil.rmtree(settings.output_dir)
    settings.output_dir.mkdir(parents=True, exist_ok=True)

    had_failure = False
    for component_dir in components:
        try:
            build_component_book(component_dir, settings)
        except Exception as error:
            had_failure = True
            LOG.error("Documentation build failed for %s: %s", component_dir.name, error)
            if settings.stop_on_error:
                return 1

    return 1 if had_failure else 0


def main() -> int:
    args = parse_args()
    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
        datefmt="%H:%M:%S",
    )

    settings = PipelineSettings(
        root=Path.cwd(),
        output_dir=Path.cwd() / args.output_dir,
        docs_version=args.version,
        stop_on_error=not args.no_fail_fast,
    )
    return run_pipeline(settings)


if __name__ == "__main__":
    sys.exit(main())
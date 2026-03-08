#!/usr/bin/env python3

import argparse
import logging
import shutil
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


LOG = logging.getLogger("api_md")


@dataclass
class ComponentDocs:
    name: str
    component_dir: Path
    docs_dir: Path
    doxyfile: Path
    xml_dir: Path
    root_api_md: Path
    api_md: Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate docs/src/api.md for every component that has docs/Doxyfile",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--root", type=Path, default=Path.cwd(), help="Repository root")
    parser.add_argument("--dry-run", action="store_true", help="Print commands only")
    parser.add_argument("--verbose", "-v", action="store_true", help="Enable debug logs")
    return parser.parse_args()


def read_doxyfile_setting(doxyfile: Path, key: str, default: str) -> str:
    for raw_line in doxyfile.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        left, right = line.split("=", 1)
        if left.strip() == key:
            value = right.strip()
            return value if value else default
    return default


def discover_component_docs(root: Path) -> list[ComponentDocs]:
    items: list[ComponentDocs] = []
    for manifest in sorted(root.glob("*/idf_component.yml")):
        component_dir = manifest.parent
        docs_dir = component_dir / "docs"
        doxyfile = docs_dir / "Doxyfile"
        if not doxyfile.is_file():
            continue

        output_dir = read_doxyfile_setting(doxyfile, "OUTPUT_DIRECTORY", "doxygen_output")
        xml_subdir = read_doxyfile_setting(doxyfile, "XML_OUTPUT", "xml")

        xml_dir = docs_dir / output_dir / xml_subdir
        api_md = docs_dir / "src" / "api.md"
        items.append(
            ComponentDocs(
                name=component_dir.name,
                component_dir=component_dir,
                docs_dir=docs_dir,
                doxyfile=doxyfile,
                xml_dir=xml_dir,
                root_api_md=component_dir / "API.md",
                api_md=api_md,
            )
        )
    return items


def execute(cmd: list[str], cwd: Path, dry_run: bool) -> None:
    LOG.info("%s (cwd=%s)", " ".join(cmd), cwd)
    if dry_run:
        return
    subprocess.run(cmd, cwd=str(cwd), check=True)


def build_api_for_component(item: ComponentDocs, dry_run: bool) -> None:
    item.api_md.parent.mkdir(parents=True, exist_ok=True)
    item.root_api_md.parent.mkdir(parents=True, exist_ok=True)
    execute(["doxygen", "Doxyfile"], cwd=item.docs_dir, dry_run=dry_run)
    execute(["esp-doxybook", "-i", str(item.xml_dir), "-o", str(item.root_api_md)], cwd=item.docs_dir, dry_run=dry_run)
    if dry_run:
        LOG.info("copy %s -> %s", item.root_api_md, item.api_md)
    else:
        shutil.copyfile(item.root_api_md, item.api_md)


def main() -> int:
    args = parse_args()
    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO, format="%(levelname)s: %(message)s")

    root = args.root.resolve()
    components = discover_component_docs(root)
    if not components:
        LOG.info("No components with docs/Doxyfile found")
        return 0

    LOG.info("Components: %s", ", ".join(item.name for item in components))
    for item in components:
        LOG.info("Generating API markdown for %s", item.name)
        build_api_for_component(item, dry_run=args.dry_run)

    LOG.info("Finished API markdown generation")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        LOG.error("Command failed with exit code %s", error.returncode)
        raise SystemExit(error.returncode)

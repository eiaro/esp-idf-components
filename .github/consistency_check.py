#!/usr/bin/env python

import argparse
import glob
import logging
import os
from pathlib import Path

try:
    import yaml
except ModuleNotFoundError as error:
    raise SystemExit("Missing dependency: pyyaml. Install with: python -m pip install pyyaml") from error


LOG = logging.getLogger("repo_checks")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Repository consistency checks")
    parser.add_argument("--root", default=".", help="Repository root directory")
    parser.add_argument("--verbose", "-v", action="store_true", help="Enable debug logging")
    return parser.parse_args()


def run_checks(root_dir: Path) -> list[str]:
    errors: list[str] = []
    errors.extend(validate_build_manifest_registry(root_dir))
    errors.extend(validate_upload_workflow_components(root_dir))
    errors.extend(validate_issue_template_components(root_dir))
    return errors


def validate_build_manifest_registry(root_dir: Path) -> list[str]:
    LOG.info("Checking .idf_build_apps.toml manifest entries")

    repo_manifests = {
        Path(path)
        for path in glob.glob(f"{root_dir}/**/.build-test-rules.yml", recursive=True)
        if "managed_components" not in path
    }

    config = load_toml(root_dir / ".idf_build_apps.toml")
    configured_manifests = {Path(path) for path in config.get("manifest_file", [])}

    missing = sorted(repo_manifests - configured_manifests)
    if missing:
        return [f"Missing .build-test-rules.yml in .idf_build_apps.toml: {', '.join(str(item) for item in missing)}"]
    return []


def validate_upload_workflow_components(root_dir: Path) -> list[str]:
    LOG.info("Checking component list in upload workflow")

    workflow = load_yaml(root_dir / ".github/workflows/upload_component.yml")
    jobs = workflow.get("jobs", {})
    upload_step = None
    for job in jobs.values():
        steps = job.get("steps", []) if isinstance(job, dict) else []
        upload_step = next(
            (
                step for step in steps
                if step.get("uses", "").startswith("espressif/upload-components-ci-action")
                and isinstance(step.get("with", {}).get("components", ""), str)
            ),
            None,
        )
        if upload_step is not None:
            break

    if upload_step is None:
        return ["Could not find upload-components-ci-action step with a components list in upload workflow"]

    listed_components = {
        line.strip()
        for line in upload_step.get("with", {}).get("components", "").splitlines()
        if line.strip()
    }
    repo_components = set(get_component_names(root_dir))

    missing = sorted(repo_components - listed_components)
    if missing:
        return [f"Missing components in upload workflow: {', '.join(missing)}"]
    return []


def validate_issue_template_components(root_dir: Path) -> list[str]:
    LOG.info("Checking component dropdown in bug report template")

    template = load_yaml(root_dir / ".github/ISSUE_TEMPLATE/bug-report.yml")
    component_field = next((item for item in template.get("body", []) if item.get("id") == "component"), None)
    if component_field is None:
        return ["Missing 'component' dropdown in .github/ISSUE_TEMPLATE/bug-report.yml"]

    options = component_field.get("attributes", {}).get("options", [])
    if not isinstance(options, list):
        return ["Invalid bug-report component options: expected list"]

    option_set = set(options)
    repo_components = set(get_component_names(root_dir))

    errors: list[str] = []
    missing = sorted(repo_components - option_set)
    extra = sorted(option_set - repo_components - {"Other"})

    if missing:
        errors.append(f"Missing components in bug-report dropdown: {', '.join(missing)}")
    if extra:
        errors.append(f"Unexpected components in bug-report dropdown: {', '.join(extra)}")
    return errors


def get_component_names(root_dir: Path) -> list[str]:
    manifests = glob.glob(f"{root_dir}/*/idf_component.yml")
    return sorted({Path(path).parent.name for path in manifests})


def load_toml(file_path: Path) -> dict:
    try:
        import tomllib  # type: ignore

        with open(file_path, "rb") as file:
            return tomllib.load(file)
    except ImportError:
        try:
            import toml
        except ModuleNotFoundError as error:
            raise SystemExit("Missing dependency: toml. Install with: python -m pip install toml") from error

        return toml.load(str(file_path))


def load_yaml(file_path: Path) -> dict:
    with open(file_path, "r", encoding="utf-8") as file:
        data = yaml.safe_load(file)
    if not isinstance(data, dict):
        raise ValueError(f"YAML root must be a mapping in {file_path}")
    return data


def main() -> int:
    args = parse_args()
    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)

    root_dir = Path(args.root).resolve()
    issues = run_checks(root_dir)
    if issues:
        for issue in issues:
            LOG.error(issue)
        LOG.error("Found %d consistency issue(s)", len(issues))
        return 1

    LOG.info("All consistency checks passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
